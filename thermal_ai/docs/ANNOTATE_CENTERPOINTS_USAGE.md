# 中心点标注工具使用说明

这个工具用于给热成像 `.bin` 样本补**中心点标注**，配合当前的“分类 + 热图头”训练方案使用。

工具文件：

- [`thermal_ai/scripts/annotate_centerpoints.py`](../scripts/annotate_centerpoints.py)

## 1. 作用

它会：

- 读取 `temp14 .bin`
- 自动生成灰度预览图
- 允许你鼠标点击目标中心
- 自动保存同名 `.json` 标注文件

适用场景：

- `person`
- `hand`
- `hot_object`
- `circuit_board_hotspot`

`empty` 场景可以直接标为空，不需要中心点。

## 2. 标注格式

每个 `.bin` 对应一个同名 `.json`：

```text
frame_0001.bin
frame_0001.json
```

非空样本示例：

```json
{
  "class_name": "person",
  "center_x": 82,
  "center_y": 56,
  "heatmap_sigma_px": 7.0
}
```

空场景示例：

```json
{
  "class_name": "empty",
  "empty": true
}
```

## 3. 启动方式

建议使用工作区自带 Python，并确保它能访问 `Pillow`：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
```

如果只标某个子目录，也可以这样：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw\person\session_20260519_a
```

## 4. 操作步骤

1. 打开工具后，会显示当前 `.bin` 的预览图。
2. 鼠标左键点击目标大概中心。
3. 按 `S` 或 `Enter` 保存并切到下一张。
4. 空场景按 `E`，会保存为空标注。
5. 如需删除当前标注，按 `Delete`。
6. 需要切换类别时，按数字键 `1-5`。

## 5. 快捷键

| 按键 | 功能 |
| --- | --- |
| 鼠标左键 | 设置中心点 |
| `S` / `Enter` | 保存并下一张 |
| `E` | 标记为空并保存 |
| `1-5` | 快速切换类别 |
| `Left` / `Right` | 上一张 / 下一张 |
| `Delete` / `Backspace` | 删除当前 JSON |
| `Q` / `Esc` | 退出 |

## 6. 标注原则

- 坐标基于 **160x120** 原始温度矩阵
- 左上角是 `(0, 0)`
- 非空类必须有中心点
- 中心点尽量标在**主体视觉中心**或**最高热区中心**
- 不需要像目标检测那样追求框边界

## 7. 建议工作流

1. 先把原始 `.bin` 按类别和 session 整理好。
2. 打开标注工具批量补 `.json`。
3. 再运行：
   ```powershell
   python thermal_ai\scripts\split_dataset.py --overwrite
   python thermal_ai\scripts\check_dataset.py --split all
   ```
4. 确认无误后再训练。

## 8. 注意事项

- 这个工具依赖 `Pillow`
- 共享 `.venv` 里当前没有 `Pillow`，所以建议直接用工作区 Python
- 不要把 `.bin` 和 `.json` 分开放丢失配对关系
