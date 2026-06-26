# 热成像检测标注格式

当前检测版使用的是 `bbox` 标注，不是中心点标注。

## 1. 文件位置

每个 `.bin` 样本对应一个同名 `.json`：

```text
thermal_ai_dataset/raw/person/session_a/frame_0001.bin
thermal_ai_dataset/raw/person/session_a/frame_0001.json
```

切分后，`.json` 会和 `.bin` 一起复制到 `processed/`。

## 2. JSON 格式

### 2.1 非空样本

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

### 2.2 空场景

```json
{
  "primary_class_name": "empty",
  "empty": true,
  "objects": []
}
```

## 3. 字段说明

| 字段 | 含义 |
| --- | --- |
| `primary_class_name` | 当前样本的主类别，通常与样本所在目录一致 |
| `objects` | 当前样本中所有需要检测的目标列表 |
| `objects[].class_name` | 目标类别 |
| `objects[].x_min` | 左上角 X |
| `objects[].y_min` | 左上角 Y |
| `objects[].x_max` | 右下角 X |
| `objects[].y_max` | 右下角 Y |
| `empty` | 是否为空场景 |

## 4. 坐标规则

- 坐标基于 `160x120` 温度矩阵
- 左上角是 `(0, 0)`
- `x` 范围：`0 ~ 159`
- `y` 范围：`0 ~ 119`
- 必须满足：
  - `x_min < x_max`
  - `y_min < y_max`

## 5. 主类别归档规则

一张图里可以有多个目标，但文件只放到一个主类别目录。

例如一张图里同时有人和电路板：

```text
thermal_ai_dataset/raw/person/session_a/frame_0001.bin
thermal_ai_dataset/raw/person/session_a/frame_0001.json
```

此时：

- `primary_class_name` 写成 `person`
- `objects` 里把 `person` 和对应 PCB 类都标出来
- 不要把同一帧复制到多个类别目录

## 6. 标注原则

- `person / circuit_board_normal / circuit_board_abnormal_hotspot` 都用 `bbox`
- 一张图里有多个目标时，全部标出来
- `empty` 不需要框
- 框尽量贴合主体热区，不要过松
- 如果样本放在 `person` 目录下，标注里至少应包含一个 `person`

## 7. 分组规则怎么理解

`session` 或自动分组的主要作用，不是给模型“额外加标签”，而是给数据切分工具提供分组依据。

也就是说它主要服务于：

- `split_dataset.py` 的 train / val / test 切分
- 避免同一段连续样本同时出现在不同 split

所以当前更推荐：

- 不必把所有拍摄都拆成很多细碎 session
- 一小批相邻样本放一组就够
- 场景明显变化时，再手动切一个新组

## 8. 坐标方向说明

当前版本已经按“和 MCU 默认显示方向一致”的思路收口：

- 固件侧 UART 温度流已按当前 preview 的 mirror / flip 方向输出
- 标注工具里看到的预览方向，应与 MCU 默认 LCD 画面一致
- 后续检测框坐标也默认按这套方向解释
