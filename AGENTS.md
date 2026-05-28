# stm32n6_thermal Agent Notes

## 项目定位

这是一个基于 **STM32N647 + Tiny1C(160x120)** 的热成像项目，当前 RTOS 为 **ThreadX**。

当前主目标不是做 MCU 本地伪彩算法，而是：

- LCD 显示 **Tiny1C 模组原生预览图像**
- UART 持续发送 **160x120 温度流**
- 后续 AI / 上位机优先使用 **温度流数据**


## 当前稳定主链

### 模组输出模式

当前默认使用 **Image + Temperature** 组合输出。

- 图像分辨率：`160x120`
- 温度分辨率：`160x120`
- MCU 接收组合帧逻辑高度：`160x240`
- 组合帧布局：
  - 上半帧：`YUV422 / YUYV` 图像
  - 下半帧：`Y14` 温度数据

相关实现：

- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_thermal_app.c`


### LCD 显示链路

当前 LCD 默认显示的是 **模组原生图像**，不是 MCU 用温度流重新做的伪彩。

链路如下：

`Tiny1C -> DCMIPP -> 组合帧拆分 -> YUV422转RGB565 -> 2x显示 -> LCD`

说明：

- 显示分辨率：`320x240`
- LCD 叠加：
  - 中心温度文本
  - 白色十字
- 模组侧伪彩模式可切换，默认值在：
  - `CFG_TINY1C_DEFAULT_PSEUDO_COLOR_MODE`

当前默认值：

- `PSEUDO_COLOR_MODE_5`

注意：

- 这里的“伪彩模式”指的是 **模组原生预览图像的伪彩**
- 不是 MCU 对温度流再做二次着色


### UART 温度流链路

UART 温度流线程仍保留，当前协议：

- 串口：`USART1 + GPDMA1`
- 波特率：`921600`
- 帧率：`1 fps`
- 数据尺寸：`160x120`
- 包类型：`TEMP14`

包头结构定义在：

- `Appli/Core/Src/app_threadx.c`

字段包括：

- `sync_word`
- `packet_type`
- `frame_counter`
- `frame_width`
- `frame_height`
- `payload_bytes`
- `center_temp14`
- `min_temp14`
- `max_temp14`
- `payload`

说明：

- 当前 `payload` 是 **解包后的 temp14 数据**
- `temp14` 语义为：`kelvin * 16`


## 关键数据格式结论

根据工作区文档 `RTOS平台DVP接口软件开发资料 Interface development instructionsV1.0.docx`：

- 每像素 `2 byte`
- **高 14 位有效**
- **小端存储**

因此 MCU 侧不能把收到的 16 位原值直接当温度算，必须先解包。

当前实现已经修正为：

- 在 `tiny1c_thermal_app_unpack_temp14_frame()` 中先做 `Y14` 解包
- 解包后的 `g_tiny1c_temp14_frame[]` 才作为：
  - 中心温度计算输入
  - UART 发送输入
  - 后续 AI / 上位机使用输入

如果再出现“中心温度 900 多度”这类异常，优先检查：

1. 是否又把原始 `Y14 packed word` 直接当 `temp14` 使用
2. 是否改坏了组合帧上下半帧偏移
3. 是否重新引入了未解包的旧路径


## 当前线程职责

### `thermal_thread`

文件：

- `Appli/Core/Src/app_threadx.c`

职责：

- 启动 Tiny1C 应用
- 驱动 `tiny1c_thermal_app_process()`
- 接收 DCMIPP 帧并触发显示处理


### `uart_stream_thread`

文件：

- `Appli/Core/Src/app_threadx.c`

职责：

- 从 `tiny1c_thermal_app_get_temp14_frame()` 取最新温度帧
- 打包 UART 温度流
- 通过 `USART1 DMA` 发出
- 计算中心点温度
- 将中心点补偿结果回写给 LCD 叠加层


## 当前温度补偿状态

### 已接入

`libirtemp.lib` 已接入工程，并通过桥接层解决了 `__hardfp_pow/__hardfp_sqrt` 链接问题。

相关文件：

- `Appli/Core/Inc/libirtemp.h`
- `Appli/Core/Inc/libirtemp_bridge.h`
- `Appli/Core/Src/libirtemp_bridge.c`
- `Appli/Core/Src/libirtemp_hardfp_math.c`
- `STM32CubeIDE/Appli/ThirdParty/libirtemp/`


### 当前实际生效范围

当前 **只对中心温度显示链路做了补偿接入**，还没有把 `libirtemp` 推进到整帧像素级温补。

也就是说：

- LCD 叠加的中心温度：经过补偿
- UART 温度流 `payload`：当前仍是解包后的原始 `temp14`
- 整帧显示：当前仍主要依赖模组原生图像链路


## 当前 GUI / LVGL 状态

### GUI 原型

当前已做出一套 `800x480` 横屏工业热像仪 GUI 原型，布局固定为：

- 顶部状态栏
- 左侧设置面板
- 右侧热成像预览区
- 底部快捷栏

主要文件：

- `STM32CubeIDE/Appli/gui_guider/generated/setup_scr_WidgetsDemo.c`
- `STM32CubeIDE/Appli/gui_guider/generated/gui_guider.h`
- `STM32CubeIDE/Appli/gui_guider/generated/events_init.c`


### GUI 与热像链路接入状态

已完成：

- `LVGL` 独立线程运行
- 触摸输入已接入 `LVGL`
- 右侧预览区接 Tiny1C 实时画面
- 伪彩切换已按驱动 `11` 档映射
- 增益切换已绑定
- 自动 `FFC` / 手动 `FFC` 已绑定
- 恢复默认已绑定
- 新增独立 **全屏预览页**
- 全屏页右上角返回按钮文案为 `设置`
- 全屏页保留：
  - 中心十字
  - 中心温度
  - 最高温 / 最低温
  - 右下角伪彩模式名
- 全屏页的热像画面当前不是 `LVGL zoom` 变换，而是：
  - 复用当前 `320x240` 预览源
  - 在 MCU 侧展开成 `640x480 RGB565` 独立缓冲
  - 再交给 LVGL 直接显示
- 最高温 / 最低温白色标记已加黑底圆角小牌子，避免与背景混色

仍属 UI 状态、未深入到底层显示逻辑的项目：

- 镜像
- 翻转
- 中心十字开关
- 中心温度显示开关
- 最高温/最低温标记开关
- 单位切换
- 保存配置持久化


### 当前全屏页实现与性能状态

本轮对话已完成以下实现与调优：

- 全屏页使用单独的 LVGL screen，不再把同一个预览对象在两个 screen 之间来回搬动
- 全屏热像图使用独立 `640x480` 缓冲：
  - `Appli/Core/Src/app_threadx.c`
  - `g_gui_fullscreen_rgb565_frame[]`
- 当前全屏缩放策略是 **整数 2x 复制**，没有引入双线性等插值
- 为降低 GUI 压力，温标叠加刷新已和图像刷新拆开：
  - 热像图像仍按新帧实时刷新
  - 叠加层刷新周期：
    - `CFG_GUI_OVERLAY_UPDATE_PERIOD_MS = 120`
- 为避免在 GUI 线程里同步查模组导致掉帧，最高温 / 最低温坐标查询已改为独立低优先级线程缓存：
  - `extrema_query_thread`
  - `CFG_EXTREMA_QUERY_PERIOD_MS = 200`
  - GUI 线程只读取缓存，不在刷新路径里直接调用 `tiny1c_vdcmd_get_frame_max_min_temp()`

当前结论：

- 全屏模式比普通预览仍更吃资源，这是预期行为
- 当前全屏性能开销主要来自：
  - `320x240 -> 640x480` 的整帧 RGB565 展开
  - 全屏页独立图像缓冲占用额外 EXTRAM
- 如果后续仍觉得全屏帧率偏低，优先考虑：
  - 把全屏尺寸从 `640x480` 降到 `480x360`
  - 或直接改为 `320x240` 居中显示
  - 不要优先回到 `LVGL zoom` 方案

### 当前高低温点状态

本轮对话里，高低温标记跟随曾出现“有时固定在同一位置”的观感问题。

已做调整：

- 最高温 / 最低温点的来源不再直接放在 GUI 刷新线程里同步查模组
- 当前由独立线程定期调用：
  - `tiny1c_vdcmd_get_frame_max_min_temp()`
- 结果写入缓存后，再由 GUI 线程读取缓存并更新 marker
- 如果缓存无效，GUI 路径仍会回退到本地扫描 `temp14_frame[]`

注意：

- 由于当前极值缓存线程是 `200 ms` 周期，所以高低温点的响应不会是“每帧级”抖动
- 这属于有意的稳定性/性能折中，不要误判为 bug


### 当前 GUI 启动链稳定性结论

本轮做过以下稳定性修正：

- Tiny1C 启动失败不再直接进 `Error_Handler()`
- GUI 不再死等模块启动
- `LVGL` 首帧会主动触发刷新
- 触摸初始化不再阻塞首帧显示
- 避免在模块启动路径里再次清黑预览界面

涉及文件：

- `Appli/Core/Src/app_threadx.c`
- `Appli/Core/Src/main.c`
- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_thermal_app.c`
- `STM32CubeIDE/Appli/lvgl/porting/lv_port_indev.c`


