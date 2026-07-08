#pragma once

#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "AppTypes.h"

class CaptivePortal {
 public:
  CaptivePortal();

  void begin(const String& apSsid);
  void stop();
  void loop();
  bool isRunning() const;

  void setStatusMessage(const String& message);
  bool consumeProvisioningRequest(ProvisioningRequest& request);

 private:
  void configureRoutes();
  void handleRoot();
  void handleScan();
  void handleConnect();
  void handleStatus();
  void handleNotFound();

  DNSServer dns_;
  ESP8266WebServer server_;
  bool running_ = false;
  String statusMessage_ = "Aguardando configuracao";
  ProvisioningRequest request_;
};
