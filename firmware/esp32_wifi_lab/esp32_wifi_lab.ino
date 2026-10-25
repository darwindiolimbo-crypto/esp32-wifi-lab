#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* AP_NAME = "ESP32-WiFi-Lab";
const char* AP_PASSWORD = "LabSetup8266";

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Wi-Fi Lab</title>
<style>
body{margin:0;padding:18px;background:#101827;color:#edf2f7;font-family:system-ui,sans-serif}
main{max-width:700px;margin:auto}
section{background:#1b2739;border:1px solid #34435a;padding:16px;border-radius:12px;margin:14px 0}
button{width:100%;padding:12px;margin:6px 0;border:0;border-radius:8px;background:#3275d9;color:white;font-size:1rem}
pre{white-space:pre-wrap;overflow-wrap:anywhere}
</style>
</head>
<body><main>
<h1>ESP32 Wi-Fi Lab</h1>
<section>
<h2>Device status</h2>
<button onclick="status()">Refresh status</button>
<pre id="status">Loading...</pre>
</section>
<section>
<h2>Nearby networks</h2>
<button onclick="scan()">Scan Wi-Fi</button>
<pre id="networks">Ready to scan.</pre>
</section>
<script>
async function status(){
try {
const r=await fetch('/api/status');
document.getElementById('status').textContent=await r.text();
} catch(e) {
document.getElementById('status').textContent='Device unavailable';
}
}
async function scan(){
const el=document.getElementById('networks');
el.textContent='Scanning...';
try {
const r=await fetch('/api/scan');
const data=await r.json();
el.textContent=data.length?data.map(n=>n.ssid+' | '+n.rssi+' dBm | '+n.security).join('\n'):'No networks found.';
} catch(e) {
el.textContent='Scan failed.';
}
}
status();
</script>
</main></body></html>
)HTML";

String escapeJson(const String& s) {
  String out;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') out += '\\';
    if (c == '\n') { out += "\\n"; continue; }
    if (c == '\r') { out += "\\r"; continue; }
    if ((uint8_t)c < 32) continue;
    out += c;
  }
  return out;
}

void handleScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    json += "{\"ssid\":\"" + escapeJson(WiFi.SSID(i)) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"security\":\"";
    json += WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Protected";
    json += "\"}";
  }
  json += "]";
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

void handleStatus() {
  String s = "Dashboard IP: " + WiFi.softAPIP().toString();
  s += "\nConnected network: ";
  s += WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Not connected";
  server.send(200, "text/plain", s);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_NAME, AP_PASSWORD);

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", PAGE);
  });
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.begin();

  Serial.println("ESP32 Wi-Fi Lab ready");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
