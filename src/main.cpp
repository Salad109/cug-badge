#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const int LED = 8;
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_BITS = 8;

const size_t MAX_PRESETS = 8;
const size_t MAX_NAME_LEN = 13;
const size_t MAX_PATTERN_LEN = 3999;
const uint16_t DEFAULT_FRAME_MS = 100;

struct Preset {
  String name;
  String pattern;
};

Preset presets[MAX_PRESETS];
size_t presetCount = 0;

String currentName;
String currentPattern;
uint16_t frameMs = DEFAULT_FRAME_MS;
uint32_t frameIdx = 0;
uint32_t lastFrameAt = 0;

WebServer server(80);
WebSocketsServer ws(81);
Preferences prefs;

bool isHex(const String& s) {
  if (s.length() == 0 || s.length() > MAX_PATTERN_LEN) return false;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
  }
  return true;
}

bool isValidName(const String& s) {
  if (s.length() == 0 || s.length() > MAX_NAME_LEN) return false;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (!(isalnum(c) || c == '_' || c == '-')) return false;
  }
  return true;
}

int findPreset(const String& name) {
  for (size_t i = 0; i < presetCount; i++) {
    if (presets[i].name == name) return (int)i;
  }
  return -1;
}

uint8_t hexCharToValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  return 0;
}

void writeLed(uint8_t level0to15) {
  uint8_t duty = level0to15 * 17;       // 0..255
  ledcWrite(PWM_CHANNEL, 255 - duty);   // active-low
}

void ledOff() {
  ledcWrite(PWM_CHANNEL, 255);
}

void persistNames() {
  String csv;
  for (size_t i = 0; i < presetCount; i++) {
    if (i) csv += ",";
    csv += presets[i].name;
  }
  prefs.putString("names", csv);
}

void persistPreset(const Preset& p) {
  prefs.putString((String("p_") + p.name).c_str(), p.pattern);
}

void erasePresetKey(const String& name) {
  prefs.remove((String("p_") + name).c_str());
}

void loadAll() {
  presetCount = 0;
  String csv = prefs.getString("names", "");
  while (csv.length() > 0 && presetCount < MAX_PRESETS) {
    int comma = csv.indexOf(',');
    String n = (comma < 0) ? csv : csv.substring(0, comma);
    csv = (comma < 0) ? "" : csv.substring(comma + 1);
    if (!isValidName(n)) continue;
    String pat = prefs.getString((String("p_") + n).c_str(), "");
    if (!isHex(pat)) continue;
    presets[presetCount].name = n;
    presets[presetCount].pattern = pat;
    presetCount++;
  }
  frameMs = prefs.getUShort("frame_ms", DEFAULT_FRAME_MS);
  if (frameMs == 0) frameMs = DEFAULT_FRAME_MS;
  String cur = prefs.getString("current", "");
  int idx = findPreset(cur);
  if (idx >= 0) {
    currentName = presets[idx].name;
    currentPattern = presets[idx].pattern;
  }
}

String serializeState() {
  String s = "STATE\n";
  s += "current=" + currentName + "\n";
  s += "pattern=" + currentPattern + "\n";
  s += "speed=" + String(frameMs) + "\n";
  s += "presets=";
  for (size_t i = 0; i < presetCount; i++) {
    if (i) s += ",";
    s += presets[i].name + ":" + presets[i].pattern;
  }
  return s;
}

void broadcastState() {
  String s = serializeState();
  ws.broadcastTXT(s);
}

void setCurrent(const String& name) {
  int idx = findPreset(name);
  if (idx < 0) return;
  currentName = presets[idx].name;
  currentPattern = presets[idx].pattern;
  frameIdx = 0;
  lastFrameAt = millis();
  prefs.putString("current", currentName);
}

void stopPlaying() {
  currentName = "";
  currentPattern = "";
  frameIdx = 0;
  ledOff();
  prefs.putString("current", "");
}

bool savePreset(const String& name, const String& pat) {
  if (!isValidName(name) || !isHex(pat)) return false;
  int idx = findPreset(name);
  if (idx >= 0) {
    presets[idx].pattern = pat;
  } else {
    if (presetCount >= MAX_PRESETS) return false;
    presets[presetCount].name = name;
    presets[presetCount].pattern = pat;
    presetCount++;
    persistNames();
  }
  persistPreset(presets[findPreset(name)]);
  setCurrent(name);
  return true;
}