## 当前中文字体状态

### 问题背景

原先尝试直接使用：

- `lv_font_SourceHanSerifSC_Regular_18`

结论是：

- 大面积挂载该字体时，界面会不稳定，曾出现黑屏
- 小范围挂载时虽然有时可亮屏，但仍不适合作为最终方案


### 当前方案

当前已经生成一个 **超小中文子集字体**，只覆盖本界面实际用到的汉字：

- `STM32CubeIDE/Appli/gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c`

生成脚本：

- `tools/gen_lvgl_subset_font.py`

当前界面中文字体入口：

- `STM32CubeIDE/Appli/gui_guider/generated/gui_guider.h`
- `STM32CubeIDE/Appli/gui_guider/generated/gui_guider.c`
- `STM32CubeIDE/Appli/gui_guider/generated/setup_scr_WidgetsDemo.c`

字体策略：

- 中文控件用 `lv_font_thermal_cn_18`
- 英文/数字继续走 `lv_font_montserratMedium_19`
- 小字库本身带 `fallback` 到 `Montserrat`


### 最新状态

本轮已确认：

- 字库本身覆盖了 `setup_scr_WidgetsDemo.c` 和 `events_init.c` 中当前实际出现的中文字符
- 还剩少量方块的原因，不是“字库缺字”，而是几处 `lv_label_create()` 直接创建的中文标签没有显式挂 `THERMAL_GUI_CN_FONT`
- 这些位置已经补上

