#include <algorithm>
#include "mbed.h"
#include "mbed_stats.h"
#include "TCPSocket.h"
#include "EthernetInterface.h"

#define MAX_TEST_ROUNDS     3

// Test connection information
#define USE_HTTP_1_1
#define HTTP_SERVER_NAME            "www.ifconfig.io"
#define HTTP_SERVER_FILE_PATH       "/method"
#define HTTP_SERVER_PORT            80
#define HTTP_RECVBUFF_MAX           1024

namespace {

char http_recv_buff[HTTP_RECVBUFF_MAX];

}

// Case insensitive of std::strcmp
int strcmp_ci(const char *s1, const char *s2)
{
    while (*s2) {
        int d = toupper(*s1) - toupper(*s2);
        if (d) {
            return d;
        }
        s1 ++;
        s2 ++;
    }

    return toupper(*s1);
}

// Case insensitive of std::search
const char *search_ci(const char *first, const char *last,
                      const char *s_first, const char *s_last)
{
    auto bin_pred_ci = [](int c1, int c2) -> bool {
        return toupper(c1) == toupper(c2);
    };
    return std::search(first, last, s_first, s_last, bin_pred_ci);
}

// Locate 'value' part of 'name:value'
// NOTE: The returned 'value' part can have leadinging and/or trailing
//       whitespace which needs further processing.
const char *http_fieldvalue(const char *first, const char *last,
                            const char *fieldname)
{
    size_t fieldname_len = strlen(fieldname);
    const char *found = search_ci(first, last,
                                  fieldname, fieldname + fieldname_len);
    return (found == last) ? NULL : (found + fieldname_len + 1);
}

// Do one HTTP request/response round
// NOTE: This will switch socket to non-blocking mode.
void http_request_response(
    const char *iface,
    TCPSocket *sock,
    const char *http_req,
    char *http_resp_buff,
    size_t http_resp_buffsize)
{
    /* Non-blocking mode
     *
     * Don't change to non-blocking mode before connect; otherwise, we may meet NSAPI_ERROR_IN_PROGRESS.
     */
    sock->set_blocking(false);
      
    int rc = 0;
    bool got200 = false;
    size_t offset = 0;
    size_t offset_end = strlen(http_req);
    
    printf("[%s]HTTP: Request message:\r\n", iface);
    printf("%s", http_req);
    printf("\r\n\r\n");

    do {
        rc = sock->send((const unsigned char *) http_req + offset, offset_end - offset);
        if (rc > 0) {
            offset += rc;
        }
    } while (offset < offset_end && 
            (rc > 0 || rc == NSAPI_ERROR_WOULD_BLOCK));
    if (rc < 0 &&
        rc != NSAPI_ERROR_WOULD_BLOCK) {
        printf("[%s]HTTP: socket send failed: %d", iface, rc);
        return;
    }

    /* Read data out of the socket */
    offset = 0;
    offset_end = 0;
    size_t content_length = 0;
    char *line_beg = http_resp_buff;
    char *line_end = NULL;
    do {
        rc = sock->recv((unsigned char *) http_resp_buff + offset, http_resp_buffsize - offset - 1);
        if (rc > 0) {
            offset += rc;
        }

        /* Make it null-terminated */
        http_resp_buff[offset] = 0;

        /* Scan response message
         *             
         * 1. A status line which includes the status code and reason message (e.g., HTTP/1.1 200 OK)
         * 2. Response header fields (e.g., Content-Type: text/html)
         * 3. An empty line (\r\n)
         * 4. An optional message body
         */
        if (! offset_end) {
            // Scan received lines as possible
            do {
                line_end = strstr(line_beg, "\r\n");
                if (line_end) {
                    /* Scan status line */
                    if (! got200) {
                        got200 = strstr(line_beg, "200 OK") != NULL;
                    }

                    /* Scan response header fields for Content-Length 
                     *
                     * NOTE: Assume chunked transfer (Transfer-Encoding: chunked) is not used
                     * NOTE: Support filed name case-insensitive
                     * NOTE: Support both 'name:field' and 'name: field ' (any number of
                     *       leading and/or trailing whitespace)
                     */
                    if (content_length == 0) {
                        const char *fieldvalue = NULL;

                        // The 'value' part can have leadinging and/or trailing whitespace.
                        // Let scanf or the like handle it.
                        fieldvalue = http_fieldvalue(line_beg, line_end, "content-length");
                        if (fieldvalue != NULL) {
                            sscanf(fieldvalue, "%d", &content_length);
                        }
                    }

                    /* An empty line indicates end of response header fields */
                    if (line_beg == line_end) {
                        offset_end = line_end - http_resp_buff + 2 + content_length;
                    }

                    /* Go to next line */
                    line_beg = line_end + 2;
                }
            } while (line_end != NULL);
        }
    } while ((offset_end == 0 || offset < offset_end) &&
            (rc > 0 || rc == NSAPI_ERROR_WOULD_BLOCK));
    if (rc < 0 && 
        rc != NSAPI_ERROR_WOULD_BLOCK) {
        printf("[%s]HTTP: socket recv failed: %d", iface, rc);
        return;
    }

    http_resp_buff[offset] = '\0';

    /* Print status messages */
    printf("[%s]HTTP: Received %d chars from server\n", iface, offset);
    printf("[%s]HTTP: Received 200 OK status ... %s\n", iface, got200 ? "[OK]" : "[FAIL]");
    printf("[%s]HTTP: Received message:\n", iface);
    printf("%s", http_resp_buff);
    printf("\r\n\r\n");
}