bool deletePreset(const String& name) {
  int idx = findPreset(name);
  if (idx < 0) return false;
  erasePresetKey(name);
  for (size_t i = idx; i + 1 < presetCount; i++) presets[i] = presets[i + 1];
  presetCount--;
  presets[presetCount] = Preset();
  persistNames();
  if (currentName == name) stopPlaying();
  return true;
}

void setSpeed(uint16_t ms) {
  if (ms == 0) ms = 1;
  frameMs = ms;
  prefs.putUShort("frame_ms", frameMs);
}

void handleRoot() {
  server.send(200, "text/html",
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>LED Pattern Player</title>"
    "<style>"
    "body{font-family:sans-serif;margin:0 auto;padding:1rem;background:#1a1a1a;color:#eee;max-width:600px}"
    "h1{font-size:1.4rem}h2{font-size:1.1rem;margin-top:1.5rem;border-bottom:1px solid #333;padding-bottom:.3rem}"
    "section{margin-bottom:1rem}"
    ".now{padding:.8rem;background:#222;border-radius:8px}"
    ".now .name{font-weight:bold;font-size:1.2rem}"
    ".now .pat{font-family:monospace;color:#8cf;word-break:break-all}"
    ".muted{color:#888}"
    "ul{list-style:none;padding:0}"
    "li{display:flex;align-items:center;gap:.5rem;padding:.4rem 0;border-bottom:1px solid #2a2a2a}"
    "li .pname{flex:1;font-weight:bold}"
    "li .ppat{font-family:monospace;color:#8cf;font-size:.85rem;flex:2;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}"
    "input,button{font-size:1rem;padding:.5rem;border-radius:6px;border:1px solid #444;background:#222;color:#eee}"
    "input{width:100%;box-sizing:border-box;margin:.2rem 0}"
    "button{cursor:pointer;background:#444}"
    "button.primary{background:#4caf50;border-color:#4caf50}"
    "button.danger{background:#a33;border-color:#a33}"
    ".row{display:flex;gap:.5rem;align-items:center}"
    ".row label{flex:0 0 auto}"
    "</style></head><body>"

    "<h1>LED Pattern Player</h1>"

    "<h2>Now playing</h2>"
    "<section class='now'>"
    "<div class='name' id='curName'>&mdash;</div>"
    "<div class='pat' id='curPat'></div>"
    "<button class='danger' onclick='send(\"stop\")' style='margin-top:.5rem'>Stop</button>"
    "</section>"

    "<h2>Speed</h2>"
    "<section class='row'>"
    "<label for='speed'>ms/frame:</label>"
    "<input type='number' id='speed' min='1' style='width:100px' onchange='send(\"speed \"+this.value)'>"
    "</section>"

    "<h2>Saved presets</h2>"
    "<ul id='list'></ul>"

    "<h2>New / overwrite preset</h2>"
    "<section>"
    "<input id='newName' placeholder='name (a-z 0-9 _ -, max 13)' maxlength='13'>"
    "<input id='newPat' placeholder='pattern: hex chars 0-F, e.g. 01234567890ABCDEFEDCBA9876543210' maxlength='3999'>"
    "<button class='primary' onclick='savePreset()'>Save & play</button>"
    "<div class='muted' style='margin-top:.5rem;font-size:.85rem'>"
    "0 = LED off, F = full bright. Pattern loops, one char per frame."
    "</div>"
    "</section>"

    "<script>"
    "let ws;"
    "function connect(){"
    "  ws=new WebSocket('ws://'+location.hostname+':81');"
    "  ws.onmessage=e=>{if(e.data.startsWith('ERROR'))alert(e.data.split('\\n')[1]||'error');else render(e.data);};"
    "  ws.onclose=()=>setTimeout(connect,1000);"
    "}"
    "function send(m){if(ws&&ws.readyState===1)ws.send(m);}"
    "function savePreset(){"
    "  const n=document.getElementById('newName').value.trim();"
    "  const p=document.getElementById('newPat').value.trim();"
    "  if(!n||!p){alert('name and pattern required');return;}"
    "  send('save '+n+'|'+p);"
    "  document.getElementById('newName').value='';"
    "  document.getElementById('newPat').value='';"
    "}"
    "function parseState(txt){"
    "  const o={presets:[]};"
    "  txt.split('\\n').forEach(line=>{"
    "    const i=line.indexOf('=');if(i<0)return;"
    "    const k=line.slice(0,i),v=line.slice(i+1);"
    "    if(k==='presets'){"
    "      o.presets=v?v.split(',').map(s=>{const j=s.indexOf(':');return{name:s.slice(0,j),pattern:s.slice(j+1)};}):[];"
    "    } else o[k]=v;"
    "  });"
    "  return o;"
    "}"
    "function render(txt){"
    "  if(!txt.startsWith('STATE'))return;"
    "  const s=parseState(txt);"
    "  document.getElementById('curName').textContent=s.current||'(stopped)';"
    "  document.getElementById('curPat').textContent=s.pattern||'';"
    "  const sp=document.getElementById('speed');"
    "  if(document.activeElement!==sp)sp.value=s.speed;"
    "  const ul=document.getElementById('list');"
    "  ul.innerHTML='';"
    "  if(s.presets.length===0){ul.innerHTML='<li class=\"muted\">no presets yet</li>';return;}"
    "  s.presets.forEach(p=>{"
    "    const li=document.createElement('li');"
    "    li.innerHTML='<span class=\"pname\"></span><span class=\"ppat\"></span>"
    "<button class=\"primary\">Play</button><button class=\"danger\">Del</button>';"
    "    li.querySelector('.pname').textContent=p.name;"
    "    li.querySelector('.ppat').textContent=p.pattern;"
    "    li.querySelectorAll('button')[0].onclick=()=>send('play '+p.name);"
    "    li.querySelectorAll('button')[1].onclick=()=>{if(confirm('delete '+p.name+'?'))send('delete '+p.name);};"
    "    ul.appendChild(li);"
    "  });"
    "}"
    "connect();"
    "</script>"
    "</body></html>"
  );
}