最新补过的关键位置：

- `STM32CubeIDE/Appli/gui_guider/generated/setup_scr_WidgetsDemo.c`

本轮新增确认：

- 因为加入了 `全屏` 按钮，`全` 字曾显示为方块
- 已使用 `tools/gen_lvgl_subset_font.py` 重新生成：
  - `STM32CubeIDE/Appli/gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c`
- 当前字库已补入 `全`

注意：

- `events_init.c` 里曾经出现过中文乱码字符串，后续如果再改中文文案，务必保证文件编码是 **UTF-8**
- 当前工作区默认 `python` 指向 `D:\STEdgeAI\2.0\Utilities\windows\python.exe`，它不自带 `PIL/pip`
- 如果需要重跑 `tools/gen_lvgl_subset_font.py`，优先复用本机可用的 Python 3.10 + Pillow 环境，不要假设工作区内置 `python` 能直接跑


## 工程文件与构建注意事项

### 当前重要结论

这轮对话里明确确认过：

- **不要把最终方案建立在 `Debug/Release` 生成目录的手改上**
- 最终应该以工程级配置为主，例如：
  - `STM32CubeIDE/Appli/.project`


### 当前已做的工程级变更

已补工程资源登记：

- `STM32CubeIDE/Appli/.project`

其中加入了：