void http_test(const char *iface, TCPSocket *sock)
{
    // We are constructing GET command like this:
    // GET /method HTTP/1.1\r\nHost: ifconfig.io\r\nConnection: close\r\n\r\n"
    const char *http_req =
        "GET " HTTP_SERVER_FILE_PATH " HTTP/1.1\r\n"
        "Host: " HTTP_SERVER_NAME "\r\n"
        #if 0
        "Connection: close\r\n"
        #endif
        "\r\n";
    http_request_response(iface, sock, http_req, http_recv_buff, sizeof(http_recv_buff));
}

int main()
{
#if MBED_HEAP_STATS_ENABLED
    mbed_stats_heap_t heap_stats;
#endif

    int rc = 0;
    bool has_eth = true;
    SocketAddress sockaddr;

    printf("Start Connection ... \r\n");

    // WiFi part
    TCPSocket wifi_sock;
    NetworkInterface *wifi_network_interface = WiFiInterface::get_default_instance();
    if (NULL == wifi_network_interface) {
        printf("NULL network interface! Exiting application....\r\n");
        return EXIT_FAILURE;
    }
    wifi_network_interface->set_default_parameters();

    printf("\n\rConnecting to WiFi..\r\n");
    rc = wifi_network_interface->connect();
    if(rc == 0) {
        printf("\n\rConnected to Network (WiFi) successfully\r\n");
    } else {
        printf("\n\rConnection to Network (WiFi) Failed %d! Exiting application....\r\n", rc);
        return EXIT_FAILURE;
    }
    rc = wifi_network_interface->get_ip_address(&sockaddr);
    if (rc != NSAPI_ERROR_OK) {
        printf("Network interface (WiFi) get_ip_address(...) failed with %d\r\n", rc);
        return EXIT_FAILURE;
    } else {
        printf("WiFi TCP client IP Address is %s\r\n", sockaddr.get_ip_address());
    }

    rc = wifi_sock.open(wifi_network_interface);
    if (rc != NSAPI_ERROR_OK) {
        printf("TCP socket open (WiFi) failed with %d", rc);
        return EXIT_FAILURE;
    }

    //Eth part
    TCPSocket eth_sock;
    NetworkInterface *eth_network_interface = EthernetInterface::get_default_instance();
    if (eth_network_interface == NULL) {
        printf("\n\rNo ETH\r\n");
        has_eth = false;
        goto DO_HTTP_CONN;
    }

    printf("\n\rConnecting to ETH..\r\n");
    rc = eth_network_interface->connect(); //Use DHCP
    if(rc == 0) {
        printf("\n\rConnected to Network (ETH) successfully\r\n");
    } else {
        printf("\n\rConnection to Network (ETH) Failed %d!\r\n", rc);
        has_eth = false;
        goto DO_HTTP_CONN;
    }

    rc = eth_network_interface->get_ip_address(&sockaddr);
    if (rc != NSAPI_ERROR_OK) {
        printf("Network interface (ETH) get_ip_address(...) failed with %d\r\n", rc);
        has_eth = false;
        goto DO_HTTP_CONN;
    } else {
        printf("Eth TCP client IP Address is %s\r\n", sockaddr.get_ip_address());
    }

    rc = eth_sock.open(eth_network_interface);
    if (rc != NSAPI_ERROR_OK) {
        printf("TCP socket open (ETH) failed with %d", rc);
        has_eth = false;
        goto DO_HTTP_CONN;
    }

DO_HTTP_CONN:

    printf("\r\n");

    printf("WiFi HTTP Connection ... \r\n");
    rc = wifi_network_interface->gethostbyname(HTTP_SERVER_NAME, &sockaddr);
    if (rc != NSAPI_ERROR_OK) {
        printf("WiFi HTTP Connection ... DNS failure\r\n");
    } else {
        sockaddr.set_port(HTTP_SERVER_PORT);
        rc = wifi_sock.connect(sockaddr);
        if (rc != NSAPI_ERROR_OK) {
            printf("WiFi HTTP Connection ... Failed\r\n");
        } else {
            printf("WiFi HTTP: Connected to %s:%d\r\n", HTTP_SERVER_NAME, HTTP_SERVER_PORT);
        }
    }

    if (!has_eth) {
        goto DO_HTTP_SEND_RECV;
    }

    printf("ETH HTTP Connection ... \r\n");
    rc = eth_network_interface->gethostbyname(HTTP_SERVER_NAME, &sockaddr);
    if (rc != NSAPI_ERROR_OK) {
        printf("ETH HTTP Connection ... DNS failure\r\n");
    } else {
        sockaddr.set_port(HTTP_SERVER_PORT);
        rc = eth_sock.connect(sockaddr);
        if (rc != NSAPI_ERROR_OK) {
            printf("ETH HTTP Connection ... Failed\r\n");
        } else {
            printf("ETH HTTP: Connected to %s:%d\r\n", HTTP_SERVER_NAME, HTTP_SERVER_PORT);
        }
    }

DO_HTTP_SEND_RECV:

    printf("\r\n");

    int test_round = 0;
    while(1) {
        printf("\r\n");

#if MBED_HEAP_STATS_ENABLED
        mbed_stats_heap_get(&heap_stats);
        printf("Current heap: %lu\r\n", heap_stats.current_size);
        printf("Max heap size: %lu\r\n", heap_stats.max_size);
        printf("\r\n");
#endif

        test_round ++;
        if (test_round > MAX_TEST_ROUNDS) {
            break;
        }
        printf("Test round %d ... \r\n\r\n", test_round);

        printf("Test WiFi TCP client\r\n");
        http_test("WiFi",&wifi_sock);
        ThisThread::sleep_for(2s);

        if (eth_network_interface == NULL) {
            continue;
        }

        printf("Test Eth TCP client\r\n");
        http_test("Eth",&eth_sock);
        ThisThread::sleep_for(2s);
    }

    printf(" Close socket & disconnect ... \r\n");
    wifi_sock.close();
    wifi_network_interface->disconnect();
    // Redundant socket close won't do harm
    eth_sock.close();
    // Redundant network interface disconnect won't do harm
    if (eth_network_interface != NULL) {
        eth_network_interface->disconnect();
    }
    printf(" End \r\n");
}
