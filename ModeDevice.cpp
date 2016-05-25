#include <Arduino.h>
#include <IPAddress.h>
#include <ModeDevice.h>
#include <Time.h>
#include "HttpClient.h"
#include "WebSocketClient.h"
#include "SimpleLog.h"

// Use SSL to communite with MODE APIs
#define MODE_HOST (char*)ModeDevice::m_APIHost.c_str()
#define MODE_PORT 443
#define USE_SSL true

#define MODE_CLOUD "ModeCloud"

#define PATH_DEVICES "/devices/"
#define PATH_EVENT "/event"
#define PATH_COMMAND "/command"

#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY 5000

#define PING_TIMEOUT (20*1000)

ModeDevice::ModeDevice(const char* token, const char* device_id) :
m_APIHost("api.tinkermode.com"),
m_websocket(new WebSocketClient),
m_reconnect_timestamp(0),
m_ping_timestamp(0),
m_fib1(1),
m_fib2(1),
m_command_callback(NULL),
m_ping_callback(NULL),
m_pong_callback(NULL),
m_close_callback(NULL),
m_open_callback(NULL)
{
  m_command_path = String(PATH_DEVICES);
  m_command_path += device_id;
  m_command_path += PATH_COMMAND;

  m_event_path = PATH_DEVICES;
  m_event_path += device_id;
  m_event_path += PATH_EVENT;

  m_auth = MODE_CLOUD;
  m_auth += " ";
  m_auth += token;
}

ModeDevice::~ModeDevice() {
}

void ModeDevice::setAPIHost(const char* host) {
  m_APIHost = host;
}

const char* ModeDevice::getAPIHost() {
  return m_APIHost.c_str();
}

// Register the callback function to be called when a command is delivered.
CommandCallback ModeDevice::setCommandCallback(CommandCallback callback) {
  CommandCallback rval = m_command_callback;
  m_command_callback = callback;
  return rval;
}

// Register the callback function to be called when a ping packet is delivered.
PingCallback ModeDevice::setPingCallback(PingCallback callback) {
  PingCallback rval = m_ping_callback;
  m_ping_callback = callback;
  return rval;
}

// Register the callback function to be called when a pong packet is delivered.
PongCallback ModeDevice::setPongCallback(PongCallback callback) {
  PongCallback rval = m_pong_callback;
  m_pong_callback = callback;
  return rval;
}

// Register the callback function to be called when a close packet is delivered.
CloseCallback ModeDevice::setCloseCallback(CloseCallback callback) {
  CloseCallback rval = m_close_callback;
  m_close_callback = callback;
  return rval;
}

void ModeDevice::resetReconnectSchedule() {
  m_fib1 = 1;
  m_fib2 = 1;
}

int ModeDevice::bumpNextReconnectSchedule() {
  int rval = m_fib2;
  if (rval >= 60) {
    // Set the cap to 60 seconds.
    m_fib2 = 60;
    return m_fib2;
  }
  m_fib2 = m_fib1 + m_fib2;
  m_fib1 = rval;
  return rval;
}

int ModeDevice::getNextReconnectSchedule() {
  return m_fib2 * 1000;
}

