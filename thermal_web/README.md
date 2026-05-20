# 热成像网页查看器

STM32N6 热成像串口网页上位机。

## 启动

```powershell
pip install -r thermal_web/requirements.txt
python thermal_web/app.py
```

浏览器打开：

```text
http://127.0.0.1:8000
```

## 使用方法

1. 先确认板子已通过 `USB OTG + CH340` 连到电脑。
2. 打开网页后，点击 `刷新`。
3. 串口列表里选中对应设备，点击 `连接`。
4. 页面会实时显示热图、中心温度、最高温和最低温。
5. 需要时可切换伪彩、调温标、暂停或截图。

## 说明

- 默认使用 `TEMP14` 协议，和 `tools/uart_temp14_parser.py` 保持一致。
- 页面支持自动选择首个可用串口。
- 如果串口被别的软件占用，网页无法连接。
