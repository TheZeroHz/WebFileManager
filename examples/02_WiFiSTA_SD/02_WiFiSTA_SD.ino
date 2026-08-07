/**
 * 02_WiFiSTA_SD — WebFileManager over WiFi station + SPI SD
 * Edit WIFI_SSID / WIFI_PASS / SD_CS for your board.
 */
#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 16384
#endif

#include <WebFileManager.h>

#ifndef WIFI_SSID
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
#endif
#ifndef SD_CS
#define SD_CS SS
#endif

WfmStorageSD storage(SD_CS);
WfmNetwork net;
WebFileManager wfm(storage);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("WebFileManager 02_WiFiSTA_SD");

  if (!storage.begin()) {
    Serial.println("SD begin failed");
    while (true) delay(1000);
  }
  if (!net.beginSTA(WIFI_SSID, WIFI_PASS, 25000)) {
    Serial.println("WiFi STA failed");
    while (true) delay(1000);
  }

  wfm.setName("WiFi SD").begin();
  wfm.startFileTask();
  Serial.printf("Open http://%s/\n", net.localIP().toString().c_str());
}

void loop() { wfm.loop(); }
