# STM32N6 热成像 AI

## 1. 项目范围

这个目录用于给现有 STM32N6 热成像工程增加一套**轻量级五分类 + 热图定位**流程，同时不影响当前稳定的热像显示主链。

当前五分类目标：

| 标签 | 含义 |
| --- | --- |
| `empty` | 画面中没有明显目标 |
| `person` | 画面主体是人体 |
| `hand` | 画面主体是手部 |
| `hot_object` | 画面主体是独立热物体 |
| `circuit_board_hotspot` | 画面主体是电路板或电子热点 |

当前方案中：

- **分类头**负责判断当前主类别
- **热图头**负责给出目标大概位置

关键约束：

- **不做目标检测**
- **不做 YOLO**
- **不使用伪彩作为 AI 输入**
- **板端输入始终是原始 `temp14`**
- **训练阶段保持原始 `160x120` 单通道温度矩阵**
- **部署路径只走 ST CubeAI**

## 2. 输入约定

| 项目 | 值 |
| --- | --- |
| 语义分辨率 | **160x120** |
| Tensor 布局 | **NHWC** |
| TensorFlow 输入形状 | **120x160x1** |
| 原始数据类型 | **小端 `uint16`** |
| 原始温度语义 | **`temp14 = kelvin * 16`** |
| 主训练输入 | **`.bin`** |
| `.pgm` 作用 | 仅用于人工检查 |

所有训练 / 导出 / 验证脚本统一使用固定归一化：

```text
temp_c = raw_temp14 / 16.0 - 273.15
normalized = clip((temp_c - 0.0) / 150.0, 0.0, 1.0)
```

这套归一化是固定写死的，目的是保证 **PC 端和 STM32 端一致**。

## 3. 目录结构

当前项目结构：

```text
thermal_ai/
  artifacts/
    models/
    reports/
  configs/
  docs/
  scripts/
  src/
  README.md
  requirements.txt
```

原始数据建议结构：

```text
thermal_ai_dataset/
  raw/
    empty/
    person/
    hand/
    hot_object/
    circuit_board_hotspot/
```

更推荐的按会话整理方式：

```text
thermal_ai_dataset/
  raw/
    person/
      session_20260518_lab_a/
        person_0001.bin
        person_0002.bin
    hand/
      session_20260518_lab_b/
        hand_0001.bin
```

非空类样本还需要同名的中心点标注文件：

```text
frame_0001.bin
frame_0001.json
```

具体格式见：

- [`thermal_ai/docs/ANNOTATION_FORMAT.md`](./docs/ANNOTATION_FORMAT.md)

切分后的输出目录：

```text
thermal_ai_dataset/
  processed/
    train/
    val/
    test/
    split_manifest.csv
    split_summary.json
```

## 4. 复用现有工具

这套流程是围绕当前工程里的工具扩展的，不是另起一套无关流程：

| 现有文件 | 复用方式 |
| --- | --- |
| [`tools/uart_temp14_parser.py`](../tools/uart_temp14_parser.py) | 采集 UART 温度流并生成 `.bin` |
| [`tools/pgm_to_png.py`](../tools/pgm_to_png.py) | 仅用于人工预览转换 |

采集示例：

```powershell
python tools\uart_temp14_parser.py --port COM3 --baud 921600 --save-bin --save-pgm
```

然后把生成的 `.bin` 手动移动到正确的目录：

```text
thermal_ai_dataset/raw/<类别>/<session>/
```

## 5. 脚本清单

| 脚本 | 作用 |
| --- | --- |
| [`thermal_ai/scripts/split_dataset.py`](./scripts/split_dataset.py) | 按会话或文件名前缀切分 `train/val/test` |
| [`thermal_ai/scripts/label_check.py`](./scripts/label_check.py) | 检查标签目录和标注文件数量 |
| [`thermal_ai/scripts/check_dataset.py`](./scripts/check_dataset.py) | 检查切分后数据、中心点标注、温度范围是否合法 |
| [`thermal_ai/scripts/inspect_samples.py`](./scripts/inspect_samples.py) | 打印样本统计信息，并可导出预览图 |
| [`thermal_ai/scripts/visualize_random_samples.py`](./scripts/visualize_random_samples.py) | 生成随机拼图预览 |
| [`thermal_ai/scripts/annotate_centerpoints.py`](./scripts/annotate_centerpoints.py) | 交互式点击标注中心点 |
| [`thermal_ai/scripts/train_cnn.py`](./scripts/train_cnn.py) | 训练轻量 CNN 并输出报告 |
| [`thermal_ai/scripts/export_tflite.py`](./scripts/export_tflite.py) | 导出 float32 / int8 TFLite |
| [`thermal_ai/scripts/validate_tflite.py`](./scripts/validate_tflite.py) | 对 TFLite 做推理验证并与 Keras 对比 |

可复用的核心模块：

