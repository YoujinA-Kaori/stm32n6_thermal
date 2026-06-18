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

## 2. 预处理方案

当前推荐板端与 PC 训练侧保持完全一致，使用“相对背景温差归一化”：

```text
temp_c = raw_temp14 / 16.0 - 273.15
background_c = percentile(temp_c, 50)
delta_c = temp_c - background_c
normalized = clip((delta_c - (-8.0)) / (80.0 - (-8.0)), 0.0, 1.0)
```

含义：

- `background_c`：当前帧背景参考温度，默认取中位数
- `delta_c`：像素相对背景的温差
- `normalized`：最终送给模型的单通道输入

当前默认把温差上限放到 `80°C`，是为了减少电路板严重异常热点在归一化阶段过早截顶。

这样做的好处是：

- 环境整体升温或降温时，输入分布更稳定
- 人体、热点、板上热点相对背景更突出
- 训练集对不同季节、不同室温更不敏感

## 3. 为什么不直接做每帧 0~255 AGC

纯 AGC 虽然也能增强对比，但会把绝对温度关系压掉。

对你这个项目来说：

- 用纯 AGC 做“有没有目标”通常没问题
- 但做 `circuit_board_normal` 和 `circuit_board_abnormal_hotspot` 时，容易把绝对热异常信息弱化

所以当前更推荐：

- AI 输入：相对背景温差归一化
- 异常告警：仍结合原始 `temp14` 做绝对温差阈值判断

## 4. 当前检测模型输出

当前模型不是 YOLO，而是轻量化 `anchor-free grid detector`。

输出是一张 `15x20` 的检测网格，每个网格预测：

- `objectness`
- `bbox`
- `class logits`

后处理包括：

- `sigmoid`
- 类别选择
- bbox 解码
- NMS

## 5. 当前坐标方向

这一版已经把串口温度流方向和默认 LCD 方向对齐：

- MCU preview 默认 `mirror + flip`
- UART 导出的温度流也按当前 preview 方向输出

因此：

- 你在 PC 侧训练和验证用的坐标
- 后面板端叠加显示的坐标

默认是一致的，不再互为反转。

## 6. 当 MCU 端切换镜像或翻转时

固件里补了：

- `tiny1c_thermal_app_transform_frame_point()`

它的作用是把原始 `temp14` 坐标映射到当前预览方向。

板端叠加检测框时建议流程：

1. CubeAI 在原始 `temp14` 坐标系下输出 bbox 或中心点
2. 用 `tiny1c_thermal_app_transform_frame_point()` 做坐标方向映射
3. 再按显示尺寸缩放到预览区

这样用户在设置里切镜像、翻转时，检测框也会同步跟着走。

## 7. 异常告警建议

对于 `circuit_board_abnormal_hotspot`，不建议只看 AI 类别。

更稳的工程方案是：

1. AI 检到 `circuit_board_abnormal_hotspot`
2. 再结合原始 `temp14` 计算目标区与背景区温差
3. 再做连续帧确认
4. 满足条件后才进入真正告警

这样可以降低环境变化、单帧噪声、偶发误检带来的影响。

## 8. 建议接入顺序

1. 先完成训练、TFLite 导出、PC 侧验证
2. 再用 CubeAI 导入 `best_model.keras` 或已经验证过的 `.tflite`
3. 板端先做：
   - 预处理
   - 推理
   - bbox 解码
   - 坐标变换
   - UI 叠加
4. 最后再调阈值、告警策略和显示策略
