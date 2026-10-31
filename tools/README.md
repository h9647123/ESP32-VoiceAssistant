# 构建与主机测试工具

`build-and-flash.ps1` 固定原版已验证的 PSRAM、Flash、分区与 UART 构建选项，默认只编译。添加 `-Upload -PortName COMx` 将调用 Arduino 常规上传；先自行备份并核对端口和分区，不要把它当作受保护应用分区升级工具。

`test-host.py` 从当前 `.ino` / 头文件提取真实的思考/SSE、TLS 重试、菜单/草稿函数，再用轻量原生 API 夹具执行测试。不会接串口、联网、烧录或读私有备份，也不需要 CAM 仓库。

```powershell
python tools/test-host.py --compiler "C:/tools/zig/zig.exe" --arduinojson "C:/Users/YourName/Documents/Arduino/libraries/ArduinoJson/src"
```

也可用 C++17 `g++` / `clang++`；编译器需自行安装。输出写到已忽略的 `validation/host-tests`。检查内容：

- 大量 reasoning_content、隐藏思考、inline `<think>`、可见输出上限及最终回答保留。
- CUSTOM 60 秒、原有 12 秒边界、尾音采样与上传容量，常见 boxed/infty/sqrt 转换。
- 模拟 SDK 连接关闭清除证书配置，验证每次备用 DNS 重试前恢复信任；明文、本地、取消及正常成功路径。
- 五组/20 项菜单可达、传感器选项、思考项显隐、分段草稿去重。

这些是合成主机测试，不能替代真机显示、麦克风音质、真人识别、实际 TLS 服务或网络速度验收。项目早期本机脚本依赖私人文件、摄像设备或固定工作区，未作为可移植工具发布。