int ModeDevice::triggerEvent(const String* event) {
  HttpClient http(m_client, USE_SSL);

  http.beginRequest();
  int err = http.put(MODE_HOST, MODE_PORT, m_event_path.c_str());
  if (err == 0) {
    DEBUG_LOGN("startedRequest ok");

    http.sendHeader("Content-Length", String(event->length(), 10).c_str());
    http.sendHeader("Authorization", m_auth.c_str());
    http.sendHeader("Content-Type", "application/json");
    http.endRequest();

    http.write((const uint8_t*)event->c_str(), event->length());
    err = http.responseStatusCode();
    if (err >= 0) {
      DEBUG_LOG("Got status code: ");
      DEBUG_LOGN(err);

      err = http.skipResponseHeaders();
      if (err >= 0) {
        int bodyLen = http.contentLength();
        DEBUG_LOG("Content length is: ");
        DEBUG_LOGN(bodyLen);
        DEBUG_LOGN();
        DEBUG_LOGN("Body returned follows:");

        unsigned long timeoutStart = millis();
        char c;
        while (
            (http.connected() || http.available()) &&
            ((millis() - timeoutStart) < NETWORK_TIMEOUT) ) {
          if (http.available()) {
            c = http.read();
            DEBUG_LOG(c);
            bodyLen--;
            timeoutStart = millis();
          } else {
            delay(NETWORK_DELAY);
          }
        }
      } else {
        ERROR_LOG("Failed to skip response headers: ");
        ERROR_LOGN(err);
      }
    } else {    
      ERROR_LOG("Getting response failed: ");
      ERROR_LOGN(err);
    }
  } else {
    ERROR_LOG("Connect failed: ");
    ERROR_LOGN(err);
  }
  http.stop();

  return err;
}

int ModeDevice::handshakeForCommand() {
  if (USE_SSL) {
    Client* client = &m_websocket_client;
    client->sslConnect(MODE_HOST, MODE_PORT);
  } else {
    m_websocket_client.connect(MODE_HOST, MODE_PORT);
  }
  DEBUG_LOGN("Connected");

  m_websocket->path = (char*)m_command_path.c_str();
  m_websocket->host = MODE_HOST;
  m_websocket->m_auth = m_auth;

  if (m_websocket->handshake(m_websocket_client)) {
    DEBUG_LOGN("Handshake successful");
    resetReconnectSchedule();
  } else {
    ERROR_LOGN("Handshake failed.");
    return -1;
  }
  return 0;
}

int defaultCallback(String* data, uint8_t opcode) {
  DEBUG_LOG("Got Opcode 0x");
  DEBUG_LOG(String(opcode, HEX).c_str());
  DEBUG_LOG(" but callback is not identified : ");
  DEBUG_LOGN(data->c_str());
  return data->length();
}

int ModeDevice::checkAndTriggerPing() {
  if (millis() - m_ping_timestamp > PING_TIMEOUT) {
    m_websocket->sendData("", WS_OPCODE_PING);
    m_ping_timestamp = millis();
  }
}

int ModeDevice::dispatchCallback() {
  checkAndTriggerPing();

  String data;
  uint8_t opcode;
  m_websocket->getData(data, &opcode);
  opcode &= 0xf; // Only 4 bits are opcode.
  if (opcode == 0) {
    return 0;
  }
  switch(opcode){
    case WS_OPCODE_TEXT:
      if (m_command_callback) {
        if (data.length()) {
          return (*m_command_callback)(&data);
        } else {
          return 0;
        }
      } else {
        return defaultCallback(&data, opcode);
      }
      break;
    case WS_OPCODE_CLOSE:
      return m_close_callback ? (*m_close_callback)(&data) : defaultCallback(&data, opcode);
    case WS_OPCODE_PING:
      return m_ping_callback ? (*m_ping_callback)(&data) : defaultCallback(&data, opcode);
    case WS_OPCODE_PONG:
      return m_pong_callback ? (*m_pong_callback)(&data) : defaultCallback(&data, opcode);
    default:
      return defaultCallback(&data, opcode);
  }
}

int ModeDevice::processCommand() {
  if (m_websocket_client.connected()) {
    return dispatchCallback();
  } else {
    String command;
    uint8_t opcode;
    if (millis() - m_reconnect_timestamp > getNextReconnectSchedule()) {
      DEBUG_LOGN("Reconnecting...");
      m_reconnect_timestamp = millis();
      bumpNextReconnectSchedule();
      if (handshakeForCommand() == 0) {
        DEBUG_LOGN("Handshake succeeded.");
        resetReconnectSchedule();
        return dispatchCallback();
      } else {
        ERROR_LOGN("Handshake failed.");
      }
    }
    return -1;
  }
  return 0;
}

