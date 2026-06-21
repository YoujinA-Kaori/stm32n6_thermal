# STM32N6 热成像目标检测 AI

这个目录用于给当前 `STM32N6 + Tiny1C` 热成像工程提供一套轻量化目标检测训练、导出、验证流程。

当前路线特点：

- 主输入始终是 `160x120` 的原始 `temp14`
- 不用伪彩图、不用 RGB 图、不走 YOLO
- 训练与部署围绕 `TensorFlow / Keras / TFLite / CubeAI`
- 标注、训练、导出、验证都集中放在 `thermal_ai/`
- 板端推理最终仍只走 `ST CubeAI`
- 当前最终交付产物以验证通过的 `.tflite` 为准，`.keras` 只是训练中间产物

## 当前检测类别

目录主类别保留为：

- `empty`
- `person`
- `circuit_board_normal`
- `circuit_board_abnormal_hotspot`

其中真正进入检测头预测的是 3 个非空类：

- `person`
- `circuit_board_normal`
- `circuit_board_abnormal_hotspot`

`empty` 只表示这一帧没有需要检测的目标。

## 归一化方案

当前已经从“固定 0~150 摄氏度线性裁剪”切换为更适合热成像检测的方案：

1. 先把 `temp14` 转成摄氏度
2. 计算当前帧背景参考温度
3. 用“像素温度 - 背景温度”得到相对温差图
4. 再把温差裁剪并映射到 `0~1`

当前默认配置：

- 背景参考：当前帧 `50% percentile`，也就是中位数
- 温差裁剪范围：`[-8, 60]` 摄氏度

公式：

```text
temp_c = raw_temp14 / 16.0 - 273.15
background_c = percentile(temp_c, 50)
delta_c = temp_c - background_c
normalized = clip((delta_c - (-8.0)) / (60.0 - (-8.0)), 0.0, 1.0)
```

这样做的目的不是保留绝对温度，而是增强“目标相对背景”的热对比，降低环境整体升温或降温带来的影响。

注意：

- 这套归一化适合 AI 检测输入
- 电路板异常告警不要只靠这张归一化图判断
- 工程上仍建议结合原始 `temp14` 做绝对温差阈值和连续帧确认

当前把 `delta_c_max` 设为 `60`，是为了在“人体 + PCB”统一单模型里做折中：

- 比 `40` 更不容易把板上异常热点过早压平
- 比 `80` 更能保住人体和普通热区的灰度层次

## 坐标方向约定

本轮已经统一了温度流、标注和检测坐标方向：

- LCD 默认预览方向
- UART 导出的 `temp14`
- 标注工具预览
- 训练后的检测框坐标

现在默认都按同一方向理解，不再是互为翻转。

固件里还补了：

- `tiny1c_thermal_app_transform_frame_point()`

后面如果板端要把检测结果叠加到当前预览图上，可以直接复用这个坐标变换接口，让镜像和翻转开关与检测框同步。

## 目录结构

```text
thermal_ai/
  artifacts/
    models/
    reports/
  configs/
  docs/
  scripts/
  src/
```

数据集目录：

```text
thermal_ai_dataset/
  raw/
    empty/
    person/
    circuit_board_normal/
    circuit_board_abnormal_hotspot/
  processed/
    train/
    val/
    test/
```

## 标注格式

每个 `.bin` 对应一个同名 `.json`，当前是 `bbox xyxy` 格式：

```json
{
  "primary_class_name": "person",
  "objects": [
    {
      "class_name": "person",
      "x_min": 28,
      "y_min": 18,
      "x_max": 84,
      "y_max": 108
    },
    {
      "class_name": "circuit_board_abnormal_hotspot",
      "x_min": 108,
      "y_min": 40,
      "x_max": 136,
      "y_max": 74
    }
  ]
}
```

空场景示例：

```json
{
  "primary_class_name": "empty",
  "empty": true,
  "objects": []
}
```

详细说明见：

- [ANNOTATION_FORMAT.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/ANNOTATION_FORMAT.md)

## 当前模型形式

当前不是 YOLO，而是轻量化 `anchor-free grid detector`：

- 输入：`120x160x1`
- Backbone：几层 `Conv + MaxPool`
- 输出：`15x20` 网格
- 每个网格预测：
  - `objectness`
  - `bbox(cx_offset, cy_offset, w, h)`
  - `class logits`

这样更容易兼容 CubeAI，也更适合当前板端资源约束。

## 训练产物

训练后会保存：

- `thermal_ai/artifacts/models/best_model.keras`
- `thermal_ai/artifacts/models/final_model.keras`
- `thermal_ai/artifacts/models/best_model.tflite`
- `thermal_ai/artifacts/models/best_model_int8.tflite`
- `thermal_ai/artifacts/models/class_names.json`
- `thermal_ai/artifacts/models/detection_class_names.json`
- `thermal_ai/artifacts/reports/training/detection_report.txt`
- `thermal_ai/artifacts/reports/training/collection_suggestions.txt`
- `thermal_ai/artifacts/reports/training/training_summary.json`

主要评估指标：

- `precision`
- `recall`
- `f1-score`
- `mean IoU`
- `mean center error px`
- `exact image match ratio`

其中真正给 CubeAI / 板端部署的最终文件，应优先使用验证通过的 `.tflite`，通常是：

- `thermal_ai/artifacts/models/best_model_int8.tflite`

## 标注工具

标注工具文件名仍然是：

- [annotate_centerpoints.py](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/scripts/annotate_centerpoints.py)

但功能已经是：

- 多目标 bbox 标注
- 多类别混合标注
- 保存 `x_min / y_min / x_max / y_max`

说明见：

- [ANNOTATE_CENTERPOINTS_USAGE.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/ANNOTATE_CENTERPOINTS_USAGE.md)

## 执行流程

常用闭环：

1. 采集 `.bin`
2. 用标注工具画 `.json`
3. 运行 `split_dataset.py`
4. 运行 `check_dataset.py`
5. 运行 `train_cnn.py`
6. 运行 `export_tflite.py`
7. 运行 `validate_tflite.py`

命令清单见：

- [EXECUTION_COMMANDS.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/EXECUTION_COMMANDS.md)

## 板端接入建议

CubeAI 接入说明见：

- [CUBEAI_BOARD_PREP.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/CUBEAI_BOARD_PREP.md)

如果后续要继续往成品化走，应用方向建议见：

- [APPLICATION_SCENARIOS.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/APPLICATION_SCENARIOS.md)
