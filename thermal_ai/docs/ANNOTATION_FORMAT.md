# 热图标注格式

当前热图定位方案使用**中心点标注**，不是画框。

推荐直接使用：

- [`thermal_ai/scripts/annotate_centerpoints.py`](../scripts/annotate_centerpoints.py)

## 1. 文件位置

每个 `.bin` 文件对应一个同名 `.json` 标注文件：

```text
thermal_ai_dataset/raw/person/session_a/frame_0001.bin
thermal_ai_dataset/raw/person/session_a/frame_0001.json
```

切分后会自动复制到 `processed` 目录下。

## 2. JSON 格式

非空目标示例：

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

## 3. 字段说明

| 字段 | 含义 |
| --- | --- |
| `class_name` | 当前样本类别 |
| `center_x` | 目标中心点 X 坐标，像素坐标 |
| `center_y` | 目标中心点 Y 坐标，像素坐标 |
| `heatmap_sigma_px` | 高斯热图的扩散半径，默认可省略 |
| `empty` | 是否为空场景 |

## 4. 约定

- 坐标基于 **160x120** 原始温度矩阵
- 左上角为 `(0, 0)`
- `empty` 类不要求中心点
- 非空类必须提供中心点，否则热图头无法训练

## 5. 推荐标注原则

- 人体和手部尽量标在**最能代表主体的位置**
- 热物体标在**最高温区域中心**
- 电路板热点标在**热点中心**
- 如果目标很大，优先标“视觉中心”，不要随便漂
