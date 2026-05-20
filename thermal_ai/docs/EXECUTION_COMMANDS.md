# Thermal AI 执行命令

所有命令都建议在工程根目录执行：

```powershell
D:\PracticeProject\Stm32\stm32n6_thermal
```

## 1. 采集原始数据

```powershell
python tools\uart_temp14_parser.py --port COM3 --baud 921600 --save-bin --save-pgm
```

原始数据建议整理成：

```text
thermal_ai_dataset\raw\<类别>\<session>\*.bin
```

如果要训练“分类 + 热图头”，每个非空样本还需要一个同名的 `.json` 中心点标注文件：

```text
thermal_ai_dataset\raw\<类别>\<session>\*.json
```

## 2. 切分数据集

```powershell
python thermal_ai\scripts\split_dataset.py --overwrite
```

## 3. 检查标签

```powershell
python thermal_ai\scripts\label_check.py --stage raw --root thermal_ai_dataset\raw
python thermal_ai\scripts\label_check.py --stage processed --root thermal_ai_dataset\processed
```

## 4. 检查数据集

```powershell
python thermal_ai\scripts\check_dataset.py --split all --json-out thermal_ai\artifacts\reports\dataset_check.json
```

## 5. 查看样本

仅做数值检查：

```powershell
python thermal_ai\scripts\inspect_samples.py --input thermal_out\frame_00001804.bin
```

安装 Pillow 后可导出预览图：

```powershell
python thermal_ai\scripts\inspect_samples.py --split train --count 8 --save-dir thermal_ai\artifacts\reports\inspect_png
```

随机拼图：

```powershell
python thermal_ai\scripts\visualize_random_samples.py --split train --count 12 --output thermal_ai\artifacts\reports\train_sheet.png
```

## 6. 交互式标注中心点

请使用**带 Pillow 的 Python**，推荐直接用工作区自带解释器：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
```

如果只想标某一个目录，也可以传入具体子目录：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw\person\session_20260519_a
```

操作方式：

- 鼠标左键点击图像设置中心点
- `S` 或 `Enter` 保存并下一张
- `E` 标记为空并保存
- 数字 `1-5` 切换类别
- `Delete` 删除当前 JSON

详细说明见：

- [`thermal_ai/docs/ANNOTATE_CENTERPOINTS_USAGE.md`](./ANNOTATE_CENTERPOINTS_USAGE.md)

## 7. 训练模型

复用共享 TensorFlow / Keras 环境：

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
```

训练前请先确认：

- 非空类样本已有中心点标注
- `empty` 类可以没有中心点

## 8. 导出 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
```

## 9. 验证 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```

## 10. 共享环境

当前可复用的环境路径：

```text
D:\PracticeProject\thermal_model_tflite\.venv\Scripts\python.exe
```

这个环境里已经有 TensorFlow 和 Keras。  
Pillow 只在导出 PNG / 拼图时才需要。
