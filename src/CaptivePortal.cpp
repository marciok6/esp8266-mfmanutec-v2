#include "../include/CaptivePortal.h"

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
    .summary { display: none; margin-top: 24px; border-top: 1px solid rgba(255,255,255,0.12); padding-top: 20px; }
    .diagnostics { display: none; margin-top: 24px; border-top: 1px solid rgba(255,255,255,0.12); padding-top: 20px; }
    .row { display: flex; justify-content: space-between; gap: 12px; padding: 8px 0; border-bottom: 1px solid rgba(255,255,255,0.06); }
    .label { color: #cbd5e1; }
    .value { font-weight: 700; text-align: right; }
    .pill { display: inline-block; padding: 6px 10px; border-radius: 999px; background: #991b1b; color: #fecaca; font-size: 0.85rem; font-weight: 700; }
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
      <div id="summary" class="summary">
        <h2>Configuracao recebida do servidor</h2>
        <div id="summaryRows"></div>
        <button onclick="finalizeConfig()">Concluir Configuracao</button>
      </div>
      <div id="diagnostics" class="diagnostics">
        <h2>Dashboard de diagnostico</h2>
        <p class="muted">O dispositivo entrou em modo AP por falha operacional. Os dados abaixo ajudam na analise.</p>
        <div id="errorBadge" class="pill"></div>
        <div id="diagnosticRows" style="margin-top:16px;"></div>
      </div>
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
      document.getElementById('summary').style.display = 'none';
      document.getElementById('diagnostics').style.display = 'none';
      if (data.summary && data.summary.ready) {
        const fields = [
          ['Chip ID', data.summary.chip_id],
          ['MAC', data.summary.mac],
          ['IP', data.summary.ip],
          ['Cliente', data.summary.cliente],
          ['Descricao', data.summary.descricao],
          ['Local de instalacao', data.summary.local_instalacao],
          ['Intervalo de envio', `${data.summary.expected_interval}s`],
          ['Temperatura minima', `${data.summary.min_temperature} C`],
          ['Temperatura maxima', `${data.summary.max_temperature} C`],
          ['Estado do Rele', data.summary.relay_state ? 'Ligado' : 'Desligado'],
          ['Status', data.summary.status]
        ];
        document.getElementById('summaryRows').innerHTML = fields.map(([label, value]) =>
          `<div class="row"><span class="label">${label}</span><span class="value">${value ?? ''}</span></div>`).join('');
        document.getElementById('summary').style.display = 'block';
      }
      if (data.mode === 'diagnostics' && data.diagnostic && data.diagnostic.ready) {
        const diag = data.diagnostic;
        const fields = [
          ['Motivo', diag.reason],
          ['Chip ID', diag.chip_id],
          ['MAC', diag.mac],
          ['SSID AP', diag.ap_ssid],
          ['SSID Wi-Fi salvo', diag.station_ssid],
          ['IP da estacao', diag.station_ip],
          ['Cliente', diag.cliente],
          ['Descricao', diag.descricao],
          ['Local de instalacao', diag.local_instalacao],
          ['Status do dispositivo', diag.device_status],
          ['Temperatura', `${diag.temperature} C`],
          ['RSSI', `${diag.wifi_rssi} dBm`],
          ['Tensao', `${diag.voltage} V`],
          ['Estado do rele', diag.relay_state ? 'Ligado' : 'Desligado'],
          ['Uptime', `${diag.uptime}s`],
          ['API', diag.api_endpoint]
        ];
        document.getElementById('errorBadge').innerText = `Codigo ${diag.code}`;
        document.getElementById('diagnosticRows').innerHTML = fields.map(([label, value]) =>
          `<div class="row"><span class="label">${label}</span><span class="value">${value ?? ''}</span></div>`).join('');
        document.getElementById('diagnostics').style.display = 'block';
        document.getElementById('step2').style.display = 'none';
      }
    }
    async function finalizeConfig() {
      document.getElementById('status').innerText = 'Salvando configuracao e reiniciando...';
      await fetch('/finalize', { method: 'POST' });
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
  mode_ = PortalMode::Provisioning;
  summary_ = ProvisioningSummary{};
  diagnostic_ = DiagnosticSummary{};
  finalizeRequested_ = false;
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

void CaptivePortal::setPortalMode(PortalMode mode) { mode_ = mode; }

void CaptivePortal::setProvisioningSummary(const ProvisioningSummary& summary) { summary_ = summary; }

void CaptivePortal::clearProvisioningSummary() { summary_ = ProvisioningSummary{}; }

void CaptivePortal::setDiagnosticSummary(const DiagnosticSummary& summary) { diagnostic_ = summary; }

void CaptivePortal::clearDiagnosticSummary() { diagnostic_ = DiagnosticSummary{}; }

bool CaptivePortal::consumeProvisioningRequest(ProvisioningRequest& request) {
  if (!request_.submitted) {
    return false;
  }
  request = request_;
  request_ = ProvisioningRequest{};
  return true;
}

bool CaptivePortal::consumeFinalizeRequest() {
  if (!finalizeRequested_) {
    return false;
  }
  finalizeRequested_ = false;
  return true;
}

void CaptivePortal::configureRoutes() {
  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/scan", HTTP_GET, [this]() { handleScan(); });
  server_.on("/connect", HTTP_POST, [this]() { handleConnect(); });
  server_.on("/finalize", HTTP_POST, [this]() { handleFinalize(); });
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
  summary_ = ProvisioningSummary{};
  statusMessage_ = request_.submitted ? "Credenciais recebidas. Processando..." : "SSID invalido";
  server_.send(200, "application/json", "{\"accepted\":true}");
}

void CaptivePortal::handleFinalize() {
  finalizeRequested_ = true;
  statusMessage_ = "Finalizando configuracao...";
  server_.send(200, "application/json", "{\"accepted\":true}");
}

void CaptivePortal::handleStatus() {
  DynamicJsonDocument json(1536);
  json["message"] = statusMessage_;
  json["mode"] = mode_ == PortalMode::Diagnostics ? "diagnostics" : "provisioning";
  JsonObject summary = json.createNestedObject("summary");
  summary["ready"] = summary_.ready;
  if (summary_.ready) {
    summary["chip_id"] = summary_.chipId;
    summary["mac"] = summary_.mac;
    summary["ip"] = summary_.ip;
    summary["cliente"] = summary_.cliente;
    summary["descricao"] = summary_.descricao;
    summary["local_instalacao"] = summary_.localInstalacao;
    summary["status"] = summary_.status;
    summary["expected_interval"] = summary_.expectedInterval;
    summary["min_temperature"] = summary_.minTemperature;
    summary["max_temperature"] = summary_.maxTemperature;
    summary["relay_state"] = summary_.relayState;
  }
  JsonObject diagnostic = json.createNestedObject("diagnostic");
  diagnostic["ready"] = diagnostic_.ready;
  if (diagnostic_.ready) {
    diagnostic["code"] = diagnostic_.code;
    diagnostic["reason"] = diagnostic_.reason;
    diagnostic["chip_id"] = diagnostic_.chipId;
    diagnostic["mac"] = diagnostic_.mac;
    diagnostic["ap_ssid"] = diagnostic_.apSsid;
    diagnostic["station_ssid"] = diagnostic_.stationSsid;
    diagnostic["station_ip"] = diagnostic_.stationIp;
    diagnostic["api_endpoint"] = diagnostic_.apiEndpoint;
    diagnostic["device_status"] = diagnostic_.deviceStatus;
    diagnostic["cliente"] = diagnostic_.cliente;
    diagnostic["descricao"] = diagnostic_.descricao;
    diagnostic["local_instalacao"] = diagnostic_.localInstalacao;
    diagnostic["temperature"] = diagnostic_.temperature;
    diagnostic["wifi_rssi"] = diagnostic_.wifiRssi;
    diagnostic["voltage"] = diagnostic_.voltage;
    diagnostic["relay_state"] = diagnostic_.relayState;
    diagnostic["uptime"] = diagnostic_.uptime;
  }
  String response;
  serializeJson(json, response);
  server_.send(200, "application/json", response);
}

void CaptivePortal::handleNotFound() {
  server_.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server_.send(302, "text/plain", "");
}
