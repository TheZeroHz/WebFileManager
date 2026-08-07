# WebFileManager

Explorer-style **web file manager** for Arduino-ESP32 3.x.

Browse, preview, upload, download, rename, copy, move, and delete files from a browser over **WiFi (STA or AP)** or **Ethernet**. Storage is pluggable: **SD / SD_MMC / FFat / LittleFS / SPIFFS** (or any `fs::FS`).

## Features

| Area | Details |
|------|---------|
| Storage | `WfmStorageSD`, `WfmStorageSD_MMC`, `WfmStorageFFat`, `WfmStorageLittleFS`, `WfmStorageSPIFFS`, `WfmStorageFS` |
| Network | `WfmNetwork::beginSTA`, `beginAP`, `beginEthernet` |
| HTTP | UI/API on port **80**, transfers on **81** (configurable) |
| Transfers | Upload, download with HTTP Range, **browser ZIP** (multi-select), image/video preview |
| Concurrency | Optional FreeRTOS file task + storage mutex |
| UI | Windows-like explorer (PROGMEM), live transfer speed |

## Install

Copy the `WebFileManager` folder into your Arduino libraries directory:

`Documents/Arduino/libraries/WebFileManager`

Or clone / open this repo and add the folder to your sketchbook libraries.

**Requires:** Arduino-ESP32 3.x, PSRAM recommended for large transfers.

## Quick start (WiFi AP + LittleFS)

```cpp
#include <WebFileManager.h>

WfmStorageLittleFS storage;
WfmNetwork net;
WebFileManager wfm(storage);

void setup() {
  Serial.begin(115200);
  storage.begin();                 // mount LittleFS
  net.beginAP("ESP-Files", "password");
  wfm.setName("My Board").setPorts(80, 81).begin();
  wfm.startFileTask();             // UI stays usable during uploads
  Serial.printf("Open http://%s/\n", net.localIP().toString().c_str());
}

void loop() {
  wfm.loop();                      // serve UI / JSON
}
```

## Quick start (Ethernet + SD_MMC)

```cpp
#include <WebFileManager.h>

WfmStorageSD_MMC storage;
WfmNetwork net;
WebFileManager wfm(storage);

void setup() {
  Serial.begin(115200);
  // storage.setPins(clk, cmd, d0, ...);  // if required by your board
  if (!storage.begin()) {
    Serial.println("SD mount failed");
    return;
  }
  net.setHostname("file-mgr");
  net.beginEthernet(WfmNetwork::guitionM3Eth());  // or fill WfmEthConfig (see below)
  while (!net.ready()) delay(200);

  wfm.setName("SD Archive").begin();
  wfm.startFileTask();
  Serial.printf("UI http://%s/\n", net.localIP().toString().c_str());
}

void loop() {
  wfm.loop();
}
```

## Ethernet pins (`WfmEthConfig`)

Ethernet wiring is **board-specific**. Pass a `WfmEthConfig` into `net.beginEthernet(cfg)`.

| Field | Meaning | Typical |
|-------|---------|---------|
| `phyType` | PHY chip | `ETH_PHY_IP101`, `ETH_PHY_LAN8720`, `ETH_PHY_RTL8201`, … |
| `phyAddr` | PHY MDIO address | often `0` or `1` |
| `mdc` | MDC GPIO | schematic |
| `mdio` | MDIO GPIO | schematic |
| `power` | PHY power / enable GPIO, or `-1` if hard-wired | schematic |
| `clkMode` | RMII clock mode | ESP32-P4: `EMAC_CLK_EXT_IN`; classic ESP32: `ETH_CLOCK_GPIO0_IN` / `OUT`, etc. |

### Preset: Guition JC-ESP32P4-M3 (IP101)

```cpp
net.beginEthernet(WfmNetwork::guitionM3Eth());
// MDC=31, MDIO=52, POWER=51, PHY addr=1, clk=EMAC_CLK_EXT_IN
```

### Custom pins (any board)

```cpp
WfmEthConfig eth;
eth.phyType = ETH_PHY_IP101;   // change to match your PHY
eth.phyAddr = 1;
eth.mdc     = 31;              // your MDC GPIO
eth.mdio    = 52;              // your MDIO GPIO
eth.power   = 51;              // or -1 if always powered
#if CONFIG_IDF_TARGET_ESP32P4
eth.clkMode = EMAC_CLK_EXT_IN; // external RMII clock (common on P4)
#endif

if (!net.beginEthernet(eth)) {
  Serial.println("ETH.begin failed — check pins / PHY");
}
while (!net.ready()) delay(200);
```

`EMAC_CLK_*` / `ETH_CLOCK_*` are **enum values**, not `#define`s — do not wrap them in `#ifdef EMAC_CLK_EXT_IN`.

### SD_MMC pins (if needed)

```cpp
storage.setPins(clk, cmd, d0);                 // 1-bit
// storage.setPins(clk, cmd, d0, d1, d2, d3);  // 4-bit
storage.begin();
```

Call `setPins()` **before** `begin()`. Omit it if the board uses Arduino-ESP32 defaults.

## Multiple volumes (SD + flash)

```cpp
WfmStorageFS sd(sdFs, "SD", totalFn, usedFn);
WfmStorageLittleFS flash;
WebFileManager wfm(sd);              // first volume
wfm.addVolume("LittleFS", flash);    // second (or FFat / SPIFFS)

// Browser paths become:
//   /           -> volume list
//   /SD/...     -> SD card
//   /LittleFS/...
```

Single-volume mode keeps classic paths (`/folder/file`) with no prefix.

## Architecture (OOP / DRY)

```
WfmStorage  ←── SD / SD_MMC / FFat / LittleFS / SPIFFS / FS adapter
WfmNetwork  ←── WiFi STA | WiFi AP | Ethernet
WebFileManager ── dual WebServer (UI + file), shared util (WfmUtil), UI (WfmUi)
```

- **One HTTP implementation** for all backends (no duplicated handlers).
- **Mutex on storage** so UI listing and transfers can run on different tasks.
- **Helpers** (`pathOk`, MIME, JSON escape, Range parse) live in `WfmUtil.h`.

## API cheat sheet

```cpp
storage.begin();
net.beginSTA(ssid, pass);          // or beginAP / beginEthernet
wfm.setPorts(80, 81).setName("X").begin();
wfm.startFileTask();               // recommended
wfm.loop();                        // in loop()
wfm.refreshUsageAsync();           // re-scan used/total
```

### Ports

| Port | Role |
|------|------|
| UI (80) | HTML + `/api/list`, status, rename, delete, … |
| File (81) | `/view`, `/download`, `/api/upload` (CORS enabled) |

Open **`http://<ip>/`** in a browser. Hard-refresh after firmware updates.

## Examples

| Sketch | Stack |
|--------|--------|
| `examples/01_Ethernet_SD` | Ethernet + SD_MMC (Guition M3 pins helper) |
| `examples/02_WiFiSTA_SD` | WiFi station + SPI SD |
| `examples/03_WiFiAP_LittleFS` | SoftAP + LittleFS |

## Security (username / password)

```cpp
wfm.setAuth("admin", "changeme");  // omit or empty user = open access
```

The UI shows a sign-in dialog. Credentials are sent as HTTP Basic on API and
transfer requests. See `examples/04_Auth_MultiVolume`.

