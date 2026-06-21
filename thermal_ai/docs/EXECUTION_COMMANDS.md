# Thermal AI 执行命令

所有命令都建议在工程根目录执行：

```powershell
D:\PracticeProject\Stm32\stm32n6_thermal
```

## 1. 采集原始数据

当前固件默认串口温度流：

- 波特率：`2000000`
- 帧率：约 `3 fps`

采集命令：

```powershell
python tools\uart_temp14_parser.py --port COM6 --baud 2000000 --save-bin --save-pgm
```

原始数据建议整理成：

```text
thermal_ai_dataset\raw\<类别>\<session>\*.bin
thermal_ai_dataset\raw\<类别>\<session>\*.json
```

其中 `.json` 是后面标注工具生成的检测框标注。

## 2. 启动 bbox 标注工具

现在推荐直接从 `thermal_out/` 读原始帧，在工具里完成：

- 画 bbox
- 设主类别
- 保存时自动归类到 `thermal_ai_dataset/raw/<类别>/<session>/`

建议直接用当前工作区自带 Python 启动：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_out --session-name session_20260620_a
```

如果你已经手动整理过一批 `raw` 样本，也仍然可以直接打开某个现有 session：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw\person\session_20260603_a
```

说明：

- 这个工具虽然文件名还是 `annotate_centerpoints.py`
- 但现在已经是多目标 bbox 标注工具
- 预览图会自动先按当前 AI 归一化逻辑生成，再做一层仅用于显示的对比增强，不需要你手动先转 PNG
- 从 `thermal_out` 打开时，按 `S` 保存会把当前 `.bin` 以及同名 `.pgm/.csv` 一起移动到 `raw/<类别>/<session>/`
- 按 `P` 可以把“当前选中的框类别”设成这张样本的主文件夹类别
- 当前统一模型建议类别是：`person / circuit_board_normal / circuit_board_abnormal_hotspot`

## 3. 划分数据集

```powershell
python thermal_ai\scripts\split_dataset.py --overwrite
```

## 4. 检查标签目录

```powershell
python thermal_ai\scripts\label_check.py --stage raw --root thermal_ai_dataset\raw
python thermal_ai\scripts\label_check.py --stage processed --root thermal_ai_dataset\processed
```

## 5. 检查数据集质量

```powershell
python thermal_ai\scripts\check_dataset.py --split all --json-out thermal_ai\artifacts\reports\dataset_check.json
```

这个脚本会重点检查：

- 类别分布
- 空样本与非空样本是否匹配
- bbox 是否越界
- 同一检测网格是否发生过多碰撞

## 6. 检查单样本与归一化效果

只看一个 `.bin`：

```powershell
python thermal_ai\scripts\inspect_samples.py --input thermal_out\frame_00001804.bin
```

如果想顺便导出预览图：

```powershell
python thermal_ai\scripts\inspect_samples.py --input thermal_out\frame_00001804.bin --save-dir thermal_ai\artifacts\reports\inspect
```

当前输出除了原始温度统计，还会带上：

- `norm_mode`
- `bg_p`
- `bg_c`
- `delta_min_c`
- `delta_max_c`

这样你可以直接看出这一帧的背景参考温度和相对温差范围。

## 7. 随机可视化样本

```powershell
python thermal_ai\scripts\visualize_random_samples.py --split train --count 12 --output thermal_ai\artifacts\reports\train_sheet.png
```

## 8. 训练检测模型

训练、导出、验证建议统一复用共享虚拟环境：

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
```

## 9. 导出 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
```

默认会导出：

- `float32 TFLite`
- `int8 TFLite`

并且会自动做：

- representative dataset 量化
- Keras 与 TFLite 输出对比
- test 集检测指标对比

板端最终建议使用验证通过的 `.tflite`，通常优先选：

- `thermal_ai\artifacts\models\best_model_int8.tflite`

## 10. 验证 TFLite

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```

## 11. 可视化预测框

如果你想直观看模型到底在报哪些框，可以把测试集样本渲染成“灰度热像 + 真值框 + 预测框”：

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 visualize_predictions.py --model thermal_ai\artifacts\models\best_model_int8.tflite --split test --count 12
```

默认会输出到：

```text
thermal_ai\artifacts\reports\visualization\best_model_int8_test\
```

其中会生成：

- 多张单样本叠加图
- `contact_sheet.png`
- `summary.json`

## 12. 生成 CubeAI 验证输入

如果你要在 CubeMX / X-CUBE-AI 的 `Validation inputs -> Browse` 里喂真实样本，可以先导出一份和模型输入 dtype 对齐的 raw 文件：

```powershell
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_cubeai_validation_input.py --model thermal_ai\artifacts\models\best_model_int8.tflite --split test --class-name person --sample-index 0
```

默认会输出到：

```text
thermal_ai\artifacts\reports\cubeai_inputs\
```

其中会带：

- `input_model_dtype_nhwc.int8.raw`
- `input_model_dtype_flat.int8.csv`
- `input_float32_nhwc.raw`
- `output_float32_ghwc.raw`
- `preview.png`
- `metadata.json`
- `how_to_use_in_cubeai.txt`

在 CubeAI 里优先选：

- `Validation inputs -> Browse`
- 选择 `input_model_dtype_flat.int8.csv`

## 13. 当前共享环境

当前复用的训练环境：

```text
D:\PracticeProject\thermal_model_tflite\.venv\Scripts\python.exe
```

建议：

- 标注工具和不依赖 TensorFlow 的检查脚本，直接用工作区 Python
- 训练、导出、TFLite 验证，统一走 `run_with_shared_venv.ps1`

## 14. 最短闭环流程

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
python thermal_ai\scripts\split_dataset.py --overwrite
python thermal_ai\scripts\check_dataset.py --split all
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 train_cnn.py
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 export_tflite.py
powershell -ExecutionPolicy Bypass -File thermal_ai\scripts\run_with_shared_venv.ps1 validate_tflite.py --model thermal_ai\artifacts\models\best_model_int8.tflite --keras-model thermal_ai\artifacts\models\best_model.keras
```
