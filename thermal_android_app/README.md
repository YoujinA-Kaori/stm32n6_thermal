# 热成像 Android App

STM32N6 热成像手机端查看器，使用 `USB OTG + CH340` 直连板子。

## APK

安装包位置：

```text
thermal_android_app/dist/thermal_android_app-debug.apk
```

## 使用方法

1. 手机接 `OTG`，把 CH340 板子插到手机上。
2. 安装 APK，首次打开后授权 USB 权限。
3. App 会自动连接首个可用串口，也可以手动切换后再点 `连接`。
4. 连接成功后即可查看热图、中心温度、最高温、最低温。
5. 可切换伪彩、调自动/手动温标、暂停、截图。

## 说明

- 协议与 `tools/uart_temp14_parser.py` 保持一致。
- 默认波特率是 `921600`。
- 如果手机不支持 OTG，无法直接连接 CH340。
