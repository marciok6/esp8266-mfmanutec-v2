#include "CaptivePortal.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

namespace {
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>MFMANUTEC Setup</title>
  <style>
    body { font-family: Georgia, serif; margin: 0; background: linear-gradient(135deg, #0f172a, #1e3a8a); color: #f8fafc; }
    .wrap { max-width: 720px; margin: 0 auto; padding: 32px 20px 64px; }
    .card { background: rgba(15, 23, 42, 0.84); border: 1px solid rgba(255,255,255,0.14); border-radius: 20px; padding: 24px; box-shadow: 0 20px 40px rgba(0,0,0,0.25); }
    h1 { margin-top: 0; font-size: 2rem; }
    button { background: #f59e0b; color: #111827; border: 0; border-radius: 999px; padding: 12px 18px; font-weight: 700; cursor: pointer; }
    input, select { width: 100%; padding: 12px; margin: 8px 0 16px; border-radius: 12px; border: 1px solid #475569; background: #0f172a; color: #f8fafc; }
    .muted { color: #cbd5e1; }
    .network { display: flex; justify-content: space-between; gap: 12px; padding: 10px 0; border-bottom: 1px solid rgba(255,255,255,0.08); }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>Bem-vindo</h1>
      <p class="muted">Vamos configurar seu dispositivo.</p>
      <button onclick="loadNetworks()">Iniciar Configuracao</button>
      <div id="step2" style="display:none; margin-top:24px;">
        <h2>Redes Wi-Fi</h2>
        <div id="networks"></div>
        <label for="ssid">SSID</label>
        <input id="ssid" autocomplete="off">
        <label for="password">Senha</label>
        <input id="password" type="password" autocomplete="new-password">
        <button onclick="submitConfig()">Conectar</button>
      </div>
      <div id="status" style="margin-top:24px;"></div>
    </div>
  </div>
  <script>
    async function loadNetworks() {
      document.getElementById('step2').style.display = 'block';
      document.getElementById('networks').innerHTML = 'Buscando redes...';
      const response = await fetch('/scan');
      const data = await response.json();
      const html = data.networks.map(n => `<div class="network"><span>${n.ssid}</span><span>${n.rssi} dBm</span></div>`).join('');
      document.getElementById('networks').innerHTML = html || 'Nenhuma rede encontrada';
      if (data.networks.length > 0) {
        document.getElementById('ssid').value = data.networks[0].ssid;
      }
    }
    async function submitConfig() {
      const payload = new URLSearchParams();
      payload.append('ssid', document.getElementById('ssid').value);
      payload.append('password', document.getElementById('password').value);
      document.getElementById('status').innerText = 'Conectando ao Wi-Fi...';
      await fetch('/connect', { method: 'POST', body: payload });
    }
    async function pollStatus() {
      const response = await fetch('/status');
      const data = await response.json();
      document.getElementById('status').innerText = data.message;
    }
    setInterval(pollStatus, 1500);
  </script>
</body>
</html>
)rawliteral";
}

CaptivePortal::CaptivePortal() : server_(80) {}

void CaptivePortal::begin(const String& apSsid) {
  request_ = ProvisioningRequest{};
  statusMessage_ = "Aguardando configuracao";
  dns_.start(53, "*", IPAddress(192, 168, 4, 1));
  configureRoutes();
  server_.begin();
  running_ = true;
}

void CaptivePortal::stop() {
  dns_.stop();
  server_.stop();
  running_ = false;
}

void CaptivePortal::loop() {
  if (!running_) {
    return;
  }
  dns_.processNextRequest();
  server_.handleClient();
}

bool CaptivePortal::isRunning() const { return running_; }

void CaptivePortal::setStatusMessage(const String& message) { statusMessage_ = message; }

bool CaptivePortal::consumeProvisioningRequest(ProvisioningRequest& request) {
  if (!request_.submitted) {
    return false;
  }
  request = request_;
  request_ = ProvisioningRequest{};
  return true;
}

void CaptivePortal::configureRoutes() {
  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/scan", HTTP_GET, [this]() { handleScan(); });
  server_.on("/connect", HTTP_POST, [this]() { handleConnect(); });
  server_.on("/status", HTTP_GET, [this]() { handleStatus(); });
  server_.onNotFound([this]() { handleNotFound(); });
}

void CaptivePortal::handleRoot() { server_.send_P(200, "text/html", PAGE); }

void CaptivePortal::handleScan() {
  DynamicJsonDocument json(1024);
  JsonArray networks = json.createNestedArray("networks");
  int count = WiFi.scanNetworks();
  for (int index = 0; index < count; ++index) {
    JsonObject entry = networks.createNestedObject();
    entry["ssid"] = WiFi.SSID(index);
    entry["rssi"] = WiFi.RSSI(index);
  }
  String response;
  serializeJson(json, response);
  server_.send(200, "application/json", response);
}

void CaptivePortal::handleConnect() {
  request_.ssid = server_.arg("ssid");
  request_.password = server_.arg("password");
  request_.submitted = !request_.ssid.isEmpty();
  statusMessage_ = request_.submitted ? "Credenciais recebidas. Processando..." : "SSID invalido";
  server_.send(200, "application/json", "{\"accepted\":true}");
}

void CaptivePortal::handleStatus() {
  DynamicJsonDocument json(256);
  json["message"] = statusMessage_;
  String response;
  serializeJson(json, response);
  server_.send(200, "application/json", response);
}

void CaptivePortal::handleNotFound() {
  server_.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server_.send(302, "text/plain", "");
}
