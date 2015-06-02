/*
  ModeDeviceDemo
 
  This is the example to how to use ModeDevice class for CC3200 on Energia.
  1. Connects to WiFi
  2. Creates a ModeDevice instance with API key and Device ID
  3. Sends event with triggerEvent method call
  4. Wait for commands from MODE Cloud in busy loop and process the command in commandCallback

  The MIT License (MIT)

  Copyright (c) 2015 MODE, Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

 */

#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad.
#include <SPI.h>
#endif
#include <WiFi.h>
#include "ModeDevice.h"

// Your network SSID and password.
char ssid[] = "your_network_ssid";
char password[] = "your_network_password";

// You device API key and device id.
const char api_key[] = "YOUR_OWN_API_KEY";
const char device_id[] = "12345678"; // You need to replace with your MODE device id.

ModeDevice device(api_key, device_id);

//
// This is callback for commands from MODE.
// You can register the function with setCommandCallback.
// 

int commandCallback(String* command) {
  Serial.println(command->c_str());
  return command->length();
}

int pongCallback(String* command) {
  Serial.println(command->c_str());
  return command->length();
}

void setup() {
  //Initialize serial and wait for port to open.
  Serial.begin(115200);

  // Attempt to connect to Wifi network.
  Serial.print("Attempting to connect to Network named: ");
  WiFi.begin(ssid, password);

  // You need to call this function after WiFi.begin(), otherwise it will become stuck
  // For production code, you need to get the current time from a reliable source (such as NTP).
  ModeWiFiClient::setDeviceTime(2016, 6, 1, 0, 0, 1);

  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    Serial.print(".");
    delay(300);
  }

  Serial.print("\nGot IP Address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // Send event with JSON string.
  // You can follow the schema like {"eventType": "foo", "eventData": {"bar": "hoge" }}.
  String event("{\"eventType\": \"light\", \"eventData\": {\"on\": 1}}");
  device.triggerEvent(&event);
  Serial.println("Sent event");

  // You can call handshakeForCommand once if you want to receive commands.
  device.handshakeForCommand();

  // You can register commandCallback to process received commands.
  device.setCommandCallback(commandCallback);

  // Periodically Ping data is sent through WebSocket to keep alive.
  // And Pong will be returned from server. It's up to users.
  device.setPongCallback(pongCallback);

  Serial.println("Going to busy loop() and waiting for commands...");
}

void loop() {
  // You would need to insert delay to tune up power consumption.
  //  delay(100); 
  // You want to receive commands from MODE Cloud, you need to keep calling this function.
  // First, Energia doesn't support multi threading for CC3200 yet.
  // If we block (e.g. calling sl_Select in side), then CC3200 cannot run anything except receiving commands.
  // Inside processCommands(), cammandCallback function is called only when receiving.
  int rval = device.processCommand();
}


