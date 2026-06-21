# 检测框标注工具使用说明

虽然脚本文件名仍然是：

- [annotate_centerpoints.py](D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/scripts/annotate_centerpoints.py)

但它现在已经不是中心点标注工具，而是：

- 多目标
- 多类别
- bbox 拖拽标注工具

## 1. 作用

它会：

- 读取 `temp14 .bin`
- 自动先按当前 AI 归一化方式生成灰度预览，再做一层仅用于显示的自适应对比拉伸
- 用鼠标拖拽绘制目标框
- 支持一张图里标多个目标、多个类别
- 自动保存同名 `.json`

这里的“当前 AI 归一化方式”已经是：

- 相对背景温差归一化
- 也就是标注预览会更强调目标相对背景的热差

也就是说：

- 预览图的基础温差语义仍然来自当前 AI 归一化
- 但为了更容易看清人体/PCB 热区边界，显示时会再做一层轻量对比增强
- 这层增强只影响预览显示，不会改训练输入

## 2. 启动方式

建议直接从 `thermal_out/` 读取原始帧，并在工具里完成分类：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_out --session-name session_20260620_a
```

保存时，工具会自动把当前样本归档到：

```text
thermal_ai_dataset/raw/<类别>/<session-name>/
```

如果你只想继续标某个已经归类好的 session：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw\person\session_20260603_a
```

## 3. 操作方法

1. 打开工具后，会显示当前 `.bin` 的灰度预览图
2. 按数字键选择当前要画的类别
3. 鼠标左键按下并拖拽，松开后生成一个 bbox
4. 一张图里有多个目标，就继续拖多个框
5. 如果框错了：
   - 鼠标右键删除最近的框
   - 或按 `Delete` 删除最后一个框
6. 如果当前是多目标混合场景，按 `P` 把“当前选中的框类别”设为这张样本的主文件夹类别
7. 标完后按 `S` 或 `Enter` 保存并切到下一张
8. 空场景按 `E`

## 4. 快捷键

| 按键 | 功能 |
| --- | --- |
| 鼠标左键拖拽 | 画一个框 |
| 鼠标右键 | 删除最近的框 |
| `1-3` | 切换类别 |
| `P` | 把当前选中的框类别设为主文件夹类别 |
| `S` / `Enter` | 保存并下一张 |
| `E` | 标记为空并保存 |
| `Delete` / `Backspace` | 删除最后一个框 |
| `C` | 清空当前所有框 |
| `Left` / `Right` | 上一张 / 下一张 |
| `Q` / `Esc` | 退出 |

默认数字键映射：

1. `person`
2. `circuit_board_normal`
3. `circuit_board_abnormal_hotspot`

## 5. 保存结果

从 `thermal_out` 导入时，保存后会得到：

```text
thermal_ai_dataset/raw/person/session_20260620_a/frame_0001.bin
thermal_ai_dataset/raw/person/session_20260620_a/frame_0001.json
```

JSON 示例：

```json
{
  "primary_class_name": "person",
  "objects": [
    { "class_name": "person", "x_min": 28, "y_min": 18, "x_max": 84, "y_max": 108 },
    { "class_name": "circuit_board_abnormal_hotspot", "x_min": 108, "y_min": 40, "x_max": 136, "y_max": 74 }
  ]
}
```

## 6. 目录规则

- 放在 `empty` 目录下的样本，才能标记为 `empty`
- 如果样本在 `person/circuit_board_normal/...` 目录下，标注框里至少要有一个对应主类
- 多目标样本只放在一个主类别目录里，不要复制到多个类目录

## 7. 多目标场景怎么归类

如果一张图里既有人又有 PCB，不要纠结“副目标”。

建议规则：

- 这个样本主要想让模型学谁，就把主文件夹类别设成谁
- 其他目标仍然在同一张图里正常画框

例如：

- 画面主体是人，旁边还有一块电路板
- 这张图可以把主文件夹类别设成 `person`
- 同时标一个 `person` 框和一个 `circuit_board_normal` 或 `circuit_board_abnormal_hotspot` 框

## 8. 方向说明

当前工具按“和 MCU 默认画面一致”的方向理解样本。

也就是说：

- 你在工具里画的 bbox
- 未来推理出来的检测框坐标
- MCU 默认预览方向

现在是对齐的，不再是互为反转。
