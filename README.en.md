# ESP32 VoiceAssistant · Original Edition

[中文](README.md) | **English**

An ESP32-S3 Chinese voice assistant with push-to-talk recording, speech recognition, streaming LLM conversations, web search, scientific symbols, and AHT20/BMP280 temperature, humidity, and pressure readings. It uses an ST7789 240×240 display and a custom UI without LVGL.

Current public version: **v1.1.45-i2c-aht20 · 2026-11-15**. This repository retains the original AHT20/BMP280 hardware support and five Flash-backed chat sessions. For the camera, album, image questions, and video recording, see the separate [ESP32-CAM-VoiceAssistant](https://github.com/h9647123/ESP32-CAM-VoiceAssistant). Pin mappings, firmware, and configuration are maintained separately for the two editions.

## Features

| Module | Capabilities |
|---|---|
| Speech recognition | Original iFlytek RTASR, independent iFlytek IAT 2.0, self-hosted iFlytek-compatible CUSTOM IAT, and the Volcengine interface; separate configurations |
| Audio capture | INMP441, 16 kHz mono PCM16; iFlytek IAT 2.0 uses a dedicated PSRAM capture task with concurrent 40 ms packet uploads; CUSTOM IAT uploads 200 ms packets during capture, supports revised partial results and final ACKs, and records clipping, I2S/software overflow, and transmission statistics |
| Conversations | OpenAI-compatible chat APIs; multiple providers, models, and prompts; five local SPIFFS sessions; smooth UTF-8 text reveal, last-line following, thinking timers, and answer text in the serial console |
| History protection | Separate in-memory browsing windows and original disk records; preserved committed boundaries, temporary-file validation and previous-generation backups; incomplete/corrupt loads block overwrites; mount failures never trigger automatic formatting |
| Thinking and search | Independent controls for thinking mode and thinking visibility; search, Agent search, and direct answers; bounded long-thinking retention to limit memory and layout cost |
| Text | Common math/chemistry symbols, smaller readable scripts, fractions mixed with Chinese, actual bold/headings, and tables converted into readable entries; limited LaTeX support, not a complete TeX engine |
| UI | Settings, UI, Chat, AI, and System menu groups; cards, scrolling and page transitions; brightness control and sleep delays up to 30 minutes |
| Clock and sensors | NTP time, full-screen clock, AHT20 temperature/humidity, BMP280 pressure, off-screen drawing, and change-based refresh |
| Service status | Automatic checks after connecting; ASR connection checks every 60 seconds while idle; manual offline checks return immediately |
| Configuration | USB serial and LAN web interface; settings stored in NVS; configuration imports/exports contain sensitive information and are for trusted devices only |

CUSTOM IAT supports recording up to 60 seconds with a reserved audio tail buffer. Other paths retain their own limits. Usable duration also depends on PSRAM, server policy, and the network. Users must deploy the self-hosted recognition service separately; this firmware does not include WeType, a Windows VM, or server components.

## Hardware and wiring

Validated hardware: ESP32-S3, 16 MB Flash, and 8 MB OPI PSRAM. The public template enables the AHT20/BMP280 I2C variant by default. A DHT compatibility path remains in the source.

| Device | ESP32-S3 GPIO / notes |
|---|---|
| ST7789 / GMT130 240×240 | MOSI 11, SCLK 12, RST 8, DC 7, backlight 9; CS is -1, subject to board wiring |
| INMP441 | BCLK 4, WS 5, DIN 6; connect L/R to GND for the left channel |
| AHT20 + BMP280 | SDA 18, SCL 17; AHT20 address 0x38; BMP280 probes 0x76/0x77 |
| A / B / C buttons | GPIO 13 / 14 / 15, active-low with internal pull-ups |
| Serial console | UART/CH340 at 115200 baud on the validated board; determine the current port on your computer |

Check your wiring, voltages, and module specifications before compiling or flashing. The CAM firmware's camera and TF pin assignments do not apply to this board.

## Build

Validated tool versions: Arduino ESP32 **3.3.8**, ArduinoJson **7.4.3**, LovyanGFX **1.2.21**, WebSockets **2.7.2**, zlib_turbo **1.0.0**, and DHT sensor library for ESPx **1.19**.

```powershell
# After installing Arduino CLI, run from the repository root.
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.8 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "ArduinoJson@7.4.3" "LovyanGFX@1.2.21" "WebSockets@2.7.2" "zlib_turbo@1.0.0" "DHT sensor library for ESPx@1.19"
Copy-Item VoiceAssistant/config.h.example VoiceAssistant/config.h
powershell -NoProfile -ExecutionPolicy Bypass -File tools/build-and-flash.ps1
```

Standard FQBN:

```text
esp32:esp32:esp32s3:PartitionScheme=huge_app,PSRAM=opi,FlashSize=16M,USBMode=hwcdc,CDCOnBoot=default
```

`PSRAM=opi` must match the module and must not be omitted: missing PSRAM can cause allocation failures with real chat history and TLS. Do not reuse `CDCOnBoot=cdc` from old notes; it previously left the validated board's UART console without output.

The public repository does not include a real `config.h`. The blank-credential template will not connect to the developer's Wi-Fi or services. Back up existing Flash and verify the chip, partition table, and port before flashing. `build-and-flash.ps1 -Upload -PortName COMx` uses the normal Arduino upload process and may write boot-related regions; it is not an automatic backup tool or a protected application-partition updater.

## Initial configuration and controls

Open the serial console at 115200 baud and enter `help` for the full command list. Configure network, model, and ASR credentials through local serial or a trusted LAN; they are saved in NVS. Replace the placeholders below with your own values and never commit real credentials to Git.

```text
connect <SSID> <password>
provideradd <name> <OpenAI-compatible-base-url> <api_key>
xfyun2asr <appid> <apikey> <apisecret>
asrprovider xfyun2
servicecheck
ui status
```

See [Setup](docs/SETUP.md) for other ASR settings and self-hosted WebSocket endpoints. The `webui` command shows or changes web-interface status. Use it only on trusted networks; do not expose it directly to the public internet.

| Screen | Controls |
|---|---|
| Main screen | Tap A to open the menu; hold A to record and release to recognize; B/C browse text |
| Conversation browsing | Hold B/C for continuous scrolling; while holding B/C, tap A to jump between turns |
| Draft | Follow the on-screen prompts to send or append more speech |
| Menu | B/C select, A enters; tap A to toggle binary settings; for multi-value settings, A starts editing, B/C decrease/increase, and A finishes |
| Draft deletion | Tap C to delete one UTF-8 character; double-tap C to request clearing the entire draft; A confirms, B cancels, C cancels and continues deleting; the confirmation survives redraws |
| Back | Hold A in a submenu to return one level; hold A in the root menu to return to the main screen; Wi-Fi items retain the hold-to-scan shortcut |
| Sleep | A button first wakes the display; operate it afterward; the wake notice lasts about 0.8 seconds |

## Development and validation

The project evolved from voice input and a three-button UI to multiple models/search, web configuration, scientific symbols, buffered capture, and this backport of common CAM improvements. See [Development](docs/DEVELOPMENT.md) and [Changelog](CHANGELOG.md).

The public v1.1.45 image is built independently from the blank-credential template. Current host regressions run without a CAM checkout. The development device passed application-write verification, byte-for-byte SPIFFS comparison against the pre-upgrade backup, AHT20 detection, A-toggle binary menus and B/C multi-value editing, draft-clear confirmation, and serial screenshot inspection. The user confirmed immediate initial volume readings and live text updates during recording, while reporting remaining recognition-response latency. The public blank-configuration image was not flashed again. Its defaults differ from local NVS, so the local image's hardware validation is not validation of the public binary's exact SHA. See [Validation](docs/VALIDATION.md) and [Release notes](docs/RELEASE-v1.1.45-i2c-aht20.md).

Host tests need only Python, a C++17 compiler, and ArduinoJson headers; they do not depend on the CAM repository, private backups, or online services:

```powershell
python tools/test-host.py --compiler "C:/path/to/zig.exe" --arduinojson "C:/path/to/ArduinoJson/src"
```

`--compiler clang++` and `g++` are also supported. See [Tools](tools/README.md).

## License

Original code is open source under the [MIT License](LICENSE), Copyright (c) 2026 h9647123. Scientific glyphs are derived from Noto fonts and retain the corresponding SIL OFL terms. Third-party dependencies retain their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md).

v1.1.45 uses 32-bit JSON string lengths and fixes reading/saving 70 KB long-thinking records across a 32 KB loading window; corrupt records block overwrites. Original history, the browsing window, and model-context budgets are managed separately. Drafts and partial ASR results reuse unchanged history layout instead of parsing the entire conversation on every character update. Layout for common formulas, fractions, scripts, and Markdown has also been updated.

This edition retains the original search implementation. It does not include the CAM multi-provider OFF/AUTO search workflow, camera, or TF support. The CUSTOM IAT 60-second budget includes a 58.2-second body and up to 1.8 seconds of trailing audio. Actual availability depends on the server and network; initial connection and recognition results are not instantaneous.
