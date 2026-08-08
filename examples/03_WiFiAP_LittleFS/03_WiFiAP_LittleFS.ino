/**
 * 03_WiFiAP_LittleFS — SoftAP + on-flash LittleFS
 * Connect to the AP, then open http://192.168.4.1/
 */
#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 16384
#endif

#include <WebFileManager.h>

WfmStorageLittleFS storage(true);  // format on fail (dev boards)
WfmNetwork net;
WebFileManager wfm(storage);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("WebFileManager 03_WiFiAP_LittleFS");

  if (!storage.begin()) {
    Serial.println("LittleFS begin failed");
    while (true) delay(1000);
  }
  if (!net.beginAP("ESP-Files", "password123")) {
    Serial.println("SoftAP failed");
    while (true) delay(1000);
  }

  wfm.setName("LittleFS").begin();
  wfm.startFileTask();
  Serial.printf("Join WiFi ESP-Files / password123\n");
  Serial.printf("Open http://%s/\n", net.localIP().toString().c_str());
}

void loop() { wfm.loop(); }
