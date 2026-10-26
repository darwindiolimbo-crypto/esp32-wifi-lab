#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* AP_NAME = "Huawei@CSSproject";
const char* AP_PASSWORD = "Darwin_03+07";

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Wi-Fi Security Lab</title>
<style>
:root{color-scheme:dark;--bg:#101827;--panel:#1b2739;--line:#34435a;--blue:#3275d9;--text:#edf2f7;--muted:#bac6d8}
*{box-sizing:border-box}
body{margin:0;padding:16px;background:var(--bg);color:var(--text);font:16px system-ui,sans-serif}
main{max-width:900px;margin:auto}
h1{font-size:1.65rem}h2{font-size:1.15rem}
p{line-height:1.5;color:var(--muted)}
section{background:var(--panel);border:1px solid var(--line);padding:16px;border-radius:12px;margin:14px 0}
button,select,input{font:inherit;border-radius:8px;padding:11px;max-width:100%}
button{border:0;background:var(--blue);color:white;margin:4px 3px 4px 0;cursor:pointer}
input,select{background:var(--bg);color:var(--text);border:1px solid var(--line);width:100%;margin:4px 0}
pre{white-space:pre-wrap;overflow-wrap:anywhere;background:var(--bg);padding:12px;border-radius:8px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:12px}
.tag{display:inline-block;padding:4px 8px;border-radius:6px;background:#293b52;color:#cfe0ff}
.good{color:#8ee0b0}.warn{color:#ffd17a}.muted{color:var(--muted)}
label{display:block;margin:9px 0}
small{color:var(--muted)}
</style>
</head>
<body><main>
<h1>ESP32 Wi-Fi Security Lab</h1>
<p>Local wireless reconnaissance, security assessment and controlled lab exercises.</p>
<p><span class="tag" id="mode">Connecting to ESP32…</span></p>

<div class="grid">
<section>
<h2>Device status</h2>
<button onclick="getStatus()">Refresh status</button>
<pre id="status">Checking device…</pre>
</section>
<section>
<h2>Wi-Fi reconnaissance</h2>
<p>Scan nearby networks and inspect their advertised security and signal strength.</p>
<button onclick="scan()">Scan Wi-Fi</button>
<pre id="networks">No scan performed yet.</pre>
</section>
</div>

<section>
<h2>Selected network</h2>
<label for="target">Choose a network from the latest scan</label>
<select id="target"><option value="">No scan results yet</option></select>
<pre id="details">Select a network to see its scan details.</pre>
<p class="muted">The current firmware API reports SSID, RSSI and a basic open/protected classification. It does not report BSSID, channel, WPS support or a full WPA protocol classification.</p>
</section>

<section>
<h2>WPS security assessment</h2>
<p>WPS availability cannot be confirmed by the current scan API. Record evidence from the router's own administration page or configuration.</p>
<label>WPS status
<select id="wps">
<option value="Unknown">Unknown / not checked</option>
<option value="Enabled">Enabled</option>
<option value="Disabled">Disabled</option>
<option value="Unavailable">Not supported / unavailable</option>
</select></label>
<label>Evidence or notes
<input id="wpsNotes" placeholder="Router settings page, model, observation…"></label>
<p class="muted">Recommendation: disable WPS if it is not needed. Do not attempt PIN guessing against real access points.</p>
</section>

<section>
<h2>Attack &amp; Penetration Lab</h2>
<p>This section provides controlled test planning and offline scenario demonstrations. These buttons do not transmit attacks or crack real Wi-Fi credentials.</p>
<label>Exercise
<select id="scenario">
<option value="password">Weak password risk</option>
<option value="wps">WPS exposure risk</option>
<option value="deauth">Deauthentication and disruption risk</option>
<option value="evil-twin">Rogue access point / evil twin risk</option>
<option value="monitoring">Wireless monitoring and detection</option>
</select></label>
<button onclick="runScenario()">Run offline scenario</button>
<pre id="scenarioResult">Choose a scenario and run the offline demonstration.</pre>
<div class="grid">
<div>
<h3>Controlled test checklist</h3>
<label><input type="checkbox" class="check" value="Scope and owner permission confirmed"> Scope and owner permission confirmed</label>
<label><input type="checkbox" class="check" value="Test SSID and equipment identified"> Test SSID and equipment identified</label>
<label><input type="checkbox" class="check" value="Baseline configuration recorded"> Baseline configuration recorded</label>
<label><input type="checkbox" class="check" value="Monitoring and recovery plan prepared"> Monitoring and recovery plan prepared</label>
<label><input type="checkbox" class="check" value="Results and remediation documented"> Results and remediation documented</label>
</div>
<div>
<h3>Assessment notes</h3>
<label>Finding
<input id="finding" placeholder="Observed configuration or issue"></label>
<label>Recommended remediation
<input id="remedy" placeholder="e.g. disable WPS, use WPA3"></label>
</div>
</div>
</section>

<section>
<h2>Event log</h2>
<button onclick="addLog('Manual event recorded')">Record event</button>
<button onclick="clearLog()">Clear log</button>
<pre id="log">Lab started. No attack traffic has been sent by this dashboard.</pre>
</section>

<section>
<h2>Export assessment</h2>
<p>Download a JSON record of scan results, checklist status, WPS notes, selected offline scenario and findings.</p>
<button onclick="exportReport()">Download JSON report</button>
<button onclick="showReport()">Preview report</button>
<button onclick="exportCSV()">Download CSV</button>
<pre id="report" hidden></pre>
</section>
<footer><small>ESP32 Wi-Fi Security Lab · Educational prototype · Verify findings on equipment you control.</small></footer>
</main>
<script>
let scanResults=[];
let events=['Lab started. Dashboard does not transmit attack traffic.'];
let lastScenario=null;
function addLog(message){
 const stamp=new Date().toISOString();
 events.push(stamp+' — '+message);
 document.getElementById('log').textContent=events.join('\n');
}
async function getStatus(){
 const el=document.getElementById('status');
 try{
  const r=await fetch('/api/status',{cache:'no-store'});
  if(!r.ok)throw Error('HTTP '+r.status);
  el.textContent=await r.text();
  document.getElementById('mode').textContent='ESP32 API connected';
  addLog('Device status refreshed');
 }catch(e){
  el.textContent='Device API unavailable. Connect to the ESP32 lab access point and reload.';
  document.getElementById('mode').textContent='Preview / disconnected';
 }
}
async function scan(){
 const el=document.getElementById('networks');
 el.textContent='Scanning nearby networks…';
 try{
  const r=await fetch('/api/scan',{cache:'no-store'});
  if(!r.ok)throw Error('HTTP '+r.status);
  const data=await r.json();
  scanResults=Array.isArray(data)?data:[];
  el.textContent=scanResults.length?scanResults.map((n,i)=>
   (i+1)+'. SSID: '+(n.ssid||'(hidden SSID)')+
   '\\n   BSSID: '+(n.bssid||'Unavailable')+
   '\\n   Signal: '+n.rssi+' dBm | Channel: '+(n.channel??'Unknown')+
   '\\n   Security: '+(n.security||'Unknown')
  ).join('\\n\\n'):'No networks found.';
  const select=document.getElementById('target');
  select.innerHTML='';
  if(!scanResults.length){
   select.add(new Option('No networks found',''));
   document.getElementById('details').textContent='No network selected.';
  }else{
   select.add(new Option('Select a network',''));
   scanResults.forEach((n,i)=>select.add(new Option((n.ssid||'(hidden SSID)')+' — '+n.rssi+' dBm',String(i))));
  }
  addLog('Scan completed: '+scanResults.length+' network(s) reported');
 }catch(e){
  el.textContent='Scan failed. Confirm the ESP32 is reachable and /api/scan is available.';
  addLog('Scan failed: '+e.message);
 }
}
document.getElementById('target').addEventListener('change',e=>{
 const n=scanResults[Number(e.target.value)];
 document.getElementById('details').textContent=n?
  'SSID: '+(n.ssid||'(hidden)')+
  '\\nBSSID: '+(n.bssid||'Unavailable')+
  '\\nSignal: '+n.rssi+' dBm'+
  '\\nChannel: '+(n.channel??'Unknown')+
  '\\nReported security: '+n.security:
  'Select a network to inspect its details.';
});
function runScenario(){
 const key=document.getElementById('scenario').value;
 const scenarios={
 password:'WEAK PASSWORD RISK — Offline scenario\nA weak or reused Wi-Fi passphrase can make an access point easier to compromise if an attacker obtains suitable authentication material.\nRecommended controls: use a long, unique passphrase; prefer WPA3-Personal when supported or WPA2-AES; update router firmware.',
 wps:'WPS EXPOSURE RISK — Offline scenario\nIf WPS is enabled, the router may expose an additional authentication path. This dashboard cannot detect WPS or test PINs.\nRecommended controls: disable WPS if not required; review router documentation and vendor updates.',
 deauth:'DEAUTHENTICATION RISK — Offline scenario\nManagement-frame disruption can disconnect clients and interrupt service. This button sends no frames.\nRecommended controls: enable Protected Management Frames (802.11w/PMF) where supported; monitor unexpected disconnects.',
 'evil-twin':'ROGUE ACCESS POINT RISK — Offline scenario\nA look-alike SSID may mislead users into connecting to an untrusted access point. This dashboard does not create an evil twin or collect credentials.\nRecommended controls: verify network identity, avoid certificate warnings, and use enterprise authentication where appropriate.',
 monitoring:'MONITORING AND DETECTION — Offline scenario\nRecord baseline SSIDs, signal levels and router configuration, then compare observations for unexpected access points or repeated disconnects.\nRecommended controls: maintain an asset list, review router logs and investigate unexplained configuration changes.'
 };
 lastScenario={key:key,result:scenarios[key],time:new Date().toISOString()};
 document.getElementById('scenarioResult').textContent=scenarios[key];
 addLog('Offline scenario viewed: '+key);
}
function getReport(){
 return {
  generatedAt:new Date().toISOString(),
  deviceStatus:document.getElementById('status').textContent,
  scanResults:scanResults,
  selectedNetwork:document.getElementById('target').value===''?null:scanResults[Number(document.getElementById('target').value)],
  wpsAssessment:{status:document.getElementById('wps').value,notes:document.getElementById('wpsNotes').value},
  offlineScenario:lastScenario,
  checklist:Array.from(document.querySelectorAll('.check')).map(x=>({item:x.value,complete:x.checked})),
  finding:document.getElementById('finding').value,
  remediation:document.getElementById('remedy').value,
  events:events
 };
}
function showReport(){
 const el=document.getElementById('report');
 el.hidden=false;
 el.textContent=JSON.stringify(getReport(),null,2);
}
function exportReport(){
 const data=JSON.stringify(getReport(),null,2);
 const blob=new Blob([data],{type:'application/json'});
 const url=URL.createObjectURL(blob);
 const a=document.createElement('a');
 a.href=url;a.download='esp32-wifi-assessment-'+new Date().toISOString().replace(/[:.]/g,'-')+'.json';
 a.click();URL.revokeObjectURL(url);
 addLog('Assessment JSON exported');
}

function exportCSV(){
 const report=getReport();
 const rows=[['type','field','value']];
 report.scanResults.forEach((n,i)=>{
  rows.push(['scan','index',i+1]);
  rows.push(['scan','ssid',n.ssid||'']);
  rows.push(['scan','bssid',n.bssid||'']);
  rows.push(['scan','rssi_dbm',n.rssi??'']);
  rows.push(['scan','channel',n.channel??'']);
  rows.push(['scan','security',n.security||'']);
 });
 rows.push(['wps','status',report.wpsAssessment.status]);
 rows.push(['wps','notes',report.wpsAssessment.notes]);
 rows.push(['finding','description',report.finding]);
 rows.push(['finding','remediation',report.remediation]);
 report.checklist.forEach(c=>rows.push(['checklist',c.item,c.complete]));
 const csv=rows.map(row=>row.map(v=>{
  const value=String(v??'');
  return '"'+value.replace(/"/g,'""')+'"';
 }).join(',')).join('\r\n');
 const blob=new Blob([csv],{type:'text/csv;charset=utf-8'});
 const url=URL.createObjectURL(blob);
 const a=document.createElement('a');
 a.href=url;a.download='esp32-wifi-assessment.csv';
 a.click();URL.revokeObjectURL(url);
 addLog('Assessment CSV exported');
}

function clearLog(){events=[];document.getElementById('log').textContent='Log cleared.';}
getStatus();
</script>
</body></html>

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

    String ssid = escapeJson(WiFi.SSID(i));
    String bssid = escapeJson(WiFi.BSSIDstr(i));
    int rssi = WiFi.RSSI(i);
    int channel = WiFi.channel(i);
    String security = "Protected";

    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN:
        security = "Open";
        break;
      case WIFI_AUTH_WEP:
        security = "WEP";
        break;
      case WIFI_AUTH_WPA_PSK:
        security = "WPA-Personal";
        break;
      case WIFI_AUTH_WPA2_PSK:
        security = "WPA2-Personal";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        security = "WPA/WPA2-Personal";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
        security = "WPA2-Enterprise";
        break;
      case WIFI_AUTH_WPA3_PSK:
        security = "WPA3-Personal";
        break;
      case WIFI_AUTH_WPA2_WPA3_PSK:
        security = "WPA2/WPA3-Personal";
        break;
      default:
        security = "Protected (type not classified)";
        break;
    }

    json += "{\"ssid\":\"" + ssid + "\",";
    json += "\"bssid\":\"" + bssid + "\",";
    json += "\"rssi\":" + String(rssi) + ",";
    json += "\"channel\":" + String(channel) + ",";
    json += "\"security\":\"" + security + "\"}";
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
