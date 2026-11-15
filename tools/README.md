# 原版主机回归与构建

在仓库根目录运行，Python 3 + C++17（已验证 Zig 0.14.1）+ ArduinoJson 7.4.3 src。测试不打开串口、不连接服务、不烧录，不要求另一个 CAM 项目目录。

```powershell
python tools/test-host.py --compiler C:/path/to/zig.exe --arduinojson C:/path/to/ArduinoJson/src
```

入口调用七组当前原版代码提取测试：历史长记录/保存保护、公式排版几何、滚动积分、IAT 批次/修正/尾音、草稿确认、历史布局缓存和连接等待期间的音量 UI。结果写到被忽略的 validation/；编译缓存位于 .build-host/。实际字体清晰度、真实语音准确率、网络可用性和物理设备必须另验，host PASS 不代表这些通过。

可单独运行 `test-v145-*.py`，使用 `ZIG_EXE` 或 `CXX` 指定编译器，`ARDUINOJSON_INCLUDE` 指向 src。旧版 host-tests/ 保留历史 fixture；当前入口使用 host-tests-v145/ 与本版生产源码。

`build-and-flash.ps1` 默认只编译；只有显式 -Upload 才写入用户选择的串口。先核实原版板型、分区和备份，不要将 CAM 引脚或固件用于本板。