- `gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c`


### 当前风险

这里要特别注意：

- 我们中途为了快速验证，曾临时改过 `Debug` 下的生成清单
- 这些临时项后来已经按要求撤回
- **撤回后没有再做一次从“纯工程配置”出发的完整 clean build 验证**

因此当前最真实的状态是：

- 代码和字体文件都在
- `.project` 已更新
- 但 **是否仅靠 `.project` + IDE Refresh/Regenerate 就能让 Debug/Release 都重新纳入该字体文件，还需要你在 CubeIDE 里刷新并验证一次**

换句话说：

- 当前“功能逻辑和源文件”是完成的
- 当前“工程收口是否对 Debug/Release 都完全自洽”尚未最终闭环验证


## 当前常用构建命令

在 `STM32CubeIDE/Appli/Debug` 下执行：

- `mingw32-make -f makefile all -j2`

当前已知非阻塞 warning：

- `libirtemp.lib` 的 `wchar_t` ABI warning
- `LOAD segment with RWX permissions`
- 部分 `objdump debug_info` warning

这些 warning 当前不阻塞主要功能。


## 当前明确不再采用的方案

以下方案当前已主动弱化/废弃，不要默认往回加：

- MCU 本地 `Y14 -> 伪彩 -> LCD` 作为主显示链路
- 大量动态范围拉伸 / 本地伪彩 LUT 调优作为主方案
- `SourceHanSerif` 大面积中文字体全局挂载

原因：

- 模组原生图像显示效果更好
- 当前项目更适合“模组原生显示 + MCU 保留温度流”
- 大中文字体在当前板子/工程组合下不稳定


## 关键源码入口

### 主入口与系统初始化

- `Appli/Core/Src/main.c`
- `Appli/Core/Src/app_threadx.c`


### Tiny1C 应用层

- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_thermal_app.c`
- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_thermal_app.h`


### Tiny1C 驱动封装层

- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_vdcmd_app.c`
- `STM32CubeIDE/Appli/BSP/Tiny1C/tiny1c_vdcmd_app.h`
- `STM32CubeIDE/Appli/BSP/Tiny1C/VDCMD/`


### GUI / LVGL

- `STM32CubeIDE/Appli/gui_guider/generated/setup_scr_WidgetsDemo.c`
- `STM32CubeIDE/Appli/gui_guider/generated/events_init.c`
- `STM32CubeIDE/Appli/gui_guider/generated/gui_guider.c`
- `STM32CubeIDE/Appli/gui_guider/generated/gui_guider.h`
- `STM32CubeIDE/Appli/gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c`
- `tools/gen_lvgl_subset_font.py`


## 对后续 Agent 的建议

1. 先保住“LCD 原生图像显示 + UART 温度流”主链，不要先动显示方向。
2. 如果温度数值异常，优先检查 `Y14` 解包和组合帧拆分，不要先怪伪彩。
3. 如果显示效果问题，优先用模组侧 `pseudo_color_set()` 调整，不要先重做 MCU 本地伪彩。
4. 如果中文又出现方块，先检查：
   - 标签是否显式挂了 `THERMAL_GUI_CN_FONT`
   - `events_init.c` 是否被错误编码污染
   - 新文案是否超出了 `lv_font_thermal_cn_18` 子集
5. 如果需要扩充中文字库，优先继续用 `tools/gen_lvgl_subset_font.py` 增量生成，不要再回到 `SourceHanSerif` 全局挂载方案。
6. 如果全屏页性能不理想，优先怀疑：
   - `640x480` 全屏展开缓冲
   - 全屏页额外图像刷新
   - 不要优先把问题归因到 LVGL 本身
7. 如果最高温 / 最低温点表现异常，优先检查：
   - `extrema_query_thread` 是否正常运行
   - `g_extrema_cache_*` 是否有效更新
   - GUI 是否误走了本地扫描 fallback
8. 高低温点当前是“缓存线程 + GUI 读缓存”架构，不要轻易改回“GUI 线程里同步查模组”
9. 关于构建系统，优先修改工程级配置，不要把最终方案建立在 `Debug/Release` 生成物手改上。
10. 如果要确认 `Release`，最好在 CubeIDE 里对 `Appli` 工程做一次 `Refresh`，然后重新生成并验证对应配置。


## 当前未完成事项

- 还没有把 `libirtemp` 做成整帧像素级补偿
- 还需要继续上板验证环境参数对实际目标的影响
- 还需要继续上板验证：
  - 全屏页长期运行时的实际帧率
  - `extrema_query_thread` 200ms 周期是否是当前最合适折中
  - 全屏 `640x480` 是否需要改成更低分辨率版本
- 还需要最终确认：
  - 仅靠 `.project` 工程级登记后，`Debug/Release` 在 CubeIDE 刷新后是否都会自动纳入 `lv_font_thermal_cn_18.c`
- 如果后续切换分辨率版本，不要复用当前 `160x120` 常量，必须整体复核组合帧尺寸和 UART 协议

## 2026-05-16 Hardware Expansion Notes

Conversation summary for the next session:

- Software validation has been completed on the official STM32N6 development board.
- The next phase is custom hardware design: a core board plus a dedicated expansion board.
- The user's core board is assumed to already integrate the display interface, touch interface, XSPI, and EEPROM.

Confirmed hardware conclusions from this conversation:

- Touch control is not on the same bus as Tiny1-C control.
- Touch uses a software I2C on `PD14` and `PD4`.
- Tiny1-C control uses hardware `I2C4` on `PE13` and `PE14`.
- `I2C4` is shared in the current project by Tiny1-C control and onboard EEPROM.
- The UART used by the project is `USART1` on `PF12` and `PF13`.

Tiny1-C video capture path confirmed in the project:

- `PD7` -> `DCMIPP_D0`
- `PE6` -> `DCMIPP_D1`
- `PE0` -> `DCMIPP_D2`
- `PB9` -> `DCMIPP_D3`
- `PE8` -> `DCMIPP_D4`
- `PE5` -> `DCMIPP_D5`
- `PH9` -> `DCMIPP_D6`
- `PB7` -> `DCMIPP_D7`
- `PD0` -> `DCMIPP_HSYNC`
- `PB8` -> `DCMIPP_VSYNC`
- `PD5` -> `DCMIPP_PIXCLK`

Expansion-board implication:

- If LCD, touch, XSPI, and EEPROM are already on the core board, then the expansion board mainly needs Tiny1-C related signals.
- The must-consider signal groups are:
  - DCMIPP parallel capture bus
  - hardware `I2C4` control bus
  - `USART1` if serial logging/debug is still needed externally
  - power and ground

Important clarification:

- `DCMIPP` is a bus, not a single pin.
- `I2C4` and `USART1` are not substitutes for `DCMIPP`; each serves a different purpose.
- For PC-side serial logging/debug only, the minimum external header is usually `USART1 + GND`, not the full Tiny1-C bus.

Likely next task in the next conversation:

- Produce a final "must-route" vs "optional-route" connector checklist for the custom expansion board.

## 2026-05-19 Web Viewer Notes

- 新增 `thermal_web/` 作为独立 Web 上位机目录，基于 `FastAPI + WebSocket + Canvas`。
- 入口：`python thermal_web/app.py`，浏览器打开 `http://127.0.0.1:8000`。
- 默认自动选择第一个可用串口，也可以在网页里 `Refresh / Connect` 切换。
- 网页当前功能只保留和演示相关的最小闭环：
  - 热图实时显示
  - `Center / Max / Min`
  - 伪彩模式切换
  - 温标范围滑条
  - 截图
- Web 端直接复用 `tools/uart_temp14_parser.py` 的串口帧格式，解析的是原始 `temp14` payload。
- 图片横幅已移动到 `thermal_web/static/assets/thermal_ai_banner.png`，并放在左侧栏顶部作为品牌图。
- 同一个串口不能被别的程序占用；网页会自己打开/关闭串口，不需要再单独开串口监视器。

