# 配置与排障

## 配置范围

复制 `VoiceAssistant/config.h.example` 为 `config.h` 后编译。模板不含 Wi-Fi、ASR 或 LLM 凭据；默认启用 AHT20/BMP280 的 GPIO18/17 I2C 路径。若使用 DHT 旧板，需要按真实电路修改模板中的传感器开关。`config.h` 已被 Git 忽略。

串口支持 `help`，空格分组与部分旧式合并指令均可使用。下面的尖括号均为占位符，不应原样发送：

```text
connect <SSID> <password>
provider add <provider-name> <base-url> <api-key>
model <model-id>
```

## ASR

各提供商配置独立。可在菜单切换，也可用串口：

```text
# 官方 RTASR
xfyunasr <appid> <apikey>
asrprovider xfyun

# 官方 IAT 2.0
xfyun2asr <appid> <apikey> <apisecret>
asrprovider xfyun2

# 自建讯飞兼容 WebSocket 服务
iaturl wss://asr.example.com:16457/v1
iatasr <appid> <apikey> <apisecret>
asrprovider custom
```

示例域名和端口仅说明 URL 格式，需替换为自己的服务器。CUSTOM 支持 WS/WSS、主机、端口和路径；官方端点与其凭据不被覆盖。公网应使用证书有效的 WSS，确保设备 NTP 时间正确。默认验证证书，调试用不安全 TLS 开关不应作为正常配置。

服务端可以只在录音结束后返回全文，不能据此认为设备会实时显示 partial。`servicecheck`/后台检测检查连接健康，不证明识别准确率；`asrcheck` 等诊断可能发送合成静音，使用前先看串口说明。

## 模型、搜索与会话

模型需支持 OpenAI 兼容聊天请求，思考开关及搜索能力取决于模型与服务端实现。配置后先测试短问答，再测试上下文、思考和联网搜索。保留 5 个原版本地会话，切换用 `session 1` 至 `session 5`，查看用 `session list`。

```text
enablethinking on
thinking show
service status
ui status
meminfo
```

Web 控制页用于可信局域网操作。配置导出、NVS、完整 Flash 和会话备份可能包含敏感信息，不应公开上传。清空会话、导入配置或测试填充会改变设备内容，执行前确认当前会话。

## 常见问题

| 现象 | 检查 |
|---|---|
| TLS 报内存分配失败 | 模组 PSRAM 类型、`PSRAM=opi`、内存统计及真实会话占用 |
| UART 没有固件输出 | 数据线、实际 UART/CH340 口、波特率及 `CDCOnBoot=default`；不要根据历史 COM 号盲写 |
| 服务健康但识别文本缺字 | 原始 PCM、削顶、I2S/缓冲溢出、上传样本数；连接成功不等于完整采音 |
| 自建 `/v1` GET 404 | WebSocket 服务可能拒绝普通 GET，应检查经过鉴权的 Upgrade 及实际音频/final 帧 |
| 录音后等待较久 | 分开查看连接、上传及等待 final 的耗时；公网传输测试不能用局域网结果替代 |
| 思考很长或回答截断 | 有限的思考/可见正文内存预算、服务端 finish_reason、总时限及网络中断 |
| 复杂公式仍显示不理想 | 只支持常用文本转换及行内上下标，复杂 LaTeX 宏和二维布局不完整 |

已编译程序不能替代 Flash 备份。更新旧设备前核对实际分区，保留可恢复镜像，并避免把摄像头版应用刷进原板。
