# Thermal AI 执行命令

所有命令都建议在工程根目录执行：

```powershell
D:\PracticeProject\Stm32\stm32n6_thermal
```

## 1. 采集原始数据

当前固件默认串口温度流：`2000000` 波特率，约 `3 fps`。

```powershell
python tools\uart_temp14_parser.py --port COM3 --baud 2000000 --save-bin --save-pgm
```

原始数据整理成：

```text
thermal_ai_dataset\raw\<类别>\<session>\*.bin
thermal_ai_dataset\raw\<类别>\<session>\*.json
```

## 2. bbox 标注

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
```

只标某个 session：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw\person\session_20260603_a
```

## 3. 切分数据集

```powershell
python thermal_ai\scripts\split_dataset.py --overwrite
```

## 4. 检查标签

```powershell
python thermal_ai\scripts\label_check.py --stage raw --root thermal_ai_dataset\raw
python thermal_ai\scripts\label_check.py --stage processed --root thermal_ai_dataset\processed
```

## 5. 检查数据集

```powershell
python thermal_ai\scripts\check_dataset.py --split all --json-out thermal_ai\artifacts\reports\dataset_check.json
```

## 6. 样本预览

只做数值检查：

```powershell
python thermal_ai\scripts\inspect_samples.py --input thermal_out\frame_00001804.bin
```

随机拼图：

```powershell
python thermal_ai\scripts\visualize_random_samples.py --split train --count 12 --output thermal_ai\artifacts\reports\train_sheet.png
```

## 7. 训练检测模型

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
```

## 8. 导出 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
```

## 9. 验证 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```

## 10. 共享环境

当前复用环境：

```text
D:\PracticeProject\thermal_model_tflite\.venv\Scripts\python.exe
```

建议：
- 标注工具和不依赖 TensorFlow 的检查脚本，直接用工作区 Python
- 训练 / 导出 / TFLite 验证，走 `run_with_shared_venv.ps1`

## 11. 最短流程

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
python thermal_ai\scripts\split_dataset.py --overwrite
python thermal_ai\scripts\check_dataset.py --split all
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```
