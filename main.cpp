#include <algorithm>
#include "mbed.h"
#include "mbed_stats.h"
#include "TCPSocket.h"
#include "EthernetInterface.h"

#define MBED_HEAP_STATS_ENABLED 1
#define USE_HTTP_1_1


namespace {
    // Test connection information

const char *HTTP_SERVER_NAME = "www.ifconfig.io"; 

const char *HTTP_SERVER_FILE_PATH = "/method";
const int HTTP_SERVER_PORT = 80;


    const int RECV_BUFFER_SIZE = 512;

    // Test related data
    const char *HTTP_OK_STR = "200 OK";
    const char *HTTP_EXPECT_STR = "GET";

    // Test buffers
    char buffer[RECV_BUFFER_SIZE] = {0};
}

bool find_substring(const char *first, const char *last, const char *s_first, const char *s_last) {
    const char *f = std::search(first, last, s_first, s_last);
    return (f != last);
}

void http_test(char *str, TCPSocket *sock) {
    bool result = true;
    int rc = 0;
    
        // We are constructing GET command like this:

       // GET /method HTTP/1.1\r\nHost: ifconfig.io\r\nConnection: close\r\n\r\n"
        strcpy(buffer, "GET ");
        strcat(buffer, HTTP_SERVER_FILE_PATH);   
        strcat(buffer, " HTTP/1.1\r\nHost: ");
        strcat(buffer, HTTP_SERVER_NAME);
    #if 1
        strcat(buffer, "\r\n\r\n");
    #else
        strcat(buffer, "\r\nConnection: close\r\n\r\n");
    #endif
        
        // Send GET command
        sock->send(buffer, strlen(buffer));

        // Server will respond with HTTP GET's success code
        const int ret = sock->recv(buffer, sizeof(buffer) - 1);
        buffer[ret] = '\0';
        
        // Find 200 OK HTTP status in reply
        bool found_200_ok = find_substring(buffer, buffer + ret, HTTP_OK_STR, HTTP_OK_STR + strlen(HTTP_OK_STR));
        // Find "deny" string in reply
        bool found_expect = find_substring(buffer, buffer + ret, HTTP_EXPECT_STR, HTTP_EXPECT_STR + strlen(HTTP_EXPECT_STR));

        if (!found_200_ok) result = false;
        if (!found_expect) result = false;

        printf("[%s]HTTP: Received %d chars from server\r\n",str,ret);
        printf("[%s]HTTP: Received 200 OK status ... %s\r\n",str, found_200_ok ? "[OK]" : "[FAIL]");
        printf("[%s]HTTP: Received '%s' status ... %s\r\n",str, HTTP_EXPECT_STR, found_expect ? "[OK]" : "[FAIL]");
        printf("[%s]HTTP: Received massage:\r\n\r\n", str);
        printf("%s", buffer);
    
}


int main() {
#if MBED_HEAP_STATS_ENABLED
    mbed_stats_heap_t heap_stats;
#endif

    printf("Start WiFi test \r\n");
     
    bool result = true;
    int rc = 0;

    printf("Start Connection ... \r\n");

    // WiFi part
    NetworkInterface *wifi_network_interface = WiFiInterface::get_default_instance();
    if (NULL == wifi_network_interface) {
        printf("NULL network interface! Exiting application....\r\n");
        return 0;
    }
    wifi_network_interface->set_default_parameters();
    printf("\n\rUsing WiFi \r\n");
    printf("\n\rConnecting to WiFi..\r\n");
    rc = wifi_network_interface->connect();
    if(rc == 0) {
        printf("\n\rConnected to Network successfully\r\n");
    } else {
        printf("\n\rConnection to Network Failed %d! Exiting application....\r\n", rc);
        return 0;
    }    
    printf("WiFi TCP client IP Address is %s\r\n", wifi_network_interface->get_ip_address());
    TCPSocket wifi_sock(wifi_network_interface);

    //Eth part
    NetworkInterface *eth_network_interface = EthernetInterface::get_default_instance();
    
    eth_network_interface->connect(); //Use DHCP
        
    printf("Eth TCP client IP Address is %s\r\n", eth_network_interface->get_ip_address());

    TCPSocket eth_sock(eth_network_interface);


    printf("ETH HTTP Connection ... \r\n");
    if (eth_sock.connect(HTTP_SERVER_NAME, HTTP_SERVER_PORT) == 0) {
        printf("ETH HTTP: Connected to %s:%d\r\n", HTTP_SERVER_NAME, HTTP_SERVER_PORT);
    }
    printf("WiFi HTTP Connection ... \r\n");
    if (wifi_sock.connect(HTTP_SERVER_NAME, HTTP_SERVER_PORT) == 0) {
        printf("WiFi HTTP: Connected to %s:%d\r\n", HTTP_SERVER_NAME, HTTP_SERVER_PORT);
    }

    while(1) {
        printf("Test Eth TCP client\r\n");    
        http_test("Eth",&eth_sock);
        wait(2);
        printf("Test WiFi TCP client\r\n");
        http_test("WiFi",&wifi_sock);
        wait(2);
    }
#if MBED_HEAP_STATS_ENABLED
    mbed_stats_heap_get(&heap_stats);
    printf("Current heap: %lu\r\n", heap_stats.current_size);
    printf("Max heap size: %lu\r\n", heap_stats.max_size);
#endif
    while(1) {
        int in_char = getchar();
        if (in_char == 'h') {
            mbed_stats_heap_get(&heap_stats);
            printf("Current heap: %lu\r\n", heap_stats.current_size);
            printf("Max heap size: %lu\r\n", heap_stats.max_size);
            continue;
        } else if (in_char == 'q') {
            break;
        }
    }

    printf(" Close socket & disconnect ... \r\n");
    eth_sock.close();
    wifi_sock.close();
    
    wifi_network_interface->disconnect();
    eth_network_interface->disconnect();
    printf(" End \r\n");
}
