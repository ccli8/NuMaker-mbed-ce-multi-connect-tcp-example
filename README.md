# Getting started with WiFi on mbed OS


### Import NuMaker-mbed-wifi-tcp from on-line IDE
1. Please choose Nuvoton NuMaker-PFM-XXX as your target platform.
2. Please press the left-up icon "New", then choose "WiFi TCP Example" from the template list.
3. After new your WiFi program, please also import ESP8266 lib by pressing the left-up icon "Import".
4. Import from URL "https://github.com/ARMmbed/esp8266-driver/"
5. If the last version of ESP8266 can't work well, please switch to ver. 6:450cc12 .

### Adjust Your WiFi Setting in mbed_app.json
1. wifi-ssid: fill your WiFi AP SSID
2. wifi-password: fill the WiFi password

#### Now compile
Please press compile icon.

#### Burn Code & Execute
1. Connect NuMaker-PFM-XXX with your PC by USB cable, then there will be one "mbed" disk.
2. Drag & drop the built binary into "mbed" disk on you PC.
3. Press device's reset button and you could get the below log by uart console.
-------------------------------------------------------------------------------
 Start WiFi test
 Start Connection ...

Using WiFi

Connecting to WiFi..

Connected to Network successfully
TCP client IP Address is 192.168.43.68
 HTTP Connection ...
HTTP: Connected to developer.mbed.org:80
HTTP: Received 461 chars from server
HTTP: Received 200 OK status ... [OK]
HTTP: Received 'Hello world!' status ... [OK]
HTTP: Received massage:

HTTP/1.1 200 OK
Server: nginx/1.7.10
Date: Thu, 29 Sep 2016 05:55:06 GMT
Content-Type: text/plain
Content-Length: 14
Connection: close
Last-Modified: Fri, 27 Jul 2012 13:30:34 GMT
Accept-Ranges: bytes
Cache-Control: max-age=36000
Expires: Thu, 29 Sep 2016 15:55:06 GMT
X-Upstream-L3: 172.17.0.4:80
X-Upstream-L2: developer-sjc-cyan-1-nginx
X-Upstream-L1-next-hop: 217.140.101.22:8001
X-Upstream-L1: developer-sjc-cyan-border-nginx

Hello world!
 Close socket & disconnect ...
 End
--------------------------------------------------------------------------