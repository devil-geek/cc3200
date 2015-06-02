#ifndef __MODEWIFI_CLIENT_H__
#define __MODEWIFI_CLIENT_H__

#include "WiFiClient.h"

class ModeWiFiClient : public WiFiClient {
  static bool m_ssl_checked;

  public:
  virtual int sslConnect(IPAddress ip, uint16_t port);
  virtual int32_t sslGetReasonID();
  virtual const char *sslGetReason();

  // If you want to use SSL connection, you have to set the current time to device to check SSL cert expiration.
  // You have to call this function after WiFi.begin(), otherwise it would be stuck.
  static int setDeviceTime(int year, int mon, int day, int hour, int min, int sec);

  // This is not strictly thread safe, so make sure this func is called only when init in multi thread env.
  // You don't have to call by yourself if it's not multithread environment, because sslConnect() takes care of it.
  static bool checkAndCreateRootCACert();

};

#endif
