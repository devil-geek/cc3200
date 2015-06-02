#ifndef __MODE_DEVICE_H__
#define __MODE_DEVICE_H__

#include <Arduino.h>
#include "String.h"
#include "ModeWiFiClient.h"

class Client;
class Serial;
class String;
class WebSocketClient;

typedef int (*CommandCallback)(String* command);
typedef int (*PongCallback)(String* command);
typedef int (*PingCallback)(String* command);
typedef int (*CloseCallback)(String* command);
typedef int (*OpenCallback)(String* command);

class ModeDevice {
  WebSocketClient* m_websocket;
  // I don't like to have instances here, but somehow WiFiClient new is prohibited :-(
  ModeWiFiClient m_websocket_client;
  ModeWiFiClient m_client;
  String m_event_path;
  String m_command_path;
  String m_auth;

  CommandCallback m_command_callback;
  PongCallback m_pong_callback;
  PingCallback m_ping_callback;
  CloseCallback m_close_callback;
  OpenCallback m_open_callback;

  int m_fib1;
  int m_fib2;
  int m_reconnect_timestamp;
  int m_ping_timestamp;

  void resetReconnectSchedule();
  int bumpNextReconnectSchedule();
  int getNextReconnectSchedule();

  int dispatchCallback();
  int checkAndTriggerPing();

  public:

  ModeDevice(const char* token, const char* deviceId);
  ~ModeDevice();

  int triggerEvent(const String* event);
  int handshakeForCommand();
  int processCommand();

  CommandCallback setCommandCallback(CommandCallback callback);
  PingCallback setPingCallback(PingCallback callback);
  PongCallback setPongCallback(PongCallback callback);
  CloseCallback setCloseCallback(CloseCallback callback);
};

#endif