## 2026-05-27 Local Snapshot / FileX Notes

- Local snapshot storage is now integrated into the firmware with a usable end-to-end path.
- Final storage stack for this session:
  - `ThreadX`
  - `FileX`
  - `Appli/FileX/Target/fx_stm32_sd_driver_glue.c`
  - `STM32CubeIDE/Appli/BSP/SD_NAND/sd_nand.c`
  - `HAL_SD`
  - `SDMMC2`
- Important final decision:
  - Do not mount or scan the SD media during boot.
  - FileX is initialized lightly at startup, but SD mount is deferred until the user first saves a snapshot or opens the gallery.
  - This lazy-mount behavior fixed the previous issue where early SD/FileX activity destabilized display startup and could lead to black or corrupted screen behavior.

Current local storage behavior:

- Snapshot save path:
  - Main screen footer button saves the current thermal preview as a BMP.
  - Full-screen page also has a dedicated snapshot button.
- Gallery path:
  - The old `device info` card is no longer the storage entry.
  - A dedicated `图库` button is created on the `系统` tab and opens the gallery screen.
- File location and naming:
  - Directory: `/THERMAL`
  - Files: `THM00001.BMP`, `THM00002.BMP`, ...
- Gallery functions:
  - Open latest snapshot
  - Previous / next image
  - Delete current image

Snapshot rendering notes:

- Saved images are BMP RGB565 files based on the live LVGL thermal preview image plus overlay redraw.
- Overlay content currently includes:
  - center cross
  - center temperature
  - max temperature
  - min temperature
  - max/min markers
- The max/min temperature badges in saved images were changed from object-relative placement to a fixed safe layout so they are not clipped.
- The max/min marker labels in saved images are now forced to:
  - max marker: `高`
  - min marker: `低`
  This avoids depending on the live marker widget text.
- Gallery preview now applies image zoom-to-fit so full-screen snapshots can be viewed inside the gallery panel without cropping.

Font / Chinese glyph notes from this session:

- The tiny subset font `lv_font_thermal_cn_18.c` was regenerated again.
- Newly confirmed required glyphs include:
  - `图`
  - `库`
  - `拍`
  - `照`
  - `高`
  - `低`
- Custom buttons that show Chinese labels must explicitly use `lv_font_thermal_cn_18`.
- If new Chinese UI text appears as squares again:
  1. Check whether the widget explicitly uses `lv_font_thermal_cn_18`.
  2. Regenerate the subset font with `tools/gen_lvgl_subset_font.py`.
  3. Do not fall back to global SourceHanSerif mounting.
- Local font regeneration in this session used:
  - `C:\Users\26218\AppData\Local\Programs\Python\Python310\python.exe`
  - with Pillow available
  because the default `D:\STEdgeAI\2.0\Utilities\windows\python.exe` environment still lacks `PIL/pip`.

Files mainly touched by this session:

- `Appli/FileX/App/app_filex.c`
- `Appli/FileX/App/app_filex.h`
- `Appli/FileX/Target/fx_stm32_sd_driver_glue.c`
- `Appli/FileX/Target/fx_stm32_sd_driver.h`
- `Appli/AZURE_RTOS/App/app_azure_rtos.c`
- `Appli/AZURE_RTOS/App/app_azure_rtos_config.h`
- `Appli/Core/Src/app_threadx.c`
- `STM32CubeIDE/Appli/gui_guider/custom/custom.c`
- `STM32CubeIDE/Appli/gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c`
- `STM32CubeIDE/Appli/BSP/SD_NAND/`

Open follow-up items after this session:

- Continue long-run validation of repeated save / open / delete operations on the soldered 2GB SD NAND.
- If screenshot reliability changes again, first suspect:
  - early SD mount timing
  - glue-layer behavior in `fx_stm32_sd_driver_glue.c`
  - whether the BSP `sd_nand` path still returns success consistently
- If future agents touch the file system, preserve the lazy-mount strategy unless there is a very strong reason to change it.
