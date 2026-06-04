# CubeAI 板端接入准备

## 1. 板端输入原则

CubeAI 板端输入仍然保持：

- `160x120`
- 单通道
- 解包后的原始 `temp14`

不要使用：

- LCD 伪彩预览图
- RGB565 显示缓冲
- YUV 画面

## 2. 预处理

PC 侧和板端统一使用：

```text
temp_c = raw_temp14 / 16.0 - 273.15
normalized = clip((temp_c - 0.0) / 150.0, 0.0, 1.0)
```

## 3. 当前检测模型输出

当前模型不是 YOLO，而是轻量级 anchor-free grid detector。

输出是一张 `15x20` 的检测网格，每个网格预测：

- `objectness`
- `bbox`
- `class logits`

后处理包括：

- `sigmoid`
- 类别选择
- bbox 解码
- NMS

## 4. 当前坐标方向

当前版本已经把串口温度流方向和默认 LCD 方向对齐：

- MCU preview 默认 `mirror + flip`
- UART 温度流也按当前 preview 方向输出

这样如果你在 PC 侧训练/验证得到一个检测框坐标，再映射回当前默认预览方向时，就不会出现整体反转。

## 5. 当 MCU 端切换镜像/翻转时

这版还补了固件坐标变换接口：

- `tiny1c_thermal_app_transform_frame_point()`

它可以把原始 temp14 坐标映射到当前 preview 方向。

所以如果以后板端 CubeAI 推理仍然直接吃原始 `g_tiny1c_temp14_frame[]`，那也没问题：

1. AI 在原始 temp14 坐标系里输出 bbox / center
2. UI 叠加前，用 `tiny1c_thermal_app_transform_frame_point()` 把坐标映射到当前镜像/翻转后的显示方向
3. 再按实际显示尺寸缩放到预览区

这样用户在设置页切换 `镜像 / 翻转` 时，检测框也能同步跟着变。

## 6. 建议接入顺序

1. 先把检测训练、TFLite 导出、PC 验证跑通
2. 再用 CubeAI 导入 `best_model.keras` 或验证过的 `.tflite`
3. 板端先只做：
   - 预处理
   - 推理
   - bbox 解码
   - `tiny1c_thermal_app_transform_frame_point()` 坐标变换
   - UI 叠加
4. 最后再做置信度阈值和显示策略微调
