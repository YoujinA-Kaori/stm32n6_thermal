# STM32N6 热成像目标检测 AI

这个目录用于给当前 `STM32N6 + Tiny1C` 热成像工程提供一套**轻量级目标检测**训练/导出/验证流程。

当前方案特点：
- 输入仍然只使用 `160x120 temp14`
- 不使用伪彩图作为 AI 主输入
- 不使用 YOLO
- 模型是**轻量级 anchor-free grid detector**
- 训练、量化、验证都围绕 **TensorFlow / Keras / TFLite**
- 板端部署路线仍然是 **ST CubeAI**

## 1. 当前检测类别

目录主类别仍然保留：
- `empty`
- `person`
- `hot_object`
- `circuit_board_normal`
- `circuit_board_abnormal_hotspot`

其中真正参与检测头预测的是 4 个非空类：
- `person`
- `hot_object`
- `circuit_board_normal`
- `circuit_board_abnormal_hotspot`

`empty` 只表示“当前帧没有检测目标”。

## 2. 关键方向约定

这次版本把“坐标系一致性”作为强约束处理：

- 固件侧 LCD 默认预览当前就是 **mirror + flip**
- UART 温度流现在也已经按**当前预览方向**打包输出
- 所以后续采集的新 `.bin`、标注工具预览、训练框坐标、检测结果坐标，默认都和你在 LCD 上看到的方向一致

这样就不会再出现：
- 串口温度流和屏幕画面互为反转
- 目标框跟踪坐标方向对不上

另外，固件里新增了：
- `tiny1c_thermal_app_transform_frame_point()`

后续如果 MCU 端做 CubeAI 检测框叠加，也可以直接复用这个坐标变换函数，让镜像/翻转开关和检测坐标同步。

## 3. 目录结构

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
    hot_object/
    circuit_board_normal/
    circuit_board_abnormal_hotspot/
  processed/
    train/
    val/
    test/
```

## 4. 标注格式

每个 `.bin` 对应一个同名 `.json`，现在是 **bbox 检测标注**，不是中心点：

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
      "class_name": "hot_object",
      "x_min": 108,
      "y_min": 40,
      "x_max": 136,
      "y_max": 74
    }
  ]
}
```

空场景：

```json
{
  "primary_class_name": "empty",
  "empty": true,
  "objects": []
}
```

详细说明见：
- [ANNOTATION_FORMAT.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/ANNOTATION_FORMAT.md)

## 5. 训练模型

当前检测模型是：
- 输入：`120x160x1`
- Backbone：3 层 `Conv + MaxPool`
- 输出：`15x20` 检测网格
- 每个网格预测：
  - `objectness`
  - `bbox(cx_offset, cy_offset, w, h)`
  - `class logits`

这比 YOLO 轻很多，也比之前“分类 + 热图”更直接适合目标检测演示。

## 6. 训练输出

训练后会保存：
- `thermal_ai/artifacts/models/best_model.keras`
- `thermal_ai/artifacts/models/final_model.keras`
- `thermal_ai/artifacts/models/class_names.json`
- `thermal_ai/artifacts/models/detection_class_names.json`
- `thermal_ai/artifacts/reports/training/detection_report.txt`
- `thermal_ai/artifacts/reports/training/collection_suggestions.txt`
- `thermal_ai/artifacts/reports/training/training_summary.json`

评估指标以检测为主：
- `precision`
- `recall`
- `f1-score`
- `mean IoU`
- `mean center error px`
- `exact image match ratio`

## 7. 标注工具

当前标注工具文件名仍然保留为：
- [annotate_centerpoints.py](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/scripts/annotate_centerpoints.py)

但功能已经改成：
- **bbox 拖拽标注**
- 多目标、多类别
- 保存 `x_min / y_min / x_max / y_max`

使用说明见：
- [ANNOTATE_CENTERPOINTS_USAGE.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/ANNOTATE_CENTERPOINTS_USAGE.md)

## 8. 执行命令

完整命令清单见：
- [EXECUTION_COMMANDS.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/EXECUTION_COMMANDS.md)

常用流程是：
1. 采集 `.bin`
2. 用 bbox 工具标注 `.json`
3. `split_dataset.py`
4. `check_dataset.py`
5. `train_cnn.py`
6. `export_tflite.py`
7. `validate_tflite.py`

## 9. 应用扩展建议

更偏产品化的后续功能建议见：
- [APPLICATION_SCENARIOS.md](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/docs/APPLICATION_SCENARIOS.md)
