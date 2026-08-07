/**
 * 04_Auth_MultiVolume — username/password + SD (optional) + LittleFS example
 *
 * Default login: admin / changeme
 * Open http://<ip>/  — sign-in dialog appears when auth is enabled.
 */
#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 16384
#endif

#include <WebFileManager.h>

// --- edit these ---
#ifndef WIFI_SSID
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
#endif
#ifndef WFM_USER
#define WFM_USER "admin"
#define WFM_PASS "changeme"
#endif

WfmStorageLittleFS flash(true);
WfmNetwork net;
WebFileManager wfm(flash);

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println("WebFileManager 04_Auth_MultiVolume");

  if (!flash.begin()) {
    Serial.println("LittleFS failed");
    while (true) delay(1000);
  }

  // Optional: add SPI SD as a second volume
  // static WfmStorageSD sd(SS);
  // if (sd.begin()) wfm.addVolume("SD", sd);

  if (!net.beginSTA(WIFI_SSID, WIFI_PASS, 25000)) {
    Serial.println("WiFi failed — starting AP fallback");
    net.beginAP("ESP-Files", "password123");
  }

  wfm.setName("Secure Files")
      .setAuth(WFM_USER, WFM_PASS)  // comment out to disable auth
      .setPorts(80, 81)
      .begin();
  wfm.startFileTask();

  Serial.printf("Open http://%s/\n", net.localIP().toString().c_str());
  Serial.printf("Login: %s / %s\n", WFM_USER, WFM_PASS);
}

void loop() { wfm.loop(); }
