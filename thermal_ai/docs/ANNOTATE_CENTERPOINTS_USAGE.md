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
- 自动按当前 AI 归一化方式生成灰度预览
- 用鼠标拖拽绘制目标框
- 支持一张图里标多个目标、多个类别
- 自动保存同名 `.json`

这里的“当前 AI 归一化方式”已经是：

- 相对背景温差归一化
- 也就是标注预览会更强调目标相对背景的热差

这和后面模型训练时看到的输入是一致的。

## 2. 启动方式

建议使用工作区自带 Python：

```powershell
C:\Users\26218\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe thermal_ai\scripts\annotate_centerpoints.py --input thermal_ai_dataset\raw
```

如果只想标某一个 session：

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
6. 标完后按 `S` 或 `Enter` 保存并切到下一张
7. 空场景按 `E`

## 4. 快捷键

| 按键 | 功能 |
| --- | --- |
| 鼠标左键拖拽 | 画一个框 |
| 鼠标右键 | 删除最近的框 |
| `1-4` | 切换类别 |
| `S` / `Enter` | 保存并下一张 |
| `E` | 标记为空并保存 |
| `Delete` / `Backspace` | 删除最后一个框 |
| `C` | 清空当前所有框 |
| `Left` / `Right` | 上一张 / 下一张 |
| `Q` / `Esc` | 退出 |

默认数字键映射：

1. `person`
2. `hot_object`
3. `circuit_board_normal`
4. `circuit_board_abnormal_hotspot`

## 5. 保存结果

示例：

```text
frame_0001.bin
frame_0001.json
```

JSON 示例：

```json
{
  "primary_class_name": "person",
  "objects": [
    { "class_name": "person", "x_min": 28, "y_min": 18, "x_max": 84, "y_max": 108 },
    { "class_name": "hot_object", "x_min": 108, "y_min": 40, "x_max": 136, "y_max": 74 }
  ]
}
```

## 6. 目录规则

- 放在 `empty` 目录下的样本，才能标记为 `empty`
- 如果样本在 `person/hot_object/...` 目录下，标注框里至少要有一个对应主类
- 多目标样本只放在一个主类别目录里，不要复制到多个类目录

## 7. 多目标场景怎么归类

如果一张图里既有人又有热点，不要纠结“副目标”。

建议规则：

- 这个样本主要想让模型学谁，就放到谁的主目录
- 其他目标仍然在同一张图里正常画框

例如：

- 画面主体是人，旁边还有热杯子
- 这张图可以放在 `person\session_xxx\`
- 同时标一个 `person` 框和一个 `hot_object` 框

## 8. 方向说明

当前工具按“和 MCU 默认画面一致”的方向理解样本。

也就是说：

- 你在工具里画的 bbox
- 未来推理出来的检测框坐标
- MCU 默认预览方向

现在是对齐的，不再是互为反转。
