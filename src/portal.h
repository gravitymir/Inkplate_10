#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// --- Access point shown on power-on (open for 5 minutes, then off) ---
#define AP_SSID "Inkplate-Clock"
#define AP_PASS "clock1234" // WPA2 needs >= 8 chars
#define PORTAL_DURATION_MS (2UL * 60UL * 1000UL)

// Working hours (defaults), persisted in NVS and editable from the web page.
int g_workStart = 8;  // 08:00
int g_workEnd = 23;   // 23:00 is the last shown time

WebServer server(80);
Preferences prefs;

void loadConfig()
{
    prefs.begin("clock", true); // read-only
    g_workStart = prefs.getInt("wstart", 8);
    g_workEnd = prefs.getInt("wend", 23);
    prefs.end();
}

void saveConfig()
{
    prefs.begin("clock", false);
    prefs.putInt("wstart", g_workStart);
    prefs.putInt("wend", g_workEnd);
    prefs.end();
}

String pageHtml(const String &msg)
{
    String h = F(R"=====(<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Inkplate Clock</title>
<style>
body{font-family:sans-serif;max-width:440px;margin:24px auto;padding:0 16px;color:#222}
h2{margin-bottom:4px}label{display:block;margin:14px 0 4px;font-weight:600}
input{width:100%;padding:10px;font-size:16px;box-sizing:border-box}
button{margin-top:18px;padding:12px;font-size:16px;width:100%;border:0;border-radius:6px;background:#222;color:#fff}
.row{display:flex;gap:14px}.row>div{flex:1}
.ok{color:#0a7d28;font-weight:600}.note{color:#666;font-size:14px;margin-top:24px}
</style></head><body>
<h2>Inkplate Clock</h2>
<p class="ok">%MSG%</p>
<form method="POST" action="/save">
 <div class="row">
  <div><label>Work start (hour)</label><input type="number" name="start" min="0" max="23" value="%START%"></div>
  <div><label>Work end (hour)</label><input type="number" name="end" min="0" max="23" value="%END%"></div>
 </div>
 <button type="submit">Save work time</button>
</form>
<button onclick="syncTime()" style="background:#0a7d28">Synchronize time with this browser</button>
<p id="msg"></p>
<p class="note">This setup access point turns off automatically about 2 minutes after power-on to save battery.</p>
<script>
function syncTime(){
 var d=new Date();
 var q='/settime?yr='+d.getFullYear()+'&mo='+d.getMonth()+'&da='+d.getDate()
   +'&wd='+d.getDay()+'&h='+d.getHours()+'&m='+d.getMinutes()+'&s='+d.getSeconds();
 fetch(q).then(function(r){return r.text();}).then(function(t){
   document.getElementById('msg').innerHTML='<span class="ok">'+t+'</span>';
 }).catch(function(e){document.getElementById('msg').innerText='Error: '+e;});
}
</script></body></html>)=====");
    h.replace("%MSG%", msg);
    h.replace("%START%", String(g_workStart));
    h.replace("%END%", String(g_workEnd));
    return h;
}

void handleRoot()
{
    server.send(200, "text/html", pageHtml(""));
}

void handleSave()
{
    if (server.hasArg("start"))
        g_workStart = constrain(server.arg("start").toInt(), 0, 23);
    if (server.hasArg("end"))
        g_workEnd = constrain(server.arg("end").toInt(), 0, 23);
    saveConfig();
    Serial.printf("Saved work time: %02d:00 - %02d:00\n", g_workStart, g_workEnd);
    server.send(200, "text/html", pageHtml("Work time saved."));
}

void handleSetTime()
{
    int yr = server.arg("yr").toInt();
    int mo = server.arg("mo").toInt(); // 0-based, matches RTC convention used here
    int da = server.arg("da").toInt();
    int wd = server.arg("wd").toInt(); // 0 = Sunday
    int hh = server.arg("h").toInt();
    int mm = server.arg("m").toInt();
    int ss = server.arg("s").toInt();

    display.rtcSetTime(hh, mm, ss);
    display.rtcSetDate(wd, da, mo, yr);

    char buf[48];
    sprintf(buf, "Time set: %04d-%02d-%02d %02d:%02d:%02d", yr, mo + 1, da, hh, mm, ss);
    Serial.println(buf);
    server.send(200, "text/plain", buf);
}

// Center a default-font string horizontally at vertical position y.
void drawCenteredText(const String &s, int y, int size)
{
    int16_t bx, by;
    uint16_t bw, bh;
    display.setTextSize(size);
    display.getTextBounds(s, 0, y, &bx, &by, &bw, &bh);
    int x = (display.width() - (int)bw) / 2;
    if (x < 0)
        x = 0;
    display.setCursor(x, y);
    display.print(s);
}

// WiFi setup screen: the logo with the setup text laid over it.
void drawPortalScreen(IPAddress ip)
{
    display.clearDisplay();

    // Logo centered (same as the sleep screen).
    int lx = (display.width() - LOGO_W) / 2;
    int ly = (display.height() - LOGO_H) / 2; // 825-600 -> 112
    display.drawBitmap3Bit(lx, ly, logo3bit, LOGO_W, LOGO_H);

    display.setFont(); // default built-in font
    display.setTextColor(0);

    // Title in the white strip above the logo.
    drawCenteredText("WiFi SETUP MODE  -  active ~2 min", 35, 5);

    // Steps in the white strip below the logo.
    drawCenteredText(String("1) WiFi: ") + AP_SSID + "    Password: " + AP_PASS, 725, 3);
    drawCenteredText(String("2) Open  http://") + ip.toString(), 760, 3);
    drawCenteredText("3) Set work hours, then tap Synchronize", 795, 3);

    display.display();
}

// Bring up the AP + web server for PORTAL_DURATION_MS, then shut WiFi off.
void runConfigPortal()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    IPAddress ip = WiFi.softAPIP();
    Serial.printf("AP \"%s\" up at %s\n", AP_SSID, ip.toString().c_str());

    drawPortalScreen(ip);

    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/settime", handleSetTime);
    server.begin();

    unsigned long start = millis();
    while (millis() - start < PORTAL_DURATION_MS)
    {
        server.handleClient();
        delay(5);
    }

    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("AP closed for power saving.");
}
