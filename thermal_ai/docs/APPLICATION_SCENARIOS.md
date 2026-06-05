# 实际应用场景与建议功能

当前检测类别已经升级为：

- `person`
- `hot_object`
- `circuit_board_normal`
- `circuit_board_abnormal_hotspot`

这套类别比单纯“五分类场景识别”更接近真实设备应用。下面是更值得继续加的方向。

## 1. 电路板巡检

最直接的应用场景：

- 检测整板是否存在
- 区分“正常工作板”与“异常热点板”
- 给出异常热点的大概位置

建议再加：

- `Abnormal Board Detected` 告警文案
- 最高温与环境温差联动阈值
- 连续 `N` 帧确认后再报警

这样更像真实巡检终端，而不是单帧演示。

## 2. 设备异常发热筛查

`hot_object` 和 `circuit_board_abnormal_hotspot` 组合起来，可以做：

- 发热器件筛查
- 局部异常升温提醒
- 热源靠近提醒

建议再加：

- 置信度阈值
- 温差阈值
- 告警等级

例如：

```text
if class == circuit_board_abnormal_hotspot
and max_temp - ambient_temp > threshold:
    raise warning
```

## 3. 人体安全交互

`person` 可以做：

- 人体靠近提醒

建议再加：

- `person + hot_object` 同屏时的安全提示

## 4. 自动截图留证

很值得加，实际感会明显增强。

建议：

- 检测到 `circuit_board_abnormal_hotspot` 自动截图
- 保存：
  - 时间戳
  - 类别
  - 最高温
  - 框坐标

这样后面无论比赛还是实际排查，都更像完整产品链路。

## 5. 连续帧稳定跟踪

目标检测上板后，最值得加的不是更复杂模型，而是：

- 连续帧跟踪
- 框稳定
- 置信度平滑

这样 UI 观感会好很多，也更适合嵌入式现场展示。

## 6. 优先级建议

如果只按“投入产出比”排序，我建议：

1. `circuit_board_normal / circuit_board_abnormal_hotspot` 数据采集与标注
2. 异常热点告警逻辑
3. 连续帧确认
4. 异常自动截图
5. 跨帧框稳定

## 7. 命名建议

不建议把异常类直接写成：

- `short_circuit`

更稳的是：

- `circuit_board_abnormal_hotspot`

因为热像通常能可靠证明“异常发热”，但不一定能只靠热图证明“就是短路”。  
这样命名在比赛答辩和实际应用里都更严谨。
