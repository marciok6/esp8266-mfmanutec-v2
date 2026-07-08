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
  void setPortalMode(PortalMode mode);
  void setProvisioningSummary(const ProvisioningSummary& summary);
  void clearProvisioningSummary();
  void setDiagnosticSummary(const DiagnosticSummary& summary);
  void clearDiagnosticSummary();
  bool consumeProvisioningRequest(ProvisioningRequest& request);
  bool consumeFinalizeRequest();

 private:
  void configureRoutes();
  void handleRoot();
  void handleScan();
  void handleConnect();
  void handleFinalize();
  void handleStatus();
  void handleNotFound();

  DNSServer dns_;
  ESP8266WebServer server_;
  bool running_ = false;
  bool finalizeRequested_ = false;
  PortalMode mode_ = PortalMode::Provisioning;
  String statusMessage_ = "Aguardando configuracao";
  ProvisioningRequest request_;
  ProvisioningSummary summary_;
  DiagnosticSummary diagnostic_;
};