- [`thermal_ai/src/common.py`](./src/common.py)
- [`thermal_ai/src/model.py`](./src/model.py)
- [`thermal_ai/src/metrics.py`](./src/metrics.py)
- [`thermal_ai/src/tflite_utils.py`](./src/tflite_utils.py)

共享环境启动器：

- [`thermal_ai/scripts/run_with_shared_venv.ps1`](./scripts/run_with_shared_venv.ps1)

  它会复用：
  `D:\PracticeProject\thermal_model_tflite\.venv\Scripts\python.exe`

执行命令总览：

- [`thermal_ai/docs/EXECUTION_COMMANDS.md`](./docs/EXECUTION_COMMANDS.md)

标注格式说明：

- [`thermal_ai/docs/ANNOTATION_FORMAT.md`](./docs/ANNOTATION_FORMAT.md)

标注工具说明：

- [`thermal_ai/docs/ANNOTATE_CENTERPOINTS_USAGE.md`](./docs/ANNOTATE_CENTERPOINTS_USAGE.md)

## 6. 快速开始

### 6.1 切分数据集

```powershell
python thermal_ai\scripts\split_dataset.py --overwrite
```

### 6.2 检查标签

```powershell
python thermal_ai\scripts\label_check.py --stage raw --root thermal_ai_dataset\raw
python thermal_ai\scripts\label_check.py --stage processed --root thermal_ai_dataset\processed
```

### 6.3 检查数据集

```powershell
python thermal_ai\scripts\check_dataset.py --split all --json-out thermal_ai\artifacts\reports\dataset_check.json
```

### 6.4 查看样本

```powershell
python thermal_ai\scripts\inspect_samples.py --split train --count 8 --save-dir thermal_ai\artifacts\reports\inspect_png
python thermal_ai\scripts\visualize_random_samples.py --split train --count 12 --output thermal_ai\artifacts\reports\train_sheet.png
```

### 6.5 训练模型

```powershell
python thermal_ai\scripts\train_cnn.py
```

训练输出：

- `thermal_ai/artifacts/models/best_model.keras`
- `thermal_ai/artifacts/models/final_model.keras`
- `thermal_ai/artifacts/models/class_names.json`
- `thermal_ai/artifacts/reports/training/confusion_matrix.csv`
- `thermal_ai/artifacts/reports/training/classification_report.txt`
- `thermal_ai/artifacts/reports/training/localization_report.txt`
- `thermal_ai/artifacts/reports/training/training_summary.json`

### 6.6 导出 TFLite

```powershell
python thermal_ai\scripts\export_tflite.py
```

导出输出：

- `best_model_float32.tflite`
- `best_model_int8.tflite`
- `thermal_ai/artifacts/reports/tflite/tflite_export_report.json`

导出报告会记录：

- **输入 shape**
- **输入 dtype**
- **输入 scale / zero point**
- **分类输出 shape / dtype / scale / zero point**
- **热图输出 shape / dtype / scale / zero point**

### 6.7 验证 TFLite

```powershell
python thermal_ai\scripts\validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```

### 6.8 复用现有 TensorFlow / Keras 环境

如果要复用现有环境：

```text
D:\PracticeProject\thermal_model_tflite\.venv
```

可以这样运行：

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
```

说明：

- 训练、导出、TFLite 验证已经可以直接复用这个共享环境
- 预览图 / 拼图导出还需要在该环境里安装 `Pillow`

## 7. 模型结构

当前网络结构是一个双头模型：

```text
Input(120,160,1)
Conv2D(16) + ReLU + MaxPool
Conv2D(32) + ReLU + MaxPool
Conv2D(64) + ReLU + MaxPool

分类头:
  GlobalAveragePooling
  Dense(64) + ReLU
  Dense(5) + Softmax

热图头:
  Conv2D + UpSampling
  Conv2D + UpSampling
  Conv2D + UpSampling
  1-channel heatmap
```

这个结构刻意保持简单，目的就是兼顾 **CubeAI 兼容性** 和 **嵌入式资源占用**。

## 8. 输出内容

`train_cnn.py` 会输出并保存：

- `train accuracy`
- `val accuracy`
- `test accuracy`
- `confusion matrix`
- `precision / recall / f1-score`
- `heatmap mae`
- `localization error px`

如果混淆明显，脚本还会给出补采建议，例如：

- `person` 和 `hand` 混淆
- `hot_object` 和 `circuit_board_hotspot` 混淆
- `empty` 背景误判

## 9. 不做的事

- 不做 YOLO 或检测
- 不用 RGB、LCD 预览图或伪彩作为主输入
- 不再增加第二套推理框架
- 不预先手写 MCU 推理内核
- 不改动当前稳定的 Tiny1C 显示 / UART 主链

## 10. 后续板端接入

板端准备说明在这里：

- [`thermal_ai/docs/CUBEAI_BOARD_PREP.md`](./docs/CUBEAI_BOARD_PREP.md)

核心规则保持不变：

- **MCU 输入始终是解包后的 `temp14`**
- **AI 只输出类别与置信度**
- **热点位置仍然由传统温度分析负责**
