# ESP32 VoiceAssistant · 原版

基于 ESP32-S3 的中文语音助手：按键录音、语音识别、大模型流式问答、联网搜索、理科符号显示，以及 AHT20/BMP280 温湿度与气压显示。使用 ST7789 240×240 屏和手写 UI，无需 LVGL。

当前发布版本：**v1.1.43-i2c-aht20**。本仓库是保留环境传感器的原版，摄像头、相册、带图问答及录像功能请见独立的 [ESP32-CAM-VoiceAssistant](https://github.com/h9647123/ESP32-CAM-VoiceAssistant)。两个项目的硬件配置与固件分别维护。

## 功能

| 模块 | 功能 |
|---|---|
| 语音识别 | 保留讯飞 RTASR、独立讯飞 IAT 2.0、自建讯飞兼容 CUSTOM IAT，以及火山接口；分别保存配置 |
| 采音 | INMP441、16 kHz 单声道 PCM16；IAT V2/CUSTOM 使用 PSRAM 缓冲采音，录完后上传，记录削顶、I2S/软件溢出与传输统计 |
| 对话 | OpenAI 兼容聊天接口，多 Provider/模型/提示词；5 个本地会话，支持历史上下文 |
| 思考与搜索 | 思考模式和思考显示独立控制；搜索、Agent 搜索与直接回答；长思考内容有界保留，避免内存及排版成本无限增长 |
| 文字 | 中文/英文界面、Markdown 文本整理、常用 LaTeX 转换、理科符号、行内上下标；不是完整 LaTeX 排版引擎 |
| 界面 | 设置、UI、聊天、AI、系统五组菜单；卡片样式、滚动和页面切换动画；亮度调整、最长 30 分钟休眠 |
| 时钟与传感器 | NTP 时间、全屏时钟、AHT20 温湿度、BMP280 气压，离屏绘制与变化刷新 |
| 服务状态 | 联网后自动检测，空闲时每 60 秒检查 ASR 连接；离线手动检测立即返回 |
| 配置 | USB 串口与局域网 Web 页面，NVS 保存配置；配置导入导出含敏感信息，仅用于可信设备 |

CUSTOM IAT 最长录音为 60 秒并预留尾音缓冲，其他路径保留各自限制；实际可用时长也受 PSRAM、服务端策略及网络影响。自建识别服务由使用者另行部署，固件不包含 WeType、Windows VM 或服务器组件。

## 硬件与接线

验证硬件为 ESP32-S3、16 MB Flash、8 MB OPI PSRAM。发布模板默认启用 AHT20/BMP280 I2C 版本；源码仍保留 DHT 兼容路径。

| 器件 | ESP32-S3 GPIO / 说明 |
|---|---|
| ST7789 / GMT130 240×240 | MOSI 11、SCLK 12、RST 8、DC 7、背光 9；CS 为 -1，依板级连接确定 |
| INMP441 | BCLK 4、WS 5、DIN 6；L/R 接 GND 使用左声道 |
| AHT20 + BMP280 | SDA 18、SCL 17；AHT20 地址 0x38，BMP280 探测 0x76/0x77 |
| A / B / C 按键 | GPIO 13 / 14 / 15，按下接地，内部上拉 |
| 串口 | 验证板使用 UART/CH340，115200 波特率；端口号以当前电脑为准 |

请先核对自己的硬件接线、电压及模组规格，再编译与烧录。CAM 固件的摄像头及 TF 引脚不能用于本板。

## 编译

已验证工具版本：Arduino ESP32 **3.3.8**、ArduinoJson **7.4.3**、LovyanGFX **1.2.21**、WebSockets **2.7.2**、zlib_turbo **1.0.0**、DHT sensor library for ESPx **1.19**。

```powershell
# 安装 Arduino CLI 后，在仓库根目录执行。
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.8 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "ArduinoJson@7.4.3" "LovyanGFX@1.2.21" "WebSockets@2.7.2" "zlib_turbo@1.0.0" "DHT sensor library for ESPx@1.19"
Copy-Item VoiceAssistant/config.h.example VoiceAssistant/config.h
powershell -NoProfile -ExecutionPolicy Bypass -File tools/build-and-flash.ps1
```

标准 FQBN：

```text
esp32:esp32:esp32s3:PartitionScheme=huge_app,PSRAM=opi,FlashSize=16M,USBMode=hwcdc,CDCOnBoot=default
```

`PSRAM=opi` 必须与模组一致，不能省略：缺少 PSRAM 时，真实历史会话与 TLS 容易出现分配失败。不要沿用旧笔记中的 `CDCOnBoot=cdc`，验证板的 UART 控制台曾因此没有输出。

发布仓库不含真实 `config.h`，空凭据模板不会连接开发者的 Wi-Fi 或服务。首次烧录前应备份已有 Flash，核对芯片、分区表和端口。`build-and-flash.ps1 -Upload -PortName COMx` 使用 Arduino 常规上传流程，可能写入启动相关区域；它不是自动备份或受保护的应用分区升级器。

## 首次配置与操作

打开 115200 串口，输入 `help` 查看完整指令。网络、模型和 ASR 凭据通过本地串口或可信局域网配置，保存到 NVS。操作示例使用占位符，请替换为自己的值，勿把真实配置提交到 Git。

```text
connect <SSID> <password>
provideradd <name> <OpenAI-compatible-base-url> <api_key>
xfyun2asr <appid> <apikey> <apisecret>
asrprovider xfyun2
servicecheck
ui status
```

其他 ASR 配置及自建 WebSocket 端点见 [配置说明](docs/SETUP.md)。Web 页面通过 `webui` 指令查看状态或启停；只在可信网络使用，不应直接暴露到公网。

| 页面 | 操作 |
|---|---|
| 主界面 | 短按 A 进入菜单；长按 A 录音、松开识别；B/C 浏览文字 |
| 对话浏览 | 长按 B/C 连续滚动；按住 B/C 再短按 A 快速跳转轮次 |
| 草稿 | 按界面提示确认发送或继续追加语音 |
| 菜单 | B/C 选择，A 进入；可调值按 A 编辑，再用 B/C 减增，A 完成 |
| 返回 | 子菜单长按 A 返回上级，根菜单长按 A 返回主界面；Wi-Fi 子项保留长按扫描快捷操作 |
| 休眠 | 任意键先唤醒，随后再操作；唤醒提示约 0.8 秒 |

## 开发与验证

从早期语音与三键 UI，到多模型/搜索、Web 配置、数学符号、缓冲采音及本次 CAM 通用能力回移，概览见 [开发记录](docs/DEVELOPMENT.md) 与 [更新日志](CHANGELOG.md)。

发布前已对空凭据模板单独编译，并运行可独立执行的主机压力测试。原板 v1.1.43 已完成应用分区刷写校验，以及中英文布局、菜单进入/返回、传感器、休眠上限、轮次组合键和离线检测等串口回归。**串口回归、合成流压力测试、真人语音准确率和实体界面观感是不同的验证层级**。本次未重新执行真人语音及真实长思考模型验收，历史测试不能视作所有新环境的保证。详见 [验证说明](docs/VALIDATION.md)。

主机测试只需 Python、C++17 编译器和 ArduinoJson 头文件，不依赖 CAM 仓库、私有备份或联网服务：

```powershell
python tools/test-host.py --compiler "C:/path/to/zig.exe" --arduinojson "C:/path/to/ArduinoJson/src"
```

也支持 `--compiler clang++` 或 `g++`。见 [工具说明](tools/README.md)。

## 许可证

原创代码按 [MIT License](LICENSE) 开源，Copyright (c) 2026 h9647123。科学字形来自 Noto 字体，保留对应 SIL OFL 条款；第三方依赖仍遵循各自许可，参见 [第三方声明](THIRD_PARTY_NOTICES.md)。
