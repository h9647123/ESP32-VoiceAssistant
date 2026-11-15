# ESP32 VoiceAssistant · 原版

基于 ESP32-S3 的中文语音助手：按键录音、语音识别、大模型流式问答、联网搜索、理科符号显示，以及 AHT20/BMP280 温湿度与气压显示。使用 ST7789 240×240 屏和手写 UI，无需 LVGL。

当前公开版本：**v1.1.45-i2c-aht20 · 2026-11-15**。本仓库保留原版 AHT20/BMP280 与五个 Flash 会话；摄像头、相册、带图问答及录像请见独立的 [ESP32-CAM-VoiceAssistant](https://github.com/h9647123/ESP32-CAM-VoiceAssistant)。两版引脚、固件和配置分别维护。

## 功能

| 模块 | 功能 |
|---|---|
| 语音识别 | 保留讯飞 RTASR、独立讯飞 IAT 2.0、自建讯飞兼容 CUSTOM IAT，以及火山接口；分别保存配置 |
| 采音 | INMP441、16 kHz 单声道 PCM16；讯飞 IAT 2.0 使用 PSRAM 独立采音任务与 40ms 分包并行发送；CUSTOM IAT 支持 200ms 分包边采音边上传、部分结果修正与最终 ACK；记录削顶、I2S/软件溢出与传输统计 |
| 对话 | OpenAI 兼容聊天接口，多 Provider/模型/提示词；5 个 SPIFFS 本地会话；UTF-8 平滑吐字、末行跟随、思考计时、串口显示正文 |
| 历史保护 | 内存浏览窗口与磁盘原文分离、已提交边界保留、临时文件校验及上一代备份、损坏加载拒绝覆盖；挂载失败不自动格式化 |
| 思考与搜索 | 思考模式和思考显示独立控制；搜索、Agent 搜索与直接回答；长思考内容有界保留，避免内存及排版成本无限增长 |
| 文字 | 常用数学/化学符号、较小清晰角标、分数与中文混排、实际粗体/标题和表格转为逐项可读内容；有限 LaTeX 适配，不是完整 TeX 引擎 |
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
| 菜单 | B/C 选择，A 进入；二值设置短 A 直接切换，多值设置 A 编辑、B/C 减增、A 完成 |
| 草稿删除 | 单 C 删一个 UTF-8 字符；双 C 请求清空整段，A 确认、B 取消、C 取消并继续删字；确认提示在重绘后保持 |
| 返回 | 子菜单长按 A 返回上级，根菜单长按 A 返回主界面；Wi-Fi 子项保留长按扫描快捷操作 |
| 休眠 | 任意键先唤醒，随后再操作；唤醒提示约 0.8 秒 |

## 开发与验证

从早期语音与三键 UI，到多模型/搜索、Web 配置、数学符号、缓冲采音及本次 CAM 通用能力回移，概览见 [开发记录](docs/DEVELOPMENT.md) 与 [更新日志](CHANGELOG.md)。

v1.1.45 的公开镜像从空凭据模板独立构建；当前主机回归可在没有 CAM 检出的情况下执行。开发设备完成应用区写入校验、SPIFFS 与升级前逐字节对比、AHT20 检测、二值菜单 A 切换与多值 B/C 编辑、草稿清空确认及串口截图审计。用户确认首次音量和录音中流式更新正常，同时反馈识别返回仍有延迟。公开空配置镜像没有再次实刷；其配置默认值与本地 NVS 不同，不能将本地镜像的实机验收冒充该公开 SHA 的验收。详见 [验证说明](docs/VALIDATION.md) 和 [Release 说明](docs/RELEASE-v1.1.45-i2c-aht20.md)。

主机测试只需 Python、C++17 编译器和 ArduinoJson 头文件，不依赖 CAM 仓库、私有备份或联网服务：

```powershell
python tools/test-host.py --compiler "C:/path/to/zig.exe" --arduinojson "C:/path/to/ArduinoJson/src"
```

也支持 `--compiler clang++` 或 `g++`。见 [工具说明](tools/README.md)。

## 许可证

原创代码按 [MIT License](LICENSE) 开源，Copyright (c) 2026 h9647123。科学字形来自 Noto 字体，保留对应 SIL OFL 条款；第三方依赖仍遵循各自许可，参见 [第三方声明](THIRD_PARTY_NOTICES.md)。


v1.1.45 使用 32 位 JSON 字符串长度，修复 70KB 长思考记录跨 32KB 加载窗口的读取与保存；损坏记录阻止覆盖。历史原文、浏览窗口和模型上下文预算分开处理。草稿和 ASR 部分结果复用未变化的历史排版，避免每个字都重新解析整段会话；常见公式、分式、角标及 Markdown 的排版也已更新。

本版保留原版搜索实现，没有移植 CAM 的多搜索服务商 OFF/AUTO 流程，不提供相机或 TF 功能。CUSTOM IAT 60 秒预算包含 58.2 秒主体和最多 1.8 秒尾音，真实可用性受服务端与网络影响；首次连接和识别返回并非零延迟。
