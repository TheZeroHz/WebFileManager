/**
 * 01_Ethernet_SD — WebFileManager over Ethernet + SD_MMC
 *
 * Board example: Guition JC-ESP32P4-M3 (IP101). Adjust WfmEthConfig / SD pins
 * for your schematic. Enable PSRAM in board options.
 */
#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 16384
#endif

#include <WebFileManager.h>

WfmStorageSD_MMC storage("/sdcard", false);
WfmNetwork net;
WebFileManager wfm(storage);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("WebFileManager 01_Ethernet_SD");

  // Optional: storage.setPins(clk, cmd, d0, d1, d2, d3);
  if (!storage.begin()) {
    Serial.println("SD_MMC begin failed");
    while (true) delay(1000);
  }

  net.setHostname("web-file-mgr");
  if (!net.beginEthernet(WfmNetwork::guitionM3Eth())) {
    Serial.println("ETH.begin failed");
    while (true) delay(1000);
  }
  Serial.println("Waiting for DHCP...");
  while (!net.ready()) delay(200);

  wfm.setName("SD Archive").setPorts(80, 81).begin();
  wfm.startFileTask();
  Serial.printf("Open http://%s/\n", net.localIP().toString().c_str());
  Serial.printf("Transfers on port %u\n", (unsigned)wfm.filePort());
}

void loop() { wfm.loop(); }