void sendError(uint8_t client, const String& msg) {
  Serial.println("ws error: " + msg);
  String err = "ERROR\n" + msg;
  ws.sendTXT(client, err);
}

void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    String s = serializeState();
    ws.sendTXT(client, s);
    return;
  }
  if (type != WStype_TEXT) return;

  String msg = String((char*)payload);
  Serial.println("ws: " + msg);

  if (msg == "stop") {
    stopPlaying();
  } else if (msg.startsWith("play ")) {
    String name = msg.substring(5);
    if (findPreset(name) < 0) {
      sendError(client, "preset not found: " + name);
      return;
    }
    setCurrent(name);
  } else if (msg.startsWith("delete ")) {
    String name = msg.substring(7);
    if (!deletePreset(name)) {
      sendError(client, "preset not found: " + name);
      return;
    }
  } else if (msg.startsWith("speed ")) {
    setSpeed((uint16_t)msg.substring(6).toInt());
  } else if (msg.startsWith("save ")) {
    String rest = msg.substring(5);
    int bar = rest.indexOf('|');
    if (bar <= 0) {
      sendError(client, "malformed save command");
      return;
    }
    String name = rest.substring(0, bar);
    String pat = rest.substring(bar + 1);
    if (!isValidName(name)) {
      sendError(client, "invalid name '" + name + "': use only letters, digits, _ or -, max 12 chars");
      return;
    }
    if (!isHex(pat)) {
      sendError(client, "invalid pattern '" + pat + "': use only hex chars 0-9 A-F, max 64 chars");
      return;
    }
    if (!savePreset(name, pat)) {
      sendError(client, "save failed: preset limit reached (" + String(MAX_PRESETS) + " max)");
      return;
    }
  } else {
    return;
  }
  broadcastState();
}

void setup() {
  Serial.begin(115200);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_BITS);
  ledcAttachPin(LED, PWM_CHANNEL);
  ledOff();

  prefs.begin("led", false);
  loadAll();

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.begin();

  ws.begin();
  ws.onEvent(onWebSocketEvent);

  lastFrameAt = millis();
}

void loop() {
  server.handleClient();
  ws.loop();

  if (currentPattern.length() > 0) {
    uint32_t now = millis();
    if (now - lastFrameAt >= frameMs) {
      lastFrameAt = now;
      char c = currentPattern[frameIdx % currentPattern.length()];
      writeLed(hexCharToValue(c));
      frameIdx++;
    }
  }
}
