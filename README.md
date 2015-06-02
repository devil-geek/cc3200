# Mode Device SDK for CC3200 LaunchPad on Energia

See the first [setup](http://energia.nu/pin-maps/guide_cc3200launchpad/) for CC3200 LaunchPad.

# How to install

unzip `ModeDeviceCC3200.zip` file under your Energia `Skeckbook location`/library.
For more detail, read [the guide](http://energia.nu/Guide_Environment.html#libraries).

Next, you need to change WiFiClient.h in Enerigia. You need to find "private:" in WiFiClient.h to "protected:". The file is under the following location.

| OS           | Location                                                                                      |
| -------------:----------------------------------------------------------------------------------------------:|
| MacOS        | /Applications/Energia.app/Contents/Resources/Java/hardware/cc3200/libraries/WiFi/WiFiClient.h |
| -------------:----------------------------------------------------------------------------------------------:|
| Windows      | `Extracted Energia directory`\hardware\cc3200\libraries\WiFi\WiFiClient.h                     |
| -------------:----------------------------------------------------------------------------------------------:|
| Linux        | `Extracted Energia directory`/hardware/cc3200/libraries/WiFi/WiFiClient.h                     |


You can find the source code like following,

~~~
     boolean sslIsVerified;
     
private:
     int _socketIndex;
     uint8_t rx_buffer[TCP_RX_BUFF_MAX_SIZE] = {0};
~~~

Then, please change like this

~~~
     boolean sslIsVerified;
     
protected:  // changed this line.
     int _socketIndex;
     uint8_t rx_buffer[TCP_RX_BUFF_MAX_SIZE] = {0};
~~~

To run the demo program, please open examples/ModeDeviceDemo_raw.ino file on Energia.

- Change your network SSID and password.
- Change Mode Device API key and device ID.

Run the demo program and see Serial console. You will see the following message.

~~~
Attempting to connect to Network named: .
You're connected to the network
Waiting for an ip address
.
Got IP Address: 10.1.10.106
Sent event
Going to busy loop() and waiting for commands...
~~~

# File structures
## License
- LICENSES
Please read the file.

## Main driver files
- ModeDevice.cpp
- ModeDevice.h

We will keep the class name `ModeDevice` and public method definition as much as possible.
Private methods, properties and other logic may vary in the future.

## Example
- /examples

Under the folder, you can find demo example to use Mode Cloud.

## SSL cert register for Mode.
- ModeWiFiClient.cpp
- ModeWiFiClient.h

The current WiFiClient class in Energia doesn't support SSL and we want to control root CA cert. That's why we require to access to properties in WiFiClass.
Once Energia WiFiClient class supports SSL officially, ModeWiFiClient class will be gone.

## HTTP client lib
- HttpClient.cpp
- HttpClient.h

Thanks to [Aurduio HttpClient project](https://github.com/amcewen/HttpClient). We forked it and tweaked a bit to support SSL. This will be deprecated.

## Logging
- SimpleLog.h

If you want to get more logging information, define DUMP_DEBUG_LOG.

## WebSocket lib
- Base64.cpp
- Base64.h
- WebSocketClient.cpp
- WebSocketClient.h
- global.h
- sha1.cpp
- sha1.h

Thanks to [Aurduio WebSocket project](https://github.com/brandenhall/Arduino-Websocket).
We forked it and tweaked a bit to support SSL and Authentication header. This will be deprecated.

