# Example for multiple TCP connections on Nuvoton's Mbed CE enabled boards

This is an example to demo multiple TCP connections over WiFi and Ethernet
on Nuvoton's Mbed CE enabled boards.

Check out [Mbed CE](https://github.com/mbed-ce)
for details on Mbed OS community edition.

## Support development tools

Use cmake-based build system.
Check out [hello world example](https://github.com/mbed-ce/mbed-ce-hello-world) for getting started.

> **⚠️ Warning**
>
> Legacy development tools below are not supported anymore.
> - [Arm's Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-studio.html)
> - [Arm's Mbed CLI 2](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-cli-2.html)
> - [Arm's Mbed CLI 1](https://os.mbed.com/docs/mbed-os/v6.15/tools/developing-mbed-cli.html)

For [VS Code development](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-VS-Code)
or [OpenOCD as upload method](https://github.com/mbed-ce/mbed-os/wiki/Upload-Methods#openocd),
install below additionally:

-   [NuEclipse](https://github.com/OpenNuvoton/Nuvoton_Tools#numicro-software-development-tools): Nuvoton's fork of Eclipse
-   Nuvoton forked OpenOCD: Shipped with NuEclipse installation package above.
    Checking openocd version `openocd --version`, it should fix to `0.10.022`.

## Developer guide

In the following, we take **NuMaker-IoT-M467** board as an example for Mbed CE support.

### Hardware requirements

-   NuMaker-IoT-M467 board
-   Host OS: Windows or others
-   Ethernet cable (RJ45)
-   WiFi ESP8266

### Build the example

1.  Clone the example and navigate into it
    ```
    $ git clone https://github.com/mbed-nuvoton/NuMaker-mbed-ce-multi-connect-tcp-example
    $ cd NuMaker-mbed-ce-multi-connect-tcp-example
    $ git checkout -f master
    ```

1.  Deploy necessary libraries
    ```
    $ git submodule update --init
    ```
    Or for fast install:
    ```
    $ git submodule update --init --filter=blob:none
    ```

1.  Compile with cmake/ninja
    ```
    $ mkdir build; cd build
    $ cmake .. -GNinja -DCMAKE_BUILD_TYPE=Develop -DMBED_TARGET=NUMAKER_IOT_M467
    $ ninja
    $ cd ..
    ```

### Flash the image

Flash by drag-n-drop built image `NuMaker-mbed-ce-multi-connect-tcp-example.bin` or `NuMaker-mbed-ce-multi-connect-tcp-example.hex` onto **NuMaker-IoT-M467** board

### Verify the result

On host terminal (115200/8-N-1), you should see:

```
Start Connection ...

Connecting to WiFi..

Connected to Network (WiFi) successfully
WiFi TCP client IP Address is 192.168.1.107

Connecting to ETH..
mac address f6-bf-01-0b-f0-06
PHY ID 1:0x1c
PHY ID 2:0xc816

Connected to Network (ETH) successfully
Eth TCP client IP Address is 192.168.0.100

WiFi HTTP Connection ...
WiFi HTTP: Connected to www.ifconfig.io:80
ETH HTTP Connection ...
ETH HTTP: Connected to www.ifconfig.io:80


Current heap: 4341
Max heap size: 4965

Test round 1 ...

Test WiFi TCP client
[WiFi]HTTP: Request message:
GET /method HTTP/1.1
Host: www.ifconfig.io



[WiFi]HTTP: Received 786 chars from server
[WiFi]HTTP: Received 200 OK status ... [OK]
[WiFi]HTTP: Received message:
HTTP/1.1 200 OK
Date: Thu, 26 Dec 2024 07:44:29 GMT
Content-Type: text/plain; charset=utf-8
Content-Length: 4
Connection: keep-alive
cf-cache-status: DYNAMIC
Report-To: {"endpoints":[{"url":"https:\/\/a.nel.cloudflare.com\/report\/v4?s=aDuZlS6MnzZue0%2BPg7J%2FpfCjeMba7h37Ux7v70LOlh%2B5t%2Bvzo%2BvHjq8TxG5lkmWj8QEXgt%2Bl5we1zsTvsdYrRsOs9nKqqWomnfwnuEru9eaBDITH5iwlMJP7d1woTmwGWIg%3D"}],"group":"cf-nel","max_age":604800}
NEL: {"success_fraction":0,"report_to":"cf-nel","max_age":604800}
Server: cloudflare
CF-RAY: 8f7f72251f618451-TPE
alt-svc: h3=":443"; ma=86400
server-timing: cfL4;desc="?proto=TCP&rtt=30068&min_rtt=30068&rtt_var=15034&sent=1&recv=3&lost=0&retrans=0&sent_bytes=0&recv_bytes=47&delivery_rate=0&cwnd=249&unsent_bytes=0&cid=0000000000000000&ts=0&x=0"

GET


Test Eth TCP client
[Eth]HTTP: Request message:
GET /method HTTP/1.1
Host: www.ifconfig.io



[Eth]HTTP: Received 780 chars from server
[Eth]HTTP: Received 200 OK status ... [OK]
[Eth]HTTP: Received message:
HTTP/1.1 200 OK
Date: Thu, 26 Dec 2024 07:44:31 GMT
Content-Type: text/plain; charset=utf-8
Content-Length: 4
Connection: keep-alive
cf-cache-status: DYNAMIC
Report-To: {"endpoints":[{"url":"https:\/\/a.nel.cloudflare.com\/report\/v4?s=VzRcGDcURxWffp2phmVYu51QWhkr%2FGtqk2KDZeG0AVWPgJ3f88Fwjyai%2Fw8GTSNDaG6gJKlv357FTqelXDVxsJxu5snuxATradH69HAE8V5Hg2nJn30Imsg8qZlF1GglH%2Bw%3D"}],"group":"cf-nel","max_age":604800}
NEL: {"success_fraction":0,"report_to":"cf-nel","max_age":604800}
Server: cloudflare
CF-RAY: 8f7f7234df374a1c-TPE
alt-svc: h3=":443"; ma=86400
server-timing: cfL4;desc="?proto=TCP&rtt=29940&min_rtt=29940&rtt_var=14970&sent=1&recv=3&lost=0&retrans=0&sent_bytes=0&recv_bytes=47&delivery_rate=0&cwnd=249&unsent_bytes=0&cid=0000000000000000&ts=0&x=0"

GET
```
