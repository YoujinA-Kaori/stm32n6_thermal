# CubeAI 板端准备说明

## 1. 板端输入规则

STM32 端给 AI 的输入必须满足：

- **160x120**
- **单通道**
- **解包后的原始 `temp14`**

不要使用：

- 伪彩预览图
- LCD 的 RGB565 画面
- YUV 显示图像
- 灰度预览图作为主路线输入

## 2. 预处理约定

当前 Python 侧统一使用固定线性归一化：

```text
temp_c = raw_temp14 / 16.0 - 273.15
normalized = clip((temp_c - 0.0) / 150.0, 0.0, 1.0)
```

推荐 MCU 侧用下面这种浮点写法保持一致：

```c
float temp_c = ((float)raw_temp14 / 16.0f) - 273.15f;
float input_value = (temp_c - 0.0f) / 150.0f;
if (input_value < 0.0f)
{
    input_value = 0.0f;
}
else if (input_value > 1.0f)
{
    input_value = 1.0f;
}
```

这一步应该直接作用在**已经解包好的温度矩阵**上，而不是任何预览图缓冲上。

## 3. CubeAI 部署顺序

推荐顺序：

1. 先训练并确认 `best_model.keras`
2. 再导出并验证 `best_model_float32.tflite`
3. 再导出并验证 `best_model_int8.tflite`
4. 把确认过的模型导入 **ST CubeAI**
5. 让 CubeAI 自动生成推理入口
6. 只补最少的胶水代码：
   - 原始 temp14 到归一化张量
   - 推理触发
   - 类别名映射
   - 置信度阈值处理

## 4. 板端输出规则

AI 当前建议输出：

- `class_id`
- `class_name`
- `confidence`
- `heatmap_peak_x`
- `heatmap_peak_y`

可选 UI 文案：

- `AI: person`
- `Confidence: 93.2%`
- `Target: (82, 56)`
- `Status: Human Detected`
- `uncertain`

建议的低置信度规则：

```text
if confidence < 0.60:
    show "uncertain"
```

## 5. 仍然保留在传统链路中的内容

以下功能仍然建议继续走传统温度分析，不放进 AI：

| 功能 | 建议归属 |
| --- | --- |
| 最高温点 | 传统温度分析 |
| 最低温点 | 传统温度分析 |
| 热点区域 | 传统温度分析 |
| 位置框 / 区域高亮 | 传统温度分析 |

AI 只负责**主场景分类**。

如果当前模型启用了热图头，则 AI 还能提供：

- 目标大概中心点
- 热图峰值位置

但完整外轮廓和精确热区边界仍然建议继续走传统算法。

## 6. 固件集成优先级

建议固件阶段按下面顺序接入：

1. 保持当前热像主链稳定
2. 在最新 `temp14` 帧上增加一次推理触发
3. 把最新 AI 结果存到一个很小的共享状态块里
4. 先只显示一行最小 UI
5. 等真实板子验证后再调置信度阈值

## 7. 建议的共享状态

推荐最小状态字段：

```text
class_id
confidence
frame_counter
valid_flag
```

这已经足够支持第一版演示，不需要引入不必要的 UI 或架构改动。
