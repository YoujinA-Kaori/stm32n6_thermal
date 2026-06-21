/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
#include "app_filex.h"
#include "i2c.h"
#include "libirtemp.h"
#include "../../../Middlewares/ST/AI/Npu/ll_aton/ll_aton_runtime.h"
#include "thermal_ai_runtime.h"
#include "BQ27441/bq27441g1a.h"
#include "Tiny1C/tiny1c_thermal_app.h"
#include "usart.h"
#include "RGBLCD/rgblcd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct __attribute__((packed))
{
  uint16_t sync_word;
  uint16_t packet_type;
  uint32_t frame_counter;
  uint16_t frame_width;
  uint16_t frame_height;
  uint16_t payload_bytes;
  uint16_t center_temp14;
  uint16_t min_temp14;
  uint16_t max_temp14;
} app_threadx_uart_stream_header_t;

typedef struct
{
  uint8_t class_id;
  uint16_t confidence_permille;
  float score;
  float x_min;
  float y_min;
  float x_max;
  float y_max;
} app_threadx_ai_candidate_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CFG_THERMAL_THREAD_STACK_SIZE         4096U
#define CFG_THERMAL_THREAD_PRIORITY           13U
#define CFG_GUI_THREAD_STACK_SIZE             8192U
#define CFG_GUI_THREAD_PRIORITY               10U
#define CFG_UART_STREAM_THREAD_STACK_SIZE     4096U
#define CFG_UART_STREAM_THREAD_PRIORITY       15U
#define CFG_UART_STREAM_SOURCE_WIDTH          160U
#define CFG_UART_STREAM_SOURCE_HEIGHT         120U
#define CFG_UART_STREAM_DOWNSAMPLE_STEP_X     1U
#define CFG_UART_STREAM_DOWNSAMPLE_STEP_Y     1U
#define CFG_UART_STREAM_FRAME_WIDTH           (CFG_UART_STREAM_SOURCE_WIDTH / CFG_UART_STREAM_DOWNSAMPLE_STEP_X)
#define CFG_UART_STREAM_FRAME_HEIGHT          (CFG_UART_STREAM_SOURCE_HEIGHT / CFG_UART_STREAM_DOWNSAMPLE_STEP_Y)
#define CFG_UART_STREAM_PAYLOAD_BYTES         (CFG_UART_STREAM_FRAME_WIDTH * CFG_UART_STREAM_FRAME_HEIGHT * sizeof(uint16_t))
#define CFG_UART_STREAM_SYNC_WORD             0x55AAU
#define CFG_UART_STREAM_PACKET_TYPE_TEMP14    0x0001U
#define CFG_UART_STREAM_PERIOD_MS             333U
#define CFG_UART_STREAM_PERIOD_TICKS          (((CFG_UART_STREAM_PERIOD_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)
#define CFG_THERMAL_AI_MIN_INTERVAL_MS        500U
#define CFG_THERMAL_AI_MIN_INTERVAL_TICKS     (((CFG_THERMAL_AI_MIN_INTERVAL_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)
#define CFG_THERMAL_AI_DIAG_STAGE_SETTLE_MS   150U
#define CFG_THERMAL_AI_DIAG_STAGE_SETTLE_TICKS (((CFG_THERMAL_AI_DIAG_STAGE_SETTLE_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)

#define CFG_THERMAL_AI_ENABLE                 1U
#define CFG_THERMAL_AI_USE_REFERENCE_INPUT    0U
#define CFG_UART_COMMAND_THREAD_STACK_SIZE    4096U
#define CFG_UART_COMMAND_THREAD_PRIORITY      16U
#define CFG_UART_COMMAND_RX_TIMEOUT_MS        20U
#define CFG_UART_FILE_WEB_TIMEOUT_MS          4000U
#define CFG_UART_COMMAND_LINE_MAX             96U
#define CFG_UART_COMMAND_TX_LINE_MAX          768U
#define CFG_UART_FILE_BASE64_CHUNK_BYTES      384U
#define CFG_UART_FILE_BASE64_CHUNK_TEXT       (((CFG_UART_FILE_BASE64_CHUNK_BYTES + 2U) / 3U) * 4U)
#define CFG_UART_FILE_MAX_LISTED_SNAPSHOTS    32U
#define CFG_UART_FILE_NAME_STRIDE             CFG_APP_FILEX_SNAPSHOT_NAME_LEN
#define CFG_UART_FILE_MAX_PIXELS              (CFG_GUI_FULLSCREEN_WIDTH * CFG_GUI_FULLSCREEN_HEIGHT)
#define CFG_UART_FILE_WEB_TIMEOUT_TICKS       (((CFG_UART_FILE_WEB_TIMEOUT_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)
#define CFG_LIBIRTEMP_TEST_EMISSIVITY         0.95F
#define CFG_LIBIRTEMP_TEST_TAU_Q14            16384U
#define CFG_LIBIRTEMP_TEST_AMBIENT_TEMP_C     25.0F
#define CFG_GUI_FULLSCREEN_WIDTH              640U
#define CFG_GUI_FULLSCREEN_HEIGHT             480U
#define CFG_GUI_OVERLAY_UPDATE_PERIOD_MS      120U
#define CFG_EXTREMA_QUERY_THREAD_STACK_SIZE   2048U
#define CFG_EXTREMA_QUERY_THREAD_PRIORITY     18U
#define CFG_EXTREMA_QUERY_PERIOD_MS           80U
#define CFG_EXTREMA_QUERY_PERIOD_TICKS        (((CFG_EXTREMA_QUERY_PERIOD_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)
#define CFG_BATTERY_THREAD_STACK_SIZE         2048U
#define CFG_BATTERY_THREAD_PRIORITY           19U
#define CFG_BATTERY_POLL_PERIOD_MS            1000U
#define CFG_BATTERY_POLL_PERIOD_TICKS         (((CFG_BATTERY_POLL_PERIOD_MS * TX_TIMER_TICKS_PER_SECOND) + 999U) / 1000U)
#define CFG_BATTERY_I2C_TIMEOUT_MS            100U
#define CFG_THERMAL_AI_INPUT_WIDTH            160U
#define CFG_THERMAL_AI_INPUT_HEIGHT           120U
#define CFG_THERMAL_AI_INPUT_PIXELS           (CFG_THERMAL_AI_INPUT_WIDTH * CFG_THERMAL_AI_INPUT_HEIGHT)
#define CFG_THERMAL_AI_GRID_WIDTH             20U
#define CFG_THERMAL_AI_GRID_HEIGHT            15U
#define CFG_THERMAL_AI_OUTPUT_CHANNELS        8U
#define CFG_THERMAL_AI_OUTPUT_BYTES           (CFG_THERMAL_AI_GRID_WIDTH * CFG_THERMAL_AI_GRID_HEIGHT * CFG_THERMAL_AI_OUTPUT_CHANNELS)
#define CFG_THERMAL_AI_HISTOGRAM_BINS         16384U
#define CFG_THERMAL_AI_MAX_CANDIDATES         (CFG_THERMAL_AI_GRID_WIDTH * CFG_THERMAL_AI_GRID_HEIGHT)
#define CFG_THERMAL_AI_BACKGROUND_PERCENT_NUM 50U
#define CFG_THERMAL_AI_BACKGROUND_PERCENT_DEN 100U
#define CFG_THERMAL_AI_DELTA_TEMP14_MIN       (-128.0f)
#define CFG_THERMAL_AI_DELTA_TEMP14_MAX       (960.0f)
#define CFG_THERMAL_AI_DELTA_TEMP14_SPAN      (CFG_THERMAL_AI_DELTA_TEMP14_MAX - CFG_THERMAL_AI_DELTA_TEMP14_MIN)
#define CFG_THERMAL_AI_OBJECTNESS_THRESHOLD   0.45f
#define CFG_THERMAL_AI_CLASS_THRESHOLD        0.50f
#define CFG_THERMAL_AI_SCORE_THRESHOLD        0.30f
#define CFG_THERMAL_AI_NMS_IOU_THRESHOLD      0.15f
#define CFG_THERMAL_AI_MAX_DETECTIONS         2U
#define CFG_THERMAL_AI_PERSON_MIN_WIDTH_PX    8.0f
#define CFG_THERMAL_AI_PERSON_MIN_HEIGHT_PX   18.0f
#define CFG_THERMAL_AI_PERSON_TOP_MARGIN_PX   4.0f
#define CFG_THERMAL_AI_PERSON_SIDE_MARGIN_PX  6.0f
#define CFG_THERMAL_AI_PERSON_TOP_BAND_MAX_H  56.0f
#define CFG_THERMAL_AI_PERSON_CORNER_MAX_W    52.0f
#define CFG_THERMAL_AI_PERSON_CORNER_MAX_H    64.0f
#define CFG_THERMAL_AI_PERSON_EDGE_STRIP_MAX_W 42.0f
#define CFG_THERMAL_AI_PERSON_EDGE_STRIP_MIN_H 58.0f

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD g_thermal_thread;
static TX_THREAD g_gui_thread;
static TX_THREAD g_uart_stream_thread;
static TX_THREAD g_uart_command_thread;
static TX_THREAD g_extrema_query_thread;
static TX_THREAD g_battery_thread;
static TX_MUTEX g_thermal_ai_mutex;
static ULONG g_thermal_thread_stack[CFG_THERMAL_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_gui_thread_stack[CFG_GUI_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_uart_stream_thread_stack[CFG_UART_STREAM_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_uart_command_thread_stack[CFG_UART_COMMAND_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_extrema_query_thread_stack[CFG_EXTREMA_QUERY_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_battery_thread_stack[CFG_BATTERY_THREAD_STACK_SIZE / sizeof(ULONG)];
static volatile float g_libirtemp_probe_sink_c = 0.0f;
static volatile uint8_t g_uart_stream_tx_busy = 0U;
static volatile uint32_t g_uart_file_hold_mask = 0U;
static volatile ULONG g_uart_web_file_hold_expire_tick = 0U;
static volatile uint8_t g_tiny1c_app_started = 0U;
static volatile uint8_t g_extrema_cache_valid = 0U;
static volatile uint16_t g_extrema_cache_min_temp14 = 0U;
static volatile uint16_t g_extrema_cache_max_temp14 = 0U;
static volatile uint16_t g_extrema_cache_min_temp_x = 0U;
static volatile uint16_t g_extrema_cache_min_temp_y = 0U;
static volatile uint16_t g_extrema_cache_max_temp_x = 0U;
static volatile uint16_t g_extrema_cache_max_temp_y = 0U;
static volatile uint8_t g_battery_cache_valid = 0U;
static volatile uint8_t g_battery_cache_percent = 0U;
static volatile uint8_t g_battery_cache_charge_state = (uint8_t)BQ27441_CHARGE_STATUS_SLEEP;
static BQ27441_Device_t g_battery_device;
static uint8_t g_uart_stream_tx_buffer[sizeof(app_threadx_uart_stream_header_t) + CFG_UART_STREAM_PAYLOAD_BYTES]
  __attribute__((aligned(32)));
static uint8_t g_uart_command_rx_line[CFG_UART_COMMAND_LINE_MAX];
static uint8_t g_uart_command_tx_line[CFG_UART_COMMAND_TX_LINE_MAX];
static uint8_t g_uart_file_chunk_base64[CFG_UART_FILE_BASE64_CHUNK_TEXT + 4U];
static CHAR g_uart_file_snapshot_names[CFG_UART_FILE_MAX_LISTED_SNAPSHOTS][CFG_UART_FILE_NAME_STRIDE];
static lv_img_dsc_t g_gui_preview_img_dsc;
static lv_img_dsc_t g_gui_fullscreen_img_dsc;
static uint16_t g_gui_fullscreen_rgb565_frame[CFG_GUI_FULLSCREEN_WIDTH * CFG_GUI_FULLSCREEN_HEIGHT]
  __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t g_uart_file_rgb565_frame[CFG_UART_FILE_MAX_PIXELS]
  __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t g_thermal_ai_oriented_temp14[CFG_THERMAL_AI_INPUT_PIXELS];
static uint16_t g_thermal_ai_histogram[CFG_THERMAL_AI_HISTOGRAM_BINS];
static app_threadx_ai_candidate_t g_thermal_ai_candidates[CFG_THERMAL_AI_MAX_CANDIDATES];
static app_threadx_ai_candidate_t g_thermal_ai_kept_candidates[CFG_THERMAL_AI_MAX_CANDIDATES];
static thermal_ai_runtime_t g_thermal_ai_runtime;
static uint8_t g_thermal_ai_overlay_ready = 0U;
static uint8_t g_thermal_ai_model_ready = 0U;
static uint32_t g_thermal_ai_last_inference_ms = 0U;
static volatile uint8_t g_thermal_ai_iac_fault_latched = 0U;
static volatile uint32_t g_thermal_ai_iac_fault_count = 0U;
static volatile uint32_t g_thermal_ai_iac_last_periph_id = 0U;
static volatile uint8_t g_thermal_ai_diag_stage = 0U;
static volatile uint32_t g_thermal_ai_diag_run_count = 0U;
static volatile uint16_t g_thermal_ai_diag_raw_candidate_count = 0U;
static volatile uint16_t g_thermal_ai_diag_postfilter_candidate_count = 0U;
static volatile uint16_t g_thermal_ai_diag_max_score_permille = 0U;
static volatile uint16_t g_thermal_ai_diag_max_objectness_permille = 0U;
static volatile uint8_t g_thermal_ai_diag_output_layout = 0U;
static volatile uint8_t g_thermal_ai_diag_weights_ok = 0U;
static volatile uint8_t g_thermal_ai_diag_output_byte0 = 0U;
static const LL_Buffer_InfoTypeDef *g_thermal_ai_input_info = NULL;
static const LL_Buffer_InfoTypeDef *g_thermal_ai_output_info = NULL;
static int8_t *g_thermal_ai_input_buffer = NULL;
static int8_t *g_thermal_ai_output_buffer = NULL;
static lv_obj_t *g_thermal_ai_preview_boxes[CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS];
static lv_obj_t *g_thermal_ai_preview_labels[CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS];
static lv_obj_t *g_thermal_ai_fullscreen_boxes[CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS];
static lv_obj_t *g_thermal_ai_fullscreen_labels[CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS];

#if (CFG_THERMAL_AI_USE_REFERENCE_INPUT != 0U)
extern const int8_t g_thermal_ai_reference_input[];
extern const int8_t g_thermal_ai_reference_input_end[];

__asm__(
    ".section .rodata\n"
    ".global g_thermal_ai_reference_input\n"
    "g_thermal_ai_reference_input:\n"
    ".incbin \"D:/PracticeProject/Stm32/stm32n6_thermal/thermal_ai/artifacts/reports/cubeai_inputs/best_model_int8_person_frame_00001181/input_model_dtype_nhwc.int8.raw\"\n"
    ".global g_thermal_ai_reference_input_end\n"
    "g_thermal_ai_reference_input_end:\n"
    ".balign 4\n");
#endif

LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(thermal);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID app_threadx_thermal_thread_entry(ULONG thread_input);
static VOID app_threadx_gui_thread_entry(ULONG thread_input);
static VOID app_threadx_uart_stream_thread_entry(ULONG thread_input);
static VOID app_threadx_uart_command_thread_entry(ULONG thread_input);
static VOID app_threadx_extrema_query_thread_entry(ULONG thread_input);
static VOID app_threadx_battery_thread_entry(ULONG thread_input);
static void app_threadx_uart_process_command_line(const uint8_t *command_line_ptr);
static UINT app_threadx_uart_send_text(const char *text_ptr);
static UINT app_threadx_uart_send_text_fmt(const char *format_ptr, ...);
static void app_threadx_uart_wait_for_tx_idle(void);
static uint8_t app_threadx_uart_file_mode_active_internal(void);
static void app_threadx_uart_file_hold_set(uint32_t hold_mask);
static void app_threadx_uart_file_hold_clear(uint32_t hold_mask);
static UINT app_threadx_uart_send_snapshot_list(void);
static UINT app_threadx_uart_send_snapshot_latest(void);
static UINT app_threadx_uart_send_snapshot_named(const CHAR *file_name_ptr);
static uint32_t app_threadx_base64_encode(const uint8_t *input_ptr,
                                          uint32_t input_size,
                                          char *output_ptr,
                                          uint32_t output_capacity);
static uint32_t app_threadx_uart_stream_build_packet(uint8_t *tx_buffer, const uint16_t *temp14_frame, uint32_t frame_counter);
static void app_threadx_uart_stream_clean_dcache(void *buffer_addr, uint32_t buffer_size);
static int32_t app_threadx_temp14_to_centi_celsius(uint16_t temp14_value);
static int32_t app_threadx_temp14_to_compensated_centi_celsius(uint16_t temp14_value);
static int32_t app_threadx_centi_celsius_to_unit_centi_celsius(int32_t temp_centi_c, uint8_t unit_celsius);
static int32_t app_threadx_float_to_centi_celsius(float temp_c);
static uint16_t app_threadx_temp14_get_preview_oriented_sample(const uint16_t *temp14_frame,
                                                               uint16_t frame_width,
                                                               uint16_t frame_height,
                                                               uint16_t preview_x,
                                                               uint16_t preview_y);
static IrPoint_t app_threadx_build_center_point(uint16_t frame_width, uint16_t frame_height);
static uint16_t app_threadx_rgb565_average4(uint16_t c00,
                                            uint16_t c10,
                                            uint16_t c01,
                                            uint16_t c11);
static void app_threadx_gui_set_badge_text(lv_obj_t *badge, const char *text);
static void app_threadx_gui_update_preview(lv_ui *ui);
static void app_threadx_gui_update_fullscreen_image(void);
static void app_threadx_gui_format_battery_text(char *buffer, uint32_t buffer_size);
static void app_threadx_gui_format_ai_status_text(char *buffer, uint32_t buffer_size);
static void app_threadx_thermal_ai_set_diag_stage(uint8_t stage, uint8_t allow_settle);
static void app_threadx_gui_update_preview_layer(lv_obj_t *preview_img,
                                                 lv_obj_t *preview_center_temp,
                                                 lv_obj_t *preview_max_temp,
                                                 lv_obj_t *preview_min_temp,
                                                 lv_obj_t *preview_max_marker,
                                                 lv_obj_t *preview_min_marker,
                                                 uint16_t preview_width,
                                                 uint16_t preview_height,
                                                 uint16_t frame_width,
                                                 uint16_t frame_height,
                                                 uint8_t mirror_enable,
                                                 uint8_t flip_enable,
                                                 uint8_t unit_celsius,
                                                 uint16_t min_temp14,
                                                 uint16_t max_temp14,
                                                 int32_t center_temp_centi_c,
                                                 uint16_t min_temp_x,
                                                 uint16_t min_temp_y,
                                                 uint16_t max_temp_x,
                                                 uint16_t max_temp_y);
static uint8_t app_threadx_thermal_ai_prepare_io(void);
static void app_threadx_thermal_ai_run_inference(const uint16_t *temp14_frame, uint32_t frame_counter);
static uint8_t app_threadx_thermal_ai_load_reference_input(uint16_t *background_temp14_ptr,
                                                           uint16_t *max_temp14_ptr);
static uint8_t app_threadx_thermal_ai_probe_weight_signature(void);
static uint16_t app_threadx_thermal_ai_percentile_from_histogram(uint32_t pixel_count);
static void app_threadx_thermal_ai_build_input(const uint16_t *temp14_frame,
                                               uint16_t *background_temp14_ptr,
                                               uint16_t *max_temp14_ptr);
static int8_t app_threadx_thermal_ai_read_output_q7(uint32_t grid_y,
                                                    uint32_t grid_x,
                                                    uint32_t channel_index,
                                                    uint8_t channel_planar_layout);
static uint32_t app_threadx_thermal_ai_collect_candidates(float output_scale,
                                                          int16_t output_zero_point,
                                                          uint8_t channel_planar_layout,
                                                          uint32_t *raw_candidate_count_ptr,
                                                          uint16_t *max_score_permille_ptr,
                                                          uint16_t *max_objectness_permille_ptr);
static void app_threadx_thermal_ai_decode_output(thermal_ai_result_t *result_ptr);
static float app_threadx_thermal_ai_sigmoid(float value);
static float app_threadx_thermal_ai_candidate_iou(const app_threadx_ai_candidate_t *lhs_ptr,
                                                  const app_threadx_ai_candidate_t *rhs_ptr);
static uint8_t app_threadx_thermal_ai_person_filter(const app_threadx_ai_candidate_t *candidate_ptr);
static uint8_t app_threadx_thermal_ai_class_index_to_runtime_id(uint32_t class_index);
static lv_color_t app_threadx_thermal_ai_color_for_class(uint8_t class_id);
static void app_threadx_dcache_clean_by_addr(void *buffer_addr, uint32_t buffer_size);
static void app_threadx_dcache_invalidate_by_addr(void *buffer_addr, uint32_t buffer_size);
static void app_threadx_gui_init_ai_overlays(void);
static void app_threadx_gui_init_ai_overlay_set(lv_obj_t *parent_img,
                                                lv_obj_t **box_array,
                                                lv_obj_t **label_array);
static void app_threadx_gui_update_ai_overlay_set(lv_obj_t **box_array,
                                                  lv_obj_t **label_array,
                                                  uint16_t preview_width,
                                                  uint16_t preview_height,
                                                  const thermal_ai_result_t *result_ptr);
static void app_threadx_gui_build_fullscreen_frame(const uint16_t *source_frame,
                                                   uint16_t *dest_frame,
                                                   uint16_t source_width,
                                                   uint16_t source_height);
static void app_threadx_gui_format_temp_text(char *buffer, uint32_t buffer_size, const char *prefix, int32_t temp_centi_c, uint8_t unit_celsius);

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
  TX_PARAMETER_NOT_USED(memory_ptr);

  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  ret = app_i2c4_bus_mutex_init();
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_mutex_create(&g_thermal_ai_mutex, "thermal_ai_mutex", TX_NO_INHERIT);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  thermal_ai_runtime_init(&g_thermal_ai_runtime, 2U, 3000);

  ret = tx_thread_create(&g_thermal_thread,
                         "thermal_thread",
                         app_threadx_thermal_thread_entry,
                         0U,
                         g_thermal_thread_stack,
                         sizeof(g_thermal_thread_stack),
                         CFG_THERMAL_THREAD_PRIORITY,
                         CFG_THERMAL_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_thread_create(&g_gui_thread,
                         "gui_thread",
                         app_threadx_gui_thread_entry,
                         0U,
                         g_gui_thread_stack,
                         sizeof(g_gui_thread_stack),
                         CFG_GUI_THREAD_PRIORITY,
                         CFG_GUI_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_thread_create(&g_uart_stream_thread,
                         "uart_stream_thread",
                         app_threadx_uart_stream_thread_entry,
                         0U,
                         g_uart_stream_thread_stack,
                         sizeof(g_uart_stream_thread_stack),
                         CFG_UART_STREAM_THREAD_PRIORITY,
                         CFG_UART_STREAM_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_thread_create(&g_uart_command_thread,
                         "uart_command_thread",
                         app_threadx_uart_command_thread_entry,
                         0U,
                         g_uart_command_thread_stack,
                         sizeof(g_uart_command_thread_stack),
                         CFG_UART_COMMAND_THREAD_PRIORITY,
                         CFG_UART_COMMAND_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_thread_create(&g_extrema_query_thread,
                         "extrema_query_thread",
                         app_threadx_extrema_query_thread_entry,
                         0U,
                         g_extrema_query_thread_stack,
                         sizeof(g_extrema_query_thread_stack),
                         CFG_EXTREMA_QUERY_THREAD_PRIORITY,
                         CFG_EXTREMA_QUERY_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  ret = tx_thread_create(&g_battery_thread,
                         "battery_thread",
                         app_threadx_battery_thread_entry,
                         0U,
                         g_battery_thread_stack,
                         sizeof(g_battery_thread_stack),
                         CFG_BATTERY_THREAD_PRIORITY,
                         CFG_BATTERY_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  Request UART file mode for one owner.
  * @param  hold_mask Bit mask describing the requester.
  * @retval None
  */
void app_uart_request_file_mode(uint32_t hold_mask)
{
  if (hold_mask != 0U)
  {
    g_uart_file_hold_mask |= hold_mask;
    if ((hold_mask & APP_UART_FILE_HOLD_WEB) != 0U)
    {
      g_uart_web_file_hold_expire_tick = tx_time_get() + ((CFG_UART_FILE_WEB_TIMEOUT_TICKS > 0U) ? CFG_UART_FILE_WEB_TIMEOUT_TICKS : 1U);
    }
  }
}

/**
  * @brief  Release UART file mode for one owner.
  * @param  hold_mask Bit mask describing the requester.
  * @retval None
  */
void app_uart_release_file_mode(uint32_t hold_mask)
{
  if (hold_mask != 0U)
  {
    g_uart_file_hold_mask &= ~hold_mask;
    if ((hold_mask & APP_UART_FILE_HOLD_WEB) != 0U)
    {
      g_uart_web_file_hold_expire_tick = 0U;
    }
  }
}

/**
  * @brief  Report whether UART file mode is currently active.
  * @retval uint8_t 1 when file mode is active, otherwise 0.
  */
uint8_t app_uart_is_file_mode_active(void)
{
  return app_threadx_uart_file_mode_active_internal();
}

/**
  * @brief  Apply libirtemp environment compensation to a temperature sample.
  * @param  emissivity Emissivity in range [0, 1].
  * @param  tau_q14 Atmospheric transmittance in Q14 format.
  * @param  ambient_temp_c Ambient temperature in degrees Celsius.
  * @param  source_temp_c Source temperature in degrees Celsius.
  * @param  corrected_temp_c Output compensated temperature pointer.
  * @retval 0 on success, -1 on error.
  */
static int32_t app_threadx_libirtemp_temp_correct(float emissivity,
                                                  uint16_t tau_q14,
                                                  float ambient_temp_c,
                                                  float source_temp_c,
                                                  float *corrected_temp_c)
{
  irtemp_error_t ret;

  if (corrected_temp_c == NULL)
  {
    return -1;
  }

  ret = temp_correct(emissivity, tau_q14, ambient_temp_c, source_temp_c, corrected_temp_c);
  if (ret != IRTEMP_SUCCESS)
  {
    return -1;
  }

  return 0;
}

/**
  * @brief  Convert one temp14 value to centi-degrees Celsius.
  * @param  temp14_value Raw temperature value in 1/16 K units.
  * @retval Temperature in 0.01 degrees Celsius.
  */
static int32_t app_threadx_temp14_to_centi_celsius(uint16_t temp14_value)
{
  int32_t scaled_temp;

  scaled_temp = ((int32_t)temp14_value * 100 + 8) / 16;
  scaled_temp -= 27315;

  return scaled_temp;
}

/**
  * @brief  Convert one temp14 value to compensated centi-degrees Celsius.
  * @param  temp14_value Raw temperature value in 1/16 K units.
  * @retval Compensated temperature in 0.01 degrees Celsius.
  */
static int32_t app_threadx_temp14_to_compensated_centi_celsius(uint16_t temp14_value)
{
  int32_t raw_temp_centi_c;
  float corrected_temp_c;

  raw_temp_centi_c = app_threadx_temp14_to_centi_celsius(temp14_value);
  corrected_temp_c = (float)raw_temp_centi_c / 100.0f;

  if (app_threadx_libirtemp_temp_correct(CFG_LIBIRTEMP_TEST_EMISSIVITY,
                                         CFG_LIBIRTEMP_TEST_TAU_Q14,
                                         CFG_LIBIRTEMP_TEST_AMBIENT_TEMP_C,
                                         corrected_temp_c,
                                         &corrected_temp_c) == 0)
  {
    return app_threadx_float_to_centi_celsius(corrected_temp_c);
  }

  return raw_temp_centi_c;
}

/**
  * @brief  Convert centi-degrees Celsius into the selected unit.
  * @param  temp_centi_c Temperature in 0.01 degrees Celsius.
  * @param  unit_celsius Non-zero for Celsius, zero for Fahrenheit.
  * @retval Temperature in the selected unit, still scaled by 100.
  */
static int32_t app_threadx_centi_celsius_to_unit_centi_celsius(int32_t temp_centi_c, uint8_t unit_celsius)
{
  if (unit_celsius != 0U)
  {
    return temp_centi_c;
  }

  return ((temp_centi_c * 9) / 5) + 3200;
}

/**
  * @brief  Convert a float temperature to centi-degrees Celsius.
  * @param  temp_c Temperature in degrees Celsius.
  * @retval Temperature in 0.01 degrees Celsius.
  */
static int32_t app_threadx_float_to_centi_celsius(float temp_c)
{
  float scaled_temp;

  scaled_temp = temp_c * 100.0f;
  if (scaled_temp >= 0.0f)
  {
    scaled_temp += 0.5f;
  }
  else
  {
    scaled_temp -= 0.5f;
  }

  return (int32_t)scaled_temp;
}

/**
  * @brief  Read one temp14 sample in the current preview/UART orientation.
  * @param  temp14_frame Pointer to the raw temp14 frame.
  * @param  frame_width Raw frame width.
  * @param  frame_height Raw frame height.
  * @param  preview_x X coordinate in preview-oriented space.
  * @param  preview_y Y coordinate in preview-oriented space.
  * @retval uint16_t Preview-oriented temp14 sample.
  */
static uint16_t app_threadx_temp14_get_preview_oriented_sample(const uint16_t *temp14_frame,
                                                               uint16_t frame_width,
                                                               uint16_t frame_height,
                                                               uint16_t preview_x,
                                                               uint16_t preview_y)
{
  uint16_t source_x = preview_x;
  uint16_t source_y = preview_y;

  if ((temp14_frame == NULL) || (frame_width == 0U) || (frame_height == 0U))
  {
    return 0U;
  }

  if (source_x >= frame_width)
  {
    source_x = (uint16_t)(frame_width - 1U);
  }

  if (source_y >= frame_height)
  {
    source_y = (uint16_t)(frame_height - 1U);
  }

  /* Mirror/flip is self-inverse, so the same transform maps preview coords back to raw source coords. */
  tiny1c_thermal_app_transform_frame_point(source_x, source_y, &source_x, &source_y);
  return temp14_frame[((uint32_t)source_y * (uint32_t)frame_width) + (uint32_t)source_x];
}

/**
  * @brief  Build the center point using Tiny1-C 1-based coordinates.
  * @param  frame_width Frame width in pixels.
  * @param  frame_height Frame height in pixels.
  * @retval IrPoint_t Center point for module point-temperature queries.
  */
static IrPoint_t app_threadx_build_center_point(uint16_t frame_width, uint16_t frame_height)
{
  IrPoint_t center_point;

  center_point.x = (uint16_t)(frame_width / 2U + 1U);
  center_point.y = (uint16_t)(frame_height / 2U + 1U);

  return center_point;
}

/**
  * @brief  Average four RGB565 pixels using a light 2x2 box filter.
  * @param  c00 Top-left RGB565 pixel.
  * @param  c10 Top-right RGB565 pixel.
  * @param  c01 Bottom-left RGB565 pixel.
  * @param  c11 Bottom-right RGB565 pixel.
  * @retval Averaged RGB565 pixel.
  */
static uint16_t app_threadx_rgb565_average4(uint16_t c00,
                                            uint16_t c10,
                                            uint16_t c01,
                                            uint16_t c11)
{
  uint32_t red;
  uint32_t green;
  uint32_t blue;

  red = ((uint32_t)((c00 >> 11) & 0x1FU) +
         (uint32_t)((c10 >> 11) & 0x1FU) +
         (uint32_t)((c01 >> 11) & 0x1FU) +
         (uint32_t)((c11 >> 11) & 0x1FU) + 2U) >> 2;

  green = ((uint32_t)((c00 >> 5) & 0x3FU) +
           (uint32_t)((c10 >> 5) & 0x3FU) +
           (uint32_t)((c01 >> 5) & 0x3FU) +
           (uint32_t)((c11 >> 5) & 0x3FU) + 2U) >> 2;

  blue = ((uint32_t)(c00 & 0x1FU) +
          (uint32_t)(c10 & 0x1FU) +
          (uint32_t)(c01 & 0x1FU) +
          (uint32_t)(c11 & 0x1FU) + 2U) >> 2;

  return (uint16_t)(((red & 0x1FU) << 11) |
                    ((green & 0x3FU) << 5) |
                    (blue & 0x1FU));
}

/**
  * @brief  Set the text of a status badge label.
  * @param  badge Badge container object.
  * @param  text New badge text.
  * @retval None
  */
static void app_threadx_gui_set_badge_text(lv_obj_t *badge, const char *text)
{
  lv_obj_t *label;

  if ((badge == NULL) || (text == NULL))
  {
    return;
  }

  label = lv_obj_get_child(badge, 0);
  if (label != NULL)
  {
    lv_label_set_text(label, text);
  }
}

/**
  * @brief  Format a temperature badge text string.
  * @param  buffer Output buffer.
  * @param  buffer_size Output buffer size.
  * @param  prefix Text prefix.
  * @param  temp_centi_c Temperature in centi-degrees Celsius.
  * @retval None
  */
static void app_threadx_gui_format_temp_text(char *buffer, uint32_t buffer_size, const char *prefix, int32_t temp_centi_c, uint8_t unit_celsius)
{
  int32_t abs_temp_centi;
  int32_t temp_int;
  int32_t temp_frac;
  int32_t display_temp_centi;
  char unit_char;

  if ((buffer == NULL) || (buffer_size == 0U) || (prefix == NULL))
  {
    return;
  }

  display_temp_centi = app_threadx_centi_celsius_to_unit_centi_celsius(temp_centi_c, unit_celsius);
  unit_char = (unit_celsius != 0U) ? 'C' : 'F';

  abs_temp_centi = (display_temp_centi < 0) ? -display_temp_centi : display_temp_centi;
  temp_int = abs_temp_centi / 100;
  temp_frac = abs_temp_centi % 100;
  (void)snprintf(buffer,
                 buffer_size,
                 "%s%s%" PRId32 ".%02" PRId32 " %c",
                 prefix,
                 (display_temp_centi < 0) ? "-" : "",
                 temp_int,
                 temp_frac,
                 unit_char);
}

/**
  * @brief  Format the battery badge text from the latest cached battery state.
  * @param  buffer Output buffer.
  * @param  buffer_size Output buffer size.
  * @retval None
  */
static void app_threadx_gui_format_battery_text(char *buffer, uint32_t buffer_size)
{
  uint8_t battery_percent;
  uint8_t battery_valid;
  uint8_t battery_charge_state;
  const char *state_prefix;

  if ((buffer == NULL) || (buffer_size == 0U))
  {
    return;
  }

  battery_valid = g_battery_cache_valid;
  battery_percent = g_battery_cache_percent;
  battery_charge_state = g_battery_cache_charge_state;
  if (battery_valid == 0U)
  {
    (void)snprintf(buffer, buffer_size, "BAT --%%");
    return;
  }

  switch ((BQ27441_ChargeStatusTypeDef)battery_charge_state)
  {
    case BQ27441_CHARGE_STATUS_CHARGING:
      state_prefix = "CHG";
      break;

    case BQ27441_CHARGE_STATUS_DISCHARGING:
      state_prefix = "DSG";
      break;

    case BQ27441_CHARGE_STATUS_SLEEP:
    default:
      state_prefix = "BAT";
      break;
  }

  (void)snprintf(buffer, buffer_size, "%s %u%%", state_prefix, (unsigned)battery_percent);
}

/**
  * @brief  Format one short AI diagnostic string for the top status badge.
  * @param  buffer Output buffer pointer.
  * @param  buffer_size Output buffer size.
  * @retval None
  */
static void app_threadx_gui_format_ai_status_text(char *buffer, uint32_t buffer_size)
{
  const char *iac_name;
  uint32_t inference_ms;
  thermal_ai_result_t ai_result_snapshot;
  uint8_t ai_result_valid = 0U;
  uint16_t max_objectness_permille = 0U;
  uint8_t output_byte0 = 0U;
  char weight_char = 'X';
  char layout_char = 'I';

  if ((buffer == NULL) || (buffer_size == 0U))
  {
    return;
  }

  if (CFG_THERMAL_AI_ENABLE == 0U)
  {
    (void)snprintf(buffer, buffer_size, "AI OFF");
    return;
  }

  if (g_thermal_ai_iac_fault_latched != 0U)
  {
    switch (g_thermal_ai_iac_last_periph_id)
    {
      case RIF_RISC_PERIPH_INDEX_NPU:
        iac_name = "NPU";
        break;

      case RIF_RISC_PERIPH_INDEX_XSPI2:
        iac_name = "X2";
        break;

      case RIF_RISC_PERIPH_INDEX_XSPIM:
        iac_name = "XM";
        break;

      default:
        iac_name = "UNK";
        break;
    }

    (void)snprintf(buffer, buffer_size, "IAC %s %lu",
                   iac_name,
                   (unsigned long)g_thermal_ai_iac_fault_count);
    return;
  }

  switch (g_thermal_ai_diag_stage)
  {
    case 1U:
      (void)snprintf(buffer, buffer_size, "AI GO %lu", (unsigned long)g_thermal_ai_diag_run_count);
      return;

    case 2U:
      (void)snprintf(buffer, buffer_size, "AI ION");
      return;

    case 3U:
      (void)snprintf(buffer, buffer_size, "AI IOB");
      return;

    case 4U:
      (void)snprintf(buffer, buffer_size, "AI IOL");
      return;

    case 5U:
      (void)snprintf(buffer, buffer_size, "AI ORI");
      return;

    case 6U:
      (void)snprintf(buffer, buffer_size, "AI BG");
      return;

    case 7U:
      (void)snprintf(buffer, buffer_size, "AI TST");
      return;

    case 8U:
      (void)snprintf(buffer, buffer_size, "AI QTZ");
      return;

    case 9U:
      (void)snprintf(buffer, buffer_size, "AI CLN");
      return;

    case 10U:
      (void)snprintf(buffer, buffer_size, "AI RUN %lu", (unsigned long)g_thermal_ai_diag_run_count);
      return;

    case 11U:
      (void)snprintf(buffer, buffer_size, "AI RET %lu", (unsigned long)g_thermal_ai_diag_run_count);
      return;

    case 12U:
      (void)snprintf(buffer, buffer_size, "AI DEC %lu", (unsigned long)g_thermal_ai_diag_run_count);
      return;

    default:
      break;
  }

  if (g_thermal_ai_model_ready == 0U)
  {
    (void)snprintf(buffer, buffer_size, "AI ARM");
    return;
  }

  inference_ms = g_thermal_ai_last_inference_ms;
  if (tx_mutex_get(&g_thermal_ai_mutex, TX_NO_WAIT) == TX_SUCCESS)
  {
    ai_result_snapshot = g_thermal_ai_runtime.last_result;
    ai_result_valid = 1U;
    max_objectness_permille = g_thermal_ai_diag_max_objectness_permille;
    output_byte0 = g_thermal_ai_diag_output_byte0;
    weight_char = (g_thermal_ai_diag_weights_ok != 0U) ? 'W' : 'X';
    layout_char = (g_thermal_ai_diag_output_layout != 0U) ? 'P' : 'I';
    tx_mutex_put(&g_thermal_ai_mutex);
  }

  if ((ai_result_valid != 0U) &&
      (ai_result_snapshot.detection_count != 0U) &&
      (ai_result_snapshot.detections[0].valid != 0U))
  {
    (void)snprintf(buffer,
                   buffer_size,
                   "AI%c%c D%u %u",
                   weight_char,
                   layout_char,
                   (unsigned int)ai_result_snapshot.detection_count,
                   (unsigned int)((ai_result_snapshot.detections[0].confidence_permille + 5U) / 10U));
    return;
  }

  (void)snprintf(buffer,
                 buffer_size,
                 "AI%c%c %u %02X",
                 weight_char,
                 layout_char,
                 (unsigned int)((max_objectness_permille + 5U) / 10U),
                 (unsigned int)output_byte0);
}

/**
  * @brief  Update the AI diagnostic stage and optionally yield once so GUI can paint it.
  * @param  stage New stage code.
  * @param  allow_settle Non-zero to sleep briefly on the first AI attempt only.
  * @retval None
  */
static void app_threadx_thermal_ai_set_diag_stage(uint8_t stage, uint8_t allow_settle)
{
  g_thermal_ai_diag_stage = stage;

  if ((allow_settle != 0U) && (g_thermal_ai_diag_run_count == 1U))
  {
    tx_thread_sleep((CFG_THERMAL_AI_DIAG_STAGE_SETTLE_TICKS > 0U) ? CFG_THERMAL_AI_DIAG_STAGE_SETTLE_TICKS : 1U);
  }
}

/**
  * @brief  Capture one IAC illegal-access event raised by RIF.
  * @param  PeriphId RIF peripheral identifier that triggered the fault.
  * @retval None
  */
void HAL_RIF_ILA_Callback(uint32_t PeriphId)
{
  g_thermal_ai_iac_last_periph_id = PeriphId;
  g_thermal_ai_iac_fault_count++;
  g_thermal_ai_iac_fault_latched = 1U;
  g_thermal_ai_model_ready = 0U;

  HAL_RIF_IAC_DisableIT(PeriphId);
}

/**
  * @brief  Refresh the preview panel from the latest thermal frame.
  * @param  ui UI context.
  * @retval None
  */
static void app_threadx_gui_update_preview(lv_ui *ui)
{
  const uint16_t *temp14_frame;
  thermal_ai_result_t ai_result_snapshot;
  uint8_t mirror_enable;
  uint8_t flip_enable;
  uint16_t frame_width;
  uint16_t frame_height;
  uint32_t pixel_count;
  uint32_t pixel_index;
  uint16_t min_temp14;
  uint16_t max_temp14;
  uint16_t center_temp14;
  uint16_t min_temp_x = 0U;
  uint16_t min_temp_y = 0U;
  uint16_t max_temp_x = 0U;
  uint16_t max_temp_y = 0U;
  int32_t center_temp_centi_c;
  uint8_t unit_celsius;
  uint8_t extrema_cache_valid;
  uint8_t ai_result_valid = 0U;

  if (ui == NULL)
  {
    return;
  }

  temp14_frame = tiny1c_thermal_app_get_temp14_frame();
  if (temp14_frame == NULL)
  {
    return;
  }

  frame_width = tiny1c_thermal_app_get_frame_width();
  frame_height = tiny1c_thermal_app_get_frame_height();
  mirror_enable = tiny1c_thermal_app_get_preview_mirror_enabled();
  flip_enable = tiny1c_thermal_app_get_preview_flip_enabled();
  unit_celsius = thermal_gui_is_unit_celsius();
  pixel_count = (uint32_t)frame_width * (uint32_t)frame_height;
  min_temp14 = 0xFFFFU;
  max_temp14 = 0U;
  extrema_cache_valid = g_extrema_cache_valid;
  if ((extrema_cache_valid != 0U) &&
      (g_extrema_cache_max_temp_x < frame_width) &&
      (g_extrema_cache_max_temp_y < frame_height) &&
      (g_extrema_cache_min_temp_x < frame_width) &&
      (g_extrema_cache_min_temp_y < frame_height))
  {
    max_temp14 = g_extrema_cache_max_temp14;
    min_temp14 = g_extrema_cache_min_temp14;
    max_temp_x = g_extrema_cache_max_temp_x;
    max_temp_y = g_extrema_cache_max_temp_y;
    min_temp_x = g_extrema_cache_min_temp_x;
    min_temp_y = g_extrema_cache_min_temp_y;
  }
  else
  {
    for (pixel_index = 0U; pixel_index < pixel_count; pixel_index++)
    {
      uint16_t pixel_temp14 = temp14_frame[pixel_index];

      if (pixel_temp14 < min_temp14)
      {
        min_temp14 = pixel_temp14;
        min_temp_x = (uint16_t)(pixel_index % frame_width);
        min_temp_y = (uint16_t)(pixel_index / frame_width);
      }
      if (pixel_temp14 > max_temp14)
      {
        max_temp14 = pixel_temp14;
        max_temp_x = (uint16_t)(pixel_index % frame_width);
        max_temp_y = (uint16_t)(pixel_index / frame_width);
      }
    }
  }

  center_temp14 = temp14_frame[((uint32_t)frame_height / 2U) * (uint32_t)frame_width + ((uint32_t)frame_width / 2U)];
  center_temp_centi_c = tiny1c_thermal_app_get_center_temp_centi_c();
  if (center_temp_centi_c == 0)
  {
    center_temp_centi_c = app_threadx_temp14_to_compensated_centi_celsius(center_temp14);
  }

  app_threadx_gui_update_preview_layer(ui->WidgetsDemo_preview_img,
                                       ui->WidgetsDemo_preview_center_temp,
                                       ui->WidgetsDemo_preview_max_temp,
                                       ui->WidgetsDemo_preview_min_temp,
                                       ui->WidgetsDemo_preview_max_marker,
                                       ui->WidgetsDemo_preview_min_marker,
                                       tiny1c_thermal_app_get_preview_width(),
                                       tiny1c_thermal_app_get_preview_height(),
                                       frame_width,
                                       frame_height,
                                       mirror_enable,
                                       flip_enable,
                                       unit_celsius,
                                       min_temp14,
                                       max_temp14,
                                       center_temp_centi_c,
                                       min_temp_x,
                                       min_temp_y,
                                       max_temp_x,
                                       max_temp_y);
  app_threadx_gui_update_preview_layer(ui->WidgetsDemo_fullscreen_preview_img,
                                       ui->WidgetsDemo_fullscreen_preview_center_temp,
                                       ui->WidgetsDemo_fullscreen_preview_max_temp,
                                       ui->WidgetsDemo_fullscreen_preview_min_temp,
                                       ui->WidgetsDemo_fullscreen_preview_max_marker,
                                       ui->WidgetsDemo_fullscreen_preview_min_marker,
                                       CFG_GUI_FULLSCREEN_WIDTH,
                                       CFG_GUI_FULLSCREEN_HEIGHT,
                                       frame_width,
                                       frame_height,
                                       mirror_enable,
                                       flip_enable,
                                       unit_celsius,
                                       min_temp14,
                                       max_temp14,
                                       center_temp_centi_c,
                                       min_temp_x,
                                       min_temp_y,
                                       max_temp_x,
                                       max_temp_y);

  if ((CFG_THERMAL_AI_ENABLE != 0U) &&
      (tx_mutex_get(&g_thermal_ai_mutex, TX_NO_WAIT) == TX_SUCCESS))
  {
    if (g_thermal_ai_model_ready != 0U)
    {
      ai_result_snapshot = g_thermal_ai_runtime.last_result;
      ai_result_valid = 1U;
    }
    tx_mutex_put(&g_thermal_ai_mutex);
  }

  if (CFG_THERMAL_AI_ENABLE != 0U)
  {
    app_threadx_gui_update_ai_overlay_set(g_thermal_ai_preview_boxes,
                                          g_thermal_ai_preview_labels,
                                          tiny1c_thermal_app_get_preview_width(),
                                          tiny1c_thermal_app_get_preview_height(),
                                          (ai_result_valid != 0U) ? &ai_result_snapshot : NULL);
    app_threadx_gui_update_ai_overlay_set(g_thermal_ai_fullscreen_boxes,
                                          g_thermal_ai_fullscreen_labels,
                                          CFG_GUI_FULLSCREEN_WIDTH,
                                          CFG_GUI_FULLSCREEN_HEIGHT,
                                          (ai_result_valid != 0U) ? &ai_result_snapshot : NULL);
  }
}

/**
  * @brief  Refresh the full-screen image buffer from the latest preview frame.
  * @param  None
  * @retval None
  */
static void app_threadx_gui_update_fullscreen_image(void)
{
  const uint16_t *preview_rgb565_frame;

  preview_rgb565_frame = tiny1c_thermal_app_get_rgb565_frame();
  if (preview_rgb565_frame == NULL)
  {
    return;
  }

  app_threadx_gui_build_fullscreen_frame(preview_rgb565_frame,
                                         g_gui_fullscreen_rgb565_frame,
                                         tiny1c_thermal_app_get_preview_width(),
                                         tiny1c_thermal_app_get_preview_height());
}

/**
  * @brief  Update one preview layer and its overlays.
  * @param  preview_img Preview image object.
  * @param  preview_center_temp Center-temperature badge.
  * @param  preview_max_temp Maximum-temperature badge.
  * @param  preview_min_temp Minimum-temperature badge.
  * @param  preview_max_marker Maximum marker label.
  * @param  preview_min_marker Minimum marker label.
  * @param  preview_width Rendered preview width in pixels.
  * @param  preview_height Rendered preview height in pixels.
  * @param  frame_width Temperature-frame width.
  * @param  frame_height Temperature-frame height.
  * @param  mirror_enable Mirror flag.
  * @param  flip_enable Flip flag.
  * @param  unit_celsius Unit selector flag.
  * @param  min_temp14 Minimum temperature sample.
  * @param  max_temp14 Maximum temperature sample.
  * @param  center_temp_centi_c Center temperature in centi-degrees Celsius.
  * @param  min_temp_x Minimum-sample X coordinate.
  * @param  min_temp_y Minimum-sample Y coordinate.
  * @param  max_temp_x Maximum-sample X coordinate.
  * @param  max_temp_y Maximum-sample Y coordinate.
  * @retval None
  */
static void app_threadx_gui_update_preview_layer(lv_obj_t *preview_img,
                                                 lv_obj_t *preview_center_temp,
                                                 lv_obj_t *preview_max_temp,
                                                 lv_obj_t *preview_min_temp,
                                                 lv_obj_t *preview_max_marker,
                                                 lv_obj_t *preview_min_marker,
                                                 uint16_t preview_width,
                                                 uint16_t preview_height,
                                                 uint16_t frame_width,
                                                 uint16_t frame_height,
                                                 uint8_t mirror_enable,
                                                 uint8_t flip_enable,
                                                 uint8_t unit_celsius,
                                                 uint16_t min_temp14,
                                                 uint16_t max_temp14,
                                                 int32_t center_temp_centi_c,
                                                 uint16_t min_temp_x,
                                                 uint16_t min_temp_y,
                                                 uint16_t max_temp_x,
                                                 uint16_t max_temp_y)
{
  int32_t image_x;
  int32_t image_y;
  int32_t max_disp_x;
  int32_t max_disp_y;
  int32_t min_disp_x;
  int32_t min_disp_y;
  int32_t max_label_x;
  int32_t max_label_y;
  int32_t min_label_x;
  int32_t min_label_y;
  char temp_text[32];

  if ((preview_img == NULL) || (preview_center_temp == NULL) || (preview_max_temp == NULL) || (preview_min_temp == NULL))
  {
    return;
  }

  app_threadx_gui_format_temp_text(temp_text, sizeof(temp_text), "Center ", center_temp_centi_c, unit_celsius);
  app_threadx_gui_set_badge_text(preview_center_temp, temp_text);

  app_threadx_gui_format_temp_text(temp_text, sizeof(temp_text), "Max ", app_threadx_temp14_to_compensated_centi_celsius(max_temp14), unit_celsius);
  app_threadx_gui_set_badge_text(preview_max_temp, temp_text);

  app_threadx_gui_format_temp_text(temp_text, sizeof(temp_text), "Min ", app_threadx_temp14_to_compensated_centi_celsius(min_temp14), unit_celsius);
  app_threadx_gui_set_badge_text(preview_min_temp, temp_text);

  if ((preview_max_marker == NULL) || (preview_min_marker == NULL))
  {
    return;
  }

  image_x = lv_obj_get_x(preview_img);
  image_y = lv_obj_get_y(preview_img);

  (void)mirror_enable;
  (void)flip_enable;
  tiny1c_thermal_app_transform_frame_point(max_temp_x, max_temp_y, &max_temp_x, &max_temp_y);
  tiny1c_thermal_app_transform_frame_point(min_temp_x, min_temp_y, &min_temp_x, &min_temp_y);
  max_disp_x = (int32_t)max_temp_x;
  max_disp_y = (int32_t)max_temp_y;
  min_disp_x = (int32_t)min_temp_x;
  min_disp_y = (int32_t)min_temp_y;

  max_label_x = image_x + max_disp_x * ((int32_t)preview_width / (int32_t)frame_width) + 4;
  max_label_y = image_y + max_disp_y * ((int32_t)preview_height / (int32_t)frame_height) - 12;
  min_label_x = image_x + min_disp_x * ((int32_t)preview_width / (int32_t)frame_width) + 4;
  min_label_y = image_y + min_disp_y * ((int32_t)preview_height / (int32_t)frame_height) - 12;

  if (max_label_x < image_x)
  {
    max_label_x = image_x;
  }
  if (max_label_y < image_y)
  {
    max_label_y = image_y;
  }
  if (min_label_x < image_x)
  {
    min_label_x = image_x;
  }
  if (min_label_y < image_y)
  {
    min_label_y = image_y;
  }
  if (max_label_x > (image_x + (int32_t)preview_width - 18))
  {
    max_label_x = image_x + (int32_t)preview_width - 18;
  }
  if (max_label_y > (image_y + (int32_t)preview_height - 18))
  {
    max_label_y = image_y + (int32_t)preview_height - 18;
  }
  if (min_label_x > (image_x + (int32_t)preview_width - 18))
  {
    min_label_x = image_x + (int32_t)preview_width - 18;
  }
  if (min_label_y > (image_y + (int32_t)preview_height - 18))
  {
    min_label_y = image_y + (int32_t)preview_height - 18;
  }

  lv_obj_set_pos(preview_max_marker, (lv_coord_t)max_label_x, (lv_coord_t)max_label_y);
  lv_obj_set_pos(preview_min_marker, (lv_coord_t)min_label_x, (lv_coord_t)min_label_y);
}

/**
  * @brief  Build a 2x full-screen RGB565 frame with light 2x2 smoothing.
  * @param  source_frame Source preview frame.
  * @param  dest_frame Destination full-screen frame.
  * @param  source_width Source frame width.
  * @param  source_height Source frame height.
  * @retval None
  */
static void app_threadx_gui_build_fullscreen_frame(const uint16_t *source_frame,
                                                   uint16_t *dest_frame,
                                                   uint16_t source_width,
                                                   uint16_t source_height)
{
  uint32_t dest_width;
  uint32_t src_y;
  uint32_t src_x;

  if ((source_frame == NULL) || (dest_frame == NULL) || (source_width == 0U) || (source_height == 0U))
  {
    return;
  }

  dest_width = (uint32_t)source_width * 2U;
  for (src_y = 0U; src_y < source_height; src_y++)
  {
    uint32_t row0_index = (src_y * 2U) * dest_width;
    uint32_t row1_index = row0_index + dest_width;
    uint32_t src_row_index = src_y * source_width;
    uint32_t src_row_next_index = ((src_y + 1U < source_height) ? (src_y + 1U) : src_y) * source_width;

    for (src_x = 0U; src_x < source_width; src_x++)
    {
      uint32_t src_x_next = (src_x + 1U < source_width) ? (src_x + 1U) : src_x;
      uint16_t pixel = app_threadx_rgb565_average4(source_frame[src_row_index + src_x],
                                                   source_frame[src_row_index + src_x_next],
                                                   source_frame[src_row_next_index + src_x],
                                                   source_frame[src_row_next_index + src_x_next]);
      uint32_t dest_index = src_x * 2U;

      dest_frame[row0_index + dest_index] = pixel;
      dest_frame[row0_index + dest_index + 1U] = pixel;
      dest_frame[row1_index + dest_index] = pixel;
      dest_frame[row1_index + dest_index + 1U] = pixel;
    }
  }
}

/**
  * @brief  Return whether the compiled thermal network buffers are ready for inference.
  * @retval uint8_t 1 when input/output buffers are ready, otherwise 0.
  */
static uint8_t app_threadx_thermal_ai_prepare_io(void)
{
  if ((g_thermal_ai_input_buffer != NULL) && (g_thermal_ai_output_buffer != NULL))
  {
    return 1U;
  }

  g_thermal_ai_input_info = LL_ATON_Input_Buffers_Info_thermal();
  g_thermal_ai_output_info = LL_ATON_Output_Buffers_Info_thermal();
  if ((g_thermal_ai_input_info == NULL) || (g_thermal_ai_output_info == NULL))
  {
    app_threadx_thermal_ai_set_diag_stage(2U, 1U);
    return 0U;
  }

  g_thermal_ai_input_buffer = (int8_t *)(void *)LL_Buffer_addr_start(&g_thermal_ai_input_info[0]);
  g_thermal_ai_output_buffer = (int8_t *)(void *)LL_Buffer_addr_start(&g_thermal_ai_output_info[0]);
  if ((g_thermal_ai_input_buffer == NULL) || (g_thermal_ai_output_buffer == NULL))
  {
    app_threadx_thermal_ai_set_diag_stage(3U, 1U);
    return 0U;
  }

  if ((LL_Buffer_len(&g_thermal_ai_input_info[0]) != CFG_THERMAL_AI_INPUT_PIXELS) ||
      (LL_Buffer_len(&g_thermal_ai_output_info[0]) != CFG_THERMAL_AI_OUTPUT_BYTES))
  {
    app_threadx_thermal_ai_set_diag_stage(4U, 1U);
    g_thermal_ai_input_buffer = NULL;
    g_thermal_ai_output_buffer = NULL;
    return 0U;
  }

  return 1U;
}

/**
  * @brief  Load one fixed reference int8 input tensor for board-side AI self-test.
  * @param  background_temp14_ptr Optional output for background temp14 metadata.
  * @param  max_temp14_ptr Optional output for max temp14 metadata.
  * @retval uint8_t 1 when the reference tensor was copied, otherwise 0.
  */
static uint8_t app_threadx_thermal_ai_load_reference_input(uint16_t *background_temp14_ptr,
                                                           uint16_t *max_temp14_ptr)
{
#if (CFG_THERMAL_AI_USE_REFERENCE_INPUT != 0U)
  uint32_t reference_len;

  if (g_thermal_ai_input_buffer == NULL)
  {
    return 0U;
  }

  reference_len = (uint32_t)(g_thermal_ai_reference_input_end - g_thermal_ai_reference_input);
  if (reference_len != CFG_THERMAL_AI_INPUT_PIXELS)
  {
    return 0U;
  }

  (void)memcpy(g_thermal_ai_input_buffer, g_thermal_ai_reference_input, reference_len);
  if (background_temp14_ptr != NULL)
  {
    *background_temp14_ptr = 0U;
  }
  if (max_temp14_ptr != NULL)
  {
    *max_temp14_ptr = 0U;
  }
  return 1U;
#else
  TX_PARAMETER_NOT_USED(background_temp14_ptr);
  TX_PARAMETER_NOT_USED(max_temp14_ptr);
  return 0U;
#endif
}

/**
  * @brief  Probe the first bytes of the external weight blob at 0x71000000.
  * @retval uint8_t 1 when the signature matches the generated weight file header.
  */
static uint8_t app_threadx_thermal_ai_probe_weight_signature(void)
{
  volatile const uint8_t *weight_ptr = (volatile const uint8_t *)0x71000000UL;

  if ((weight_ptr[0] == 0x11U) &&
      (weight_ptr[1] == 0x02U) &&
      (weight_ptr[2] == 0x21U) &&
      (weight_ptr[3] == 0x0FU))
  {
    return 1U;
  }

  return 0U;
}

/**
  * @brief  Return one percentile value from the current temp14 histogram.
  * @param  pixel_count Total number of pixels accumulated in the histogram.
  * @retval uint16_t Percentile temp14 value.
  */
static uint16_t app_threadx_thermal_ai_percentile_from_histogram(uint32_t pixel_count)
{
  uint32_t cumulative = 0U;
  uint32_t rank;
  uint32_t histogram_index;

  if (pixel_count == 0U)
  {
    return 0U;
  }

  rank = ((pixel_count - 1U) * CFG_THERMAL_AI_BACKGROUND_PERCENT_NUM) / CFG_THERMAL_AI_BACKGROUND_PERCENT_DEN;
  for (histogram_index = 0U; histogram_index < CFG_THERMAL_AI_HISTOGRAM_BINS; histogram_index++)
  {
    cumulative += (uint32_t)g_thermal_ai_histogram[histogram_index];
    if (cumulative > rank)
    {
      return (uint16_t)histogram_index;
    }
  }

  return (uint16_t)(CFG_THERMAL_AI_HISTOGRAM_BINS - 1U);
}

/**
  * @brief  Build one preview-oriented AI input tensor from the latest raw temp14 frame.
  * @param  temp14_frame Pointer to the latest raw temp14 frame.
  * @param  background_temp14_ptr Output median/background temp14 pointer.
  * @param  max_temp14_ptr Output frame maximum temp14 pointer.
  * @retval None
  */
static void app_threadx_thermal_ai_build_input(const uint16_t *temp14_frame,
                                               uint16_t *background_temp14_ptr,
                                               uint16_t *max_temp14_ptr)
{
  uint32_t pixel_index = 0U;
  uint16_t background_temp14;
  uint16_t max_temp14 = 0U;
  float input_scale;
  int16_t input_zero_point;
  uint32_t y_out;
  uint32_t x_out;
  volatile int8_t input_probe;

  if ((temp14_frame == NULL) || (g_thermal_ai_input_buffer == NULL))
  {
    return;
  }

  (void)memset(g_thermal_ai_histogram, 0, sizeof(g_thermal_ai_histogram));
  app_threadx_thermal_ai_set_diag_stage(5U, 1U);

  for (y_out = 0U; y_out < CFG_THERMAL_AI_INPUT_HEIGHT; y_out++)
  {
    for (x_out = 0U; x_out < CFG_THERMAL_AI_INPUT_WIDTH; x_out++)
    {
      uint16_t temp14_value = app_threadx_temp14_get_preview_oriented_sample(temp14_frame,
                                                                              CFG_THERMAL_AI_INPUT_WIDTH,
                                                                              CFG_THERMAL_AI_INPUT_HEIGHT,
                                                                              (uint16_t)x_out,
                                                                              (uint16_t)y_out);
      uint16_t histogram_value = (temp14_value < (CFG_THERMAL_AI_HISTOGRAM_BINS - 1U))
                                   ? temp14_value
                                   : (uint16_t)(CFG_THERMAL_AI_HISTOGRAM_BINS - 1U);

      g_thermal_ai_oriented_temp14[pixel_index++] = temp14_value;
      g_thermal_ai_histogram[histogram_value]++;
      if (temp14_value > max_temp14)
      {
        max_temp14 = temp14_value;
      }
    }
  }

  background_temp14 = app_threadx_thermal_ai_percentile_from_histogram(CFG_THERMAL_AI_INPUT_PIXELS);
  input_scale = (g_thermal_ai_input_info[0].scale != NULL) ? g_thermal_ai_input_info[0].scale[0] : (1.0f / 255.0f);
  input_zero_point = (g_thermal_ai_input_info[0].offset != NULL) ? g_thermal_ai_input_info[0].offset[0] : -128;
  app_threadx_thermal_ai_set_diag_stage(6U, 1U);

  /* Probe the first CPU access to the generated NPU input buffer explicitly. */
  app_threadx_thermal_ai_set_diag_stage(7U, 1U);
  input_probe = g_thermal_ai_input_buffer[0];
  g_thermal_ai_input_buffer[0] = input_probe;
  app_threadx_thermal_ai_set_diag_stage(8U, 1U);

  for (pixel_index = 0U; pixel_index < CFG_THERMAL_AI_INPUT_PIXELS; pixel_index++)
  {
    float delta_temp14 = (float)((int32_t)g_thermal_ai_oriented_temp14[pixel_index] - (int32_t)background_temp14);
    float normalized = (delta_temp14 - CFG_THERMAL_AI_DELTA_TEMP14_MIN) / CFG_THERMAL_AI_DELTA_TEMP14_SPAN;
    float quantized;
    int32_t quantized_i32;

    if (normalized < 0.0f)
    {
      normalized = 0.0f;
    }
    else if (normalized > 1.0f)
    {
      normalized = 1.0f;
    }

    quantized = (normalized / input_scale) + (float)input_zero_point;
    quantized_i32 = (int32_t)lrintf(quantized);
    if (quantized_i32 < -128)
    {
      quantized_i32 = -128;
    }
    else if (quantized_i32 > 127)
    {
      quantized_i32 = 127;
    }

    g_thermal_ai_input_buffer[pixel_index] = (int8_t)quantized_i32;
  }

  if (background_temp14_ptr != NULL)
  {
    *background_temp14_ptr = background_temp14;
  }
  if (max_temp14_ptr != NULL)
  {
    *max_temp14_ptr = max_temp14;
  }
}

/**
  * @brief  Return the sigmoid activation for one floating-point value.
  * @param  value Input value.
  * @retval float Sigmoid output in range 0..1.
  */
static float app_threadx_thermal_ai_sigmoid(float value)
{
  if (value >= 0.0f)
  {
    float exp_neg = expf(-value);
    return 1.0f / (1.0f + exp_neg);
  }

  {
    float exp_pos = expf(value);
    return exp_pos / (1.0f + exp_pos);
  }
}

/**
  * @brief  Map one detector class index into the project runtime class identifier.
  * @param  class_index Zero-based detector class index.
  * @retval uint8_t Thermal AI runtime class identifier.
  */
static uint8_t app_threadx_thermal_ai_class_index_to_runtime_id(uint32_t class_index)
{
  switch (class_index)
  {
    case 0U:
      return (uint8_t)THERMAL_AI_CLASS_PERSON;
    case 1U:
      return (uint8_t)THERMAL_AI_CLASS_CIRCUIT_BOARD_NORMAL;
    case 2U:
      return (uint8_t)THERMAL_AI_CLASS_CIRCUIT_BOARD_ABNORMAL_HOTSPOT;
    default:
      return (uint8_t)THERMAL_AI_CLASS_NONE;
  }
}

/**
  * @brief  Read one quantized detector output value using one candidate output layout.
  * @param  grid_y Grid Y index.
  * @param  grid_x Grid X index.
  * @param  channel_index Output-channel index.
  * @param  channel_planar_layout Non-zero for channel-major planar layout.
  * @retval int8_t Raw quantized output value.
  */
static int8_t app_threadx_thermal_ai_read_output_q7(uint32_t grid_y,
                                                    uint32_t grid_x,
                                                    uint32_t channel_index,
                                                    uint8_t channel_planar_layout)
{
  uint32_t linear_index;

  if (channel_planar_layout != 0U)
  {
    linear_index = (channel_index * CFG_THERMAL_AI_GRID_HEIGHT * CFG_THERMAL_AI_GRID_WIDTH) +
                   (grid_y * CFG_THERMAL_AI_GRID_WIDTH) +
                   grid_x;
  }
  else
  {
    linear_index = ((grid_y * CFG_THERMAL_AI_GRID_WIDTH) + grid_x) * CFG_THERMAL_AI_OUTPUT_CHANNELS +
                   channel_index;
  }

  return g_thermal_ai_output_buffer[linear_index];
}

/**
  * @brief  Decode one candidate output layout into the temporary candidate list.
  * @param  output_scale Output tensor dequant scale.
  * @param  output_zero_point Output tensor dequant zero point.
  * @param  channel_planar_layout Non-zero for channel-major planar layout.
  * @param  raw_candidate_count_ptr Output pointer for pre-filter candidate count.
  * @param  max_score_permille_ptr Output pointer for best detection score in permille.
  * @param  max_objectness_permille_ptr Output pointer for best objectness score in permille.
  * @retval uint32_t Candidate count kept after class thresholding and geometric filtering.
  */
static uint32_t app_threadx_thermal_ai_collect_candidates(float output_scale,
                                                          int16_t output_zero_point,
                                                          uint8_t channel_planar_layout,
                                                          uint32_t *raw_candidate_count_ptr,
                                                          uint16_t *max_score_permille_ptr,
                                                          uint16_t *max_objectness_permille_ptr)
{
  float stride_x = (float)CFG_THERMAL_AI_INPUT_WIDTH / (float)CFG_THERMAL_AI_GRID_WIDTH;
  float stride_y = (float)CFG_THERMAL_AI_INPUT_HEIGHT / (float)CFG_THERMAL_AI_GRID_HEIGHT;
  uint32_t grid_y;
  uint32_t grid_x;
  uint32_t raw_candidate_count = 0U;
  uint32_t candidate_count = 0U;
  uint16_t max_score_permille = 0U;
  uint16_t max_objectness_permille = 0U;

  for (grid_y = 0U; grid_y < CFG_THERMAL_AI_GRID_HEIGHT; grid_y++)
  {
    for (grid_x = 0U; grid_x < CFG_THERMAL_AI_GRID_WIDTH; grid_x++)
    {
      float logits[CFG_THERMAL_AI_OUTPUT_CHANNELS];
      float objectness_score;
      float class_scores[3];
      uint32_t class_index = 0U;
      float class_score;
      float detection_score;
      float offset_x;
      float offset_y;
      float width_norm;
      float height_norm;
      float center_x;
      float center_y;
      float box_width;
      float box_height;
      app_threadx_ai_candidate_t *candidate_ptr;
      uint32_t channel_index;

      for (channel_index = 0U; channel_index < CFG_THERMAL_AI_OUTPUT_CHANNELS; channel_index++)
      {
        logits[channel_index] =
            ((float)((int32_t)app_threadx_thermal_ai_read_output_q7(grid_y,
                                                                    grid_x,
                                                                    channel_index,
                                                                    channel_planar_layout) -
                     (int32_t)output_zero_point)) *
            output_scale;
      }

      objectness_score = app_threadx_thermal_ai_sigmoid(logits[0]);
      if ((uint16_t)(objectness_score * 1000.0f + 0.5f) > max_objectness_permille)
      {
        max_objectness_permille = (uint16_t)(objectness_score * 1000.0f + 0.5f);
      }
      if (objectness_score < CFG_THERMAL_AI_OBJECTNESS_THRESHOLD)
      {
        continue;
      }

      class_scores[0] = app_threadx_thermal_ai_sigmoid(logits[5]);
      class_scores[1] = app_threadx_thermal_ai_sigmoid(logits[6]);
      class_scores[2] = app_threadx_thermal_ai_sigmoid(logits[7]);

      if (class_scores[1] > class_scores[class_index])
      {
        class_index = 1U;
      }
      if (class_scores[2] > class_scores[class_index])
      {
        class_index = 2U;
      }

      class_score = class_scores[class_index];
      detection_score = objectness_score * class_score;
      if ((uint16_t)(detection_score * 1000.0f + 0.5f) > max_score_permille)
      {
        max_score_permille = (uint16_t)(detection_score * 1000.0f + 0.5f);
      }
      if ((class_score < CFG_THERMAL_AI_CLASS_THRESHOLD) || (detection_score < CFG_THERMAL_AI_SCORE_THRESHOLD))
      {
        continue;
      }

      raw_candidate_count++;

      if (candidate_count >= CFG_THERMAL_AI_MAX_CANDIDATES)
      {
        continue;
      }

      offset_x = app_threadx_thermal_ai_sigmoid(logits[1]);
      offset_y = app_threadx_thermal_ai_sigmoid(logits[2]);
      width_norm = app_threadx_thermal_ai_sigmoid(logits[3]);
      height_norm = app_threadx_thermal_ai_sigmoid(logits[4]);
      center_x = ((float)grid_x + offset_x) * stride_x;
      center_y = ((float)grid_y + offset_y) * stride_y;
      box_width = width_norm * (float)CFG_THERMAL_AI_INPUT_WIDTH;
      box_height = height_norm * (float)CFG_THERMAL_AI_INPUT_HEIGHT;
      if (box_width < 1.0f)
      {
        box_width = 1.0f;
      }
      if (box_height < 1.0f)
      {
        box_height = 1.0f;
      }

      candidate_ptr = &g_thermal_ai_candidates[candidate_count++];
      candidate_ptr->class_id = app_threadx_thermal_ai_class_index_to_runtime_id(class_index);
      candidate_ptr->score = detection_score;
      candidate_ptr->confidence_permille = (uint16_t)(detection_score * 1000.0f + 0.5f);
      candidate_ptr->x_min = center_x - (box_width * 0.5f);
      candidate_ptr->y_min = center_y - (box_height * 0.5f);
      candidate_ptr->x_max = center_x + (box_width * 0.5f);
      candidate_ptr->y_max = center_y + (box_height * 0.5f);

      if (candidate_ptr->x_min < 0.0f)
      {
        candidate_ptr->x_min = 0.0f;
      }
      if (candidate_ptr->y_min < 0.0f)
      {
        candidate_ptr->y_min = 0.0f;
      }
      if (candidate_ptr->x_max > (float)CFG_THERMAL_AI_INPUT_WIDTH)
      {
        candidate_ptr->x_max = (float)CFG_THERMAL_AI_INPUT_WIDTH;
      }
      if (candidate_ptr->y_max > (float)CFG_THERMAL_AI_INPUT_HEIGHT)
      {
        candidate_ptr->y_max = (float)CFG_THERMAL_AI_INPUT_HEIGHT;
      }

      if ((candidate_ptr->class_id == (uint8_t)THERMAL_AI_CLASS_PERSON) &&
          (app_threadx_thermal_ai_person_filter(candidate_ptr) == 0U))
      {
        candidate_count--;
      }
    }
  }

  if (raw_candidate_count_ptr != NULL)
  {
    *raw_candidate_count_ptr = raw_candidate_count;
  }
  if (max_score_permille_ptr != NULL)
  {
    *max_score_permille_ptr = max_score_permille;
  }
  if (max_objectness_permille_ptr != NULL)
  {
    *max_objectness_permille_ptr = max_objectness_permille;
  }

  return candidate_count;
}

/**
  * @brief  Apply the board-side person-specific geometric false-positive filter.
  * @param  candidate_ptr Candidate detection to test.
  * @retval uint8_t 1 when the candidate passes, otherwise 0.
  */
static uint8_t app_threadx_thermal_ai_person_filter(const app_threadx_ai_candidate_t *candidate_ptr)
{
  float width;
  float height;
  uint8_t touches_top;
  uint8_t touches_left;
  uint8_t touches_right;

  if (candidate_ptr == NULL)
  {
    return 0U;
  }

  width = candidate_ptr->x_max - candidate_ptr->x_min;
  height = candidate_ptr->y_max - candidate_ptr->y_min;
  if ((width < CFG_THERMAL_AI_PERSON_MIN_WIDTH_PX) || (height < CFG_THERMAL_AI_PERSON_MIN_HEIGHT_PX))
  {
    return 0U;
  }

  touches_top = (candidate_ptr->y_min <= CFG_THERMAL_AI_PERSON_TOP_MARGIN_PX) ? 1U : 0U;
  touches_left = (candidate_ptr->x_min <= CFG_THERMAL_AI_PERSON_SIDE_MARGIN_PX) ? 1U : 0U;
  touches_right = (candidate_ptr->x_max >= ((float)CFG_THERMAL_AI_INPUT_WIDTH - CFG_THERMAL_AI_PERSON_SIDE_MARGIN_PX)) ? 1U : 0U;

  if (touches_top != 0U)
  {
    return 0U;
  }
  if ((touches_left != 0U) || (touches_right != 0U))
  {
    return 0U;
  }
  if ((touches_top != 0U) && (height <= CFG_THERMAL_AI_PERSON_TOP_BAND_MAX_H))
  {
    return 0U;
  }
  if ((touches_top != 0U) && (touches_left != 0U) &&
      (width <= CFG_THERMAL_AI_PERSON_CORNER_MAX_W) && (height <= CFG_THERMAL_AI_PERSON_CORNER_MAX_H))
  {
    return 0U;
  }
  if ((touches_top != 0U) && (touches_right != 0U) &&
      (width <= CFG_THERMAL_AI_PERSON_CORNER_MAX_W) && (height <= CFG_THERMAL_AI_PERSON_CORNER_MAX_H))
  {
    return 0U;
  }
  if ((touches_left != 0U) && (width <= CFG_THERMAL_AI_PERSON_EDGE_STRIP_MAX_W) &&
      (height >= CFG_THERMAL_AI_PERSON_EDGE_STRIP_MIN_H))
  {
    return 0U;
  }
  if ((touches_right != 0U) && (width <= CFG_THERMAL_AI_PERSON_EDGE_STRIP_MAX_W) &&
      (height >= CFG_THERMAL_AI_PERSON_EDGE_STRIP_MIN_H))
  {
    return 0U;
  }

  return 1U;
}

/**
  * @brief  Compute the IoU between two decoded AI candidates.
  * @param  lhs_ptr First candidate.
  * @param  rhs_ptr Second candidate.
  * @retval float Intersection-over-union ratio.
  */
static float app_threadx_thermal_ai_candidate_iou(const app_threadx_ai_candidate_t *lhs_ptr,
                                                  const app_threadx_ai_candidate_t *rhs_ptr)
{
  float inter_x_min;
  float inter_y_min;
  float inter_x_max;
  float inter_y_max;
  float inter_w;
  float inter_h;
  float inter_area;
  float lhs_area;
  float rhs_area;
  float union_area;

  if ((lhs_ptr == NULL) || (rhs_ptr == NULL))
  {
    return 0.0f;
  }

  inter_x_min = (lhs_ptr->x_min > rhs_ptr->x_min) ? lhs_ptr->x_min : rhs_ptr->x_min;
  inter_y_min = (lhs_ptr->y_min > rhs_ptr->y_min) ? lhs_ptr->y_min : rhs_ptr->y_min;
  inter_x_max = (lhs_ptr->x_max < rhs_ptr->x_max) ? lhs_ptr->x_max : rhs_ptr->x_max;
  inter_y_max = (lhs_ptr->y_max < rhs_ptr->y_max) ? lhs_ptr->y_max : rhs_ptr->y_max;
  inter_w = inter_x_max - inter_x_min;
  inter_h = inter_y_max - inter_y_min;
  if ((inter_w <= 0.0f) || (inter_h <= 0.0f))
  {
    return 0.0f;
  }

  inter_area = inter_w * inter_h;
  lhs_area = (lhs_ptr->x_max - lhs_ptr->x_min) * (lhs_ptr->y_max - lhs_ptr->y_min);
  rhs_area = (rhs_ptr->x_max - rhs_ptr->x_min) * (rhs_ptr->y_max - rhs_ptr->y_min);
  union_area = lhs_area + rhs_area - inter_area;
  if (union_area <= 0.0f)
  {
    return 0.0f;
  }

  return inter_area / union_area;
}

/**
  * @brief  Decode the current raw network output buffer into project detections.
  * @param  result_ptr Output runtime result object.
  * @retval None
  */
static void app_threadx_thermal_ai_decode_output(thermal_ai_result_t *result_ptr)
{
  float output_scale;
  int16_t output_zero_point;
  uint32_t grid_y;
  uint32_t grid_x;
  uint32_t candidate_count = 0U;
  uint32_t kept_count = 0U;
  uint32_t raw_candidate_count = 0U;
  uint32_t fallback_raw_candidate_count = 0U;
  uint32_t fallback_candidate_count = 0U;
  uint32_t class_loop_index;
  uint8_t candidate_used[CFG_THERMAL_AI_MAX_CANDIDATES];
  uint16_t max_score_permille = 0U;
  uint16_t fallback_max_score_permille = 0U;
  uint16_t max_objectness_permille = 0U;
  uint16_t fallback_max_objectness_permille = 0U;
  static const uint8_t class_order[] = {
    (uint8_t)THERMAL_AI_CLASS_PERSON,
    (uint8_t)THERMAL_AI_CLASS_CIRCUIT_BOARD_NORMAL,
    (uint8_t)THERMAL_AI_CLASS_CIRCUIT_BOARD_ABNORMAL_HOTSPOT
  };

  if ((result_ptr == NULL) || (g_thermal_ai_output_buffer == NULL))
  {
    return;
  }

  output_scale = (g_thermal_ai_output_info[0].scale != NULL) ? g_thermal_ai_output_info[0].scale[0] : 1.0f;
  output_zero_point = (g_thermal_ai_output_info[0].offset != NULL) ? g_thermal_ai_output_info[0].offset[0] : 0;
  candidate_count = app_threadx_thermal_ai_collect_candidates(output_scale,
                                                              output_zero_point,
                                                              0U,
                                                              &raw_candidate_count,
                                                              &max_score_permille,
                                                              &max_objectness_permille);
  g_thermal_ai_diag_output_layout = 0U;
  if (candidate_count == 0U)
  {
    fallback_candidate_count = app_threadx_thermal_ai_collect_candidates(output_scale,
                                                                         output_zero_point,
                                                                         1U,
                                                                         &fallback_raw_candidate_count,
                                                                         &fallback_max_score_permille,
                                                                         &fallback_max_objectness_permille);
    if ((fallback_candidate_count > 0U) ||
        (fallback_raw_candidate_count > raw_candidate_count) ||
        (fallback_max_score_permille > max_score_permille) ||
        (fallback_max_objectness_permille > max_objectness_permille))
    {
      candidate_count = fallback_candidate_count;
      raw_candidate_count = fallback_raw_candidate_count;
      max_score_permille = fallback_max_score_permille;
      max_objectness_permille = fallback_max_objectness_permille;
      g_thermal_ai_diag_output_layout = 1U;
    }
    else
    {
      candidate_count = app_threadx_thermal_ai_collect_candidates(output_scale,
                                                                  output_zero_point,
                                                                  0U,
                                                                  &raw_candidate_count,
                                                                  &max_score_permille,
                                                                  &max_objectness_permille);
    }
  }

  (void)memset(candidate_used, 0, sizeof(candidate_used));
  for (class_loop_index = 0U; class_loop_index < (sizeof(class_order) / sizeof(class_order[0])); class_loop_index++)
  {
    uint8_t target_class_id = class_order[class_loop_index];

    for (;;)
    {
      uint32_t best_index = CFG_THERMAL_AI_MAX_CANDIDATES;
      float best_score = -1.0f;
      uint32_t candidate_index;

      for (candidate_index = 0U; candidate_index < candidate_count; candidate_index++)
      {
        if ((candidate_used[candidate_index] != 0U) ||
            (g_thermal_ai_candidates[candidate_index].class_id != target_class_id))
        {
          continue;
        }
        if (g_thermal_ai_candidates[candidate_index].score > best_score)
        {
          best_score = g_thermal_ai_candidates[candidate_index].score;
          best_index = candidate_index;
        }
      }

      if (best_index >= candidate_count)
      {
        break;
      }

      g_thermal_ai_kept_candidates[kept_count++] = g_thermal_ai_candidates[best_index];
      candidate_used[best_index] = 1U;
      for (candidate_index = 0U; candidate_index < candidate_count; candidate_index++)
      {
        if ((candidate_used[candidate_index] == 0U) &&
            (g_thermal_ai_candidates[candidate_index].class_id == target_class_id) &&
            (app_threadx_thermal_ai_candidate_iou(&g_thermal_ai_candidates[best_index],
                                                  &g_thermal_ai_candidates[candidate_index]) >= CFG_THERMAL_AI_NMS_IOU_THRESHOLD))
        {
          candidate_used[candidate_index] = 1U;
        }
      }
    }
  }

  for (grid_y = 0U; grid_y < kept_count; grid_y++)
  {
    for (grid_x = grid_y + 1U; grid_x < kept_count; grid_x++)
    {
      if (g_thermal_ai_kept_candidates[grid_x].score > g_thermal_ai_kept_candidates[grid_y].score)
      {
        app_threadx_ai_candidate_t temp = g_thermal_ai_kept_candidates[grid_y];
        g_thermal_ai_kept_candidates[grid_y] = g_thermal_ai_kept_candidates[grid_x];
        g_thermal_ai_kept_candidates[grid_x] = temp;
      }
    }
  }

  if (kept_count > CFG_THERMAL_AI_MAX_DETECTIONS)
  {
    kept_count = CFG_THERMAL_AI_MAX_DETECTIONS;
  }

  result_ptr->detection_count = (uint8_t)kept_count;
  for (grid_y = 0U; grid_y < CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS; grid_y++)
  {
    result_ptr->detections[grid_y].valid = 0U;
  }

  for (grid_y = 0U; grid_y < kept_count; grid_y++)
  {
    const app_threadx_ai_candidate_t *candidate_ptr = &g_thermal_ai_kept_candidates[grid_y];
    thermal_ai_detection_t *detection_ptr = &result_ptr->detections[grid_y];

    detection_ptr->valid = 1U;
    detection_ptr->class_id = candidate_ptr->class_id;
    detection_ptr->confidence_permille = candidate_ptr->confidence_permille;
    detection_ptr->bbox.x_min = (uint16_t)(candidate_ptr->x_min + 0.5f);
    detection_ptr->bbox.y_min = (uint16_t)(candidate_ptr->y_min + 0.5f);
    detection_ptr->bbox.x_max = (uint16_t)(candidate_ptr->x_max + 0.5f);
    detection_ptr->bbox.y_max = (uint16_t)(candidate_ptr->y_max + 0.5f);
  }

  g_thermal_ai_diag_raw_candidate_count = (uint16_t)raw_candidate_count;
  g_thermal_ai_diag_postfilter_candidate_count = (uint16_t)candidate_count;
  g_thermal_ai_diag_max_score_permille = max_score_permille;
  g_thermal_ai_diag_max_objectness_permille = max_objectness_permille;
}

/**
  * @brief  Clean one address range from D-Cache with 32-byte alignment.
  * @param  buffer_addr Buffer start address.
  * @param  buffer_size Buffer size in bytes.
  * @retval None
  */
static void app_threadx_dcache_clean_by_addr(void *buffer_addr, uint32_t buffer_size)
{
  uintptr_t start_addr;
  uintptr_t end_addr;

  if ((buffer_addr == NULL) || (buffer_size == 0U))
  {
    return;
  }

  start_addr = ((uintptr_t)buffer_addr) & ~((uintptr_t)31U);
  end_addr = (((uintptr_t)buffer_addr + (uintptr_t)buffer_size + (uintptr_t)31U) & ~((uintptr_t)31U));
  SCB_CleanDCache_by_Addr((uint32_t *)start_addr, (int32_t)(end_addr - start_addr));
}

/**
  * @brief  Invalidate one address range from D-Cache with 32-byte alignment.
  * @param  buffer_addr Buffer start address.
  * @param  buffer_size Buffer size in bytes.
  * @retval None
  */
static void app_threadx_dcache_invalidate_by_addr(void *buffer_addr, uint32_t buffer_size)
{
  uintptr_t start_addr;
  uintptr_t end_addr;

  if ((buffer_addr == NULL) || (buffer_size == 0U))
  {
    return;
  }

  start_addr = ((uintptr_t)buffer_addr) & ~((uintptr_t)31U);
  end_addr = (((uintptr_t)buffer_addr + (uintptr_t)buffer_size + (uintptr_t)31U) & ~((uintptr_t)31U));
  SCB_InvalidateDCache_by_Addr((uint32_t *)start_addr, (int32_t)(end_addr - start_addr));
}

/**
  * @brief  Execute one complete thermal AI inference on the latest temp14 frame.
  * @param  temp14_frame Pointer to the latest raw temp14 frame.
  * @param  frame_counter Current frame counter.
  * @retval None
  */
static void app_threadx_thermal_ai_run_inference(const uint16_t *temp14_frame, uint32_t frame_counter)
{
  thermal_ai_result_t result;
  uint16_t background_temp14 = 0U;
  uint16_t max_temp14 = 0U;
  ULONG tick_start;

  if (CFG_THERMAL_AI_ENABLE == 0U)
  {
    return;
  }

  g_thermal_ai_diag_run_count++;
  app_threadx_thermal_ai_set_diag_stage(1U, 1U);

  if (g_thermal_ai_iac_fault_latched != 0U)
  {
    return;
  }

  if ((temp14_frame == NULL) || (app_threadx_thermal_ai_prepare_io() == 0U))
  {
    return;
  }

  g_thermal_ai_diag_weights_ok = app_threadx_thermal_ai_probe_weight_signature();
  g_thermal_ai_diag_output_byte0 = 0U;
  g_thermal_ai_diag_raw_candidate_count = 0U;
  g_thermal_ai_diag_postfilter_candidate_count = 0U;
  g_thermal_ai_diag_max_score_permille = 0U;
  g_thermal_ai_diag_max_objectness_permille = 0U;
  g_thermal_ai_diag_output_layout = 0U;

  if (app_threadx_thermal_ai_load_reference_input(&background_temp14, &max_temp14) == 0U)
  {
    app_threadx_thermal_ai_build_input(temp14_frame, &background_temp14, &max_temp14);
  }
  app_threadx_thermal_ai_set_diag_stage(9U, 1U);
  app_threadx_dcache_clean_by_addr(g_thermal_ai_input_buffer, LL_Buffer_len(&g_thermal_ai_input_info[0]));

  app_threadx_thermal_ai_set_diag_stage(10U, 1U);

  tick_start = HAL_GetTick();
  LL_ATON_RT_Main(&NN_Instance_thermal);
  g_thermal_ai_last_inference_ms = HAL_GetTick() - tick_start;

  app_threadx_thermal_ai_set_diag_stage(11U, 0U);
  app_threadx_dcache_invalidate_by_addr(g_thermal_ai_output_buffer, LL_Buffer_len(&g_thermal_ai_output_info[0]));
  g_thermal_ai_diag_output_byte0 = (uint8_t)g_thermal_ai_output_buffer[0];
  (void)memset(&result, 0, sizeof(result));
  result.frame_counter = frame_counter;
  result.max_temp_centi_c = (uint16_t)app_threadx_temp14_to_centi_celsius(max_temp14);
  result.ambient_temp_centi_c = (uint16_t)app_threadx_temp14_to_centi_celsius(background_temp14);
  app_threadx_thermal_ai_set_diag_stage(12U, 0U);
  app_threadx_thermal_ai_decode_output(&result);

  if (tx_mutex_get(&g_thermal_ai_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
  {
    thermal_ai_runtime_update(&g_thermal_ai_runtime, &result);
    g_thermal_ai_model_ready = 1U;
    g_thermal_ai_diag_stage = 0U;
    tx_mutex_put(&g_thermal_ai_mutex);
  }
}

/**
  * @brief  Return one LVGL color for the requested AI class.
  * @param  class_id Detection class identifier.
  * @retval uint16_t RGB565-compatible LVGL color value.
  */
static lv_color_t app_threadx_thermal_ai_color_for_class(uint8_t class_id)
{
  switch ((thermal_ai_class_t)class_id)
  {
    case THERMAL_AI_CLASS_PERSON:
      return lv_palette_main(LV_PALETTE_RED);
    case THERMAL_AI_CLASS_CIRCUIT_BOARD_NORMAL:
      return lv_palette_main(LV_PALETTE_BLUE);
    case THERMAL_AI_CLASS_CIRCUIT_BOARD_ABNORMAL_HOTSPOT:
      return lv_palette_main(LV_PALETTE_YELLOW);
    default:
      return lv_palette_main(LV_PALETTE_GREY);
  }
}

/**
  * @brief  Create runtime AI overlay rectangles on both preview images.
  * @retval None
  */
static void app_threadx_gui_init_ai_overlays(void)
{
  if (g_thermal_ai_overlay_ready != 0U)
  {
    return;
  }

  app_threadx_gui_init_ai_overlay_set(guider_ui.WidgetsDemo_preview_img,
                                      g_thermal_ai_preview_boxes,
                                      g_thermal_ai_preview_labels);
  app_threadx_gui_init_ai_overlay_set(guider_ui.WidgetsDemo_fullscreen_preview_img,
                                      g_thermal_ai_fullscreen_boxes,
                                      g_thermal_ai_fullscreen_labels);
  g_thermal_ai_overlay_ready = 1U;
}

/**
  * @brief  Create one set of AI overlay rectangles for one preview image.
  * @param  parent_img Parent preview image object.
  * @param  box_array Output box-object array.
  * @param  label_array Output label-object array.
  * @retval None
  */
static void app_threadx_gui_init_ai_overlay_set(lv_obj_t *parent_img,
                                                lv_obj_t **box_array,
                                                lv_obj_t **label_array)
{
  uint32_t detection_index;

  if ((parent_img == NULL) || (box_array == NULL) || (label_array == NULL))
  {
    return;
  }

  for (detection_index = 0U; detection_index < CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS; detection_index++)
  {
    lv_obj_t *box = lv_obj_create(parent_img);
    lv_obj_t *label = lv_label_create(box);

    box_array[detection_index] = box;
    label_array[detection_index] = label;

    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 2, 0);
    lv_obj_set_style_border_color(box, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(box, 0, 0);
    lv_obj_add_flag(box, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_style_bg_opa(label, LV_OPA_60, 0);
    lv_obj_set_style_bg_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
  }
}

/**
  * @brief  Update one set of AI overlay rectangles from the latest decoded detections.
  * @param  box_array Box-object array.
  * @param  label_array Label-object array.
  * @param  preview_width Target preview width in pixels.
  * @param  preview_height Target preview height in pixels.
  * @param  result_ptr Latest decoded AI result, may be NULL.
  * @retval None
  */
static void app_threadx_gui_update_ai_overlay_set(lv_obj_t **box_array,
                                                  lv_obj_t **label_array,
                                                  uint16_t preview_width,
                                                  uint16_t preview_height,
                                                  const thermal_ai_result_t *result_ptr)
{
  uint32_t detection_index;

  if ((box_array == NULL) || (label_array == NULL))
  {
    return;
  }

  for (detection_index = 0U; detection_index < CFG_THERMAL_AI_RUNTIME_MAX_DETECTIONS; detection_index++)
  {
    lv_obj_t *box = box_array[detection_index];
    lv_obj_t *label = label_array[detection_index];

    if ((box == NULL) || (label == NULL))
    {
      continue;
    }

    if ((result_ptr == NULL) ||
        (detection_index >= result_ptr->detection_count) ||
        (result_ptr->detections[detection_index].valid == 0U))
    {
      lv_obj_add_flag(box, LV_OBJ_FLAG_HIDDEN);
      continue;
    }

    {
      thermal_ai_bbox_t scaled_bbox;
      const thermal_ai_detection_t *detection_ptr = &result_ptr->detections[detection_index];
      char label_text[32];
      lv_color_t border_color = app_threadx_thermal_ai_color_for_class(detection_ptr->class_id);

      thermal_ai_runtime_scale_bbox(&detection_ptr->bbox,
                                    CFG_THERMAL_AI_INPUT_WIDTH,
                                    CFG_THERMAL_AI_INPUT_HEIGHT,
                                    preview_width,
                                    preview_height,
                                    &scaled_bbox);
      lv_obj_clear_flag(box, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_pos(box, scaled_bbox.x_min, scaled_bbox.y_min);
      lv_obj_set_size(box,
                      (lv_coord_t)((scaled_bbox.x_max > scaled_bbox.x_min) ? (scaled_bbox.x_max - scaled_bbox.x_min) : 1U),
                      (lv_coord_t)((scaled_bbox.y_max > scaled_bbox.y_min) ? (scaled_bbox.y_max - scaled_bbox.y_min) : 1U));
      lv_obj_set_style_border_color(box, border_color, 0);
      (void)snprintf(label_text,
                     sizeof(label_text),
                     "%s %u%%",
                     thermal_ai_runtime_get_class_name(detection_ptr->class_id),
                     (unsigned int)((detection_ptr->confidence_permille + 5U) / 10U));
      lv_label_set_text(label, label_text);
      lv_obj_set_style_border_color(label, border_color, 0);
    }
  }
}

/**
  * @brief  Thermal rendering thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_thermal_thread_entry(ULONG thread_input)
{
  uint32_t ai_frame_counter_last = 0U;
  ULONG ai_tick_last = 0U;

  TX_PARAMETER_NOT_USED(thread_input);

  while (tiny1c_thermal_app_start() != IR_SUCCESS)
  {
    g_tiny1c_app_started = 0U;
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }

  {
    float probe_temp_c = 0.0f;

    (void)app_threadx_libirtemp_temp_correct(0.95f, 16384U, 25.0f, 30.0f, &probe_temp_c);
    g_libirtemp_probe_sink_c = probe_temp_c;
  }

  g_tiny1c_app_started = 1U;

  for (;;)
  {
    uint32_t frame_counter_now;
    const uint16_t *temp14_frame;

    tiny1c_thermal_app_process();
    frame_counter_now = tiny1c_thermal_app_get_frame_counter();
    if ((frame_counter_now != 0U) && (frame_counter_now != ai_frame_counter_last))
    {
      temp14_frame = tiny1c_thermal_app_get_temp14_frame();
      if ((CFG_THERMAL_AI_ENABLE != 0U) &&
          (temp14_frame != NULL) &&
          ((HAL_GetTick() - ai_tick_last) >= CFG_THERMAL_AI_MIN_INTERVAL_MS))
      {
        app_threadx_thermal_ai_run_inference(temp14_frame, frame_counter_now);
        ai_frame_counter_last = frame_counter_now;
        ai_tick_last = HAL_GetTick();
      }
    }
    tx_thread_sleep(1U);
  }
}

/**
  * @brief  Low-priority extrema query thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_extrema_query_thread_entry(ULONG thread_input)
{
  MaxMinTempInfo_t max_min_temp_info;
  uint16_t max_temp14;
  uint16_t min_temp14;
  uint16_t center_temp14;
  IrPoint_t center_point;

  TX_PARAMETER_NOT_USED(thread_input);

  for (;;)
  {
    if (g_tiny1c_app_started != 0U)
    {
      if (tiny1c_vdcmd_get_frame_max_min_temp(&max_min_temp_info) == IR_SUCCESS)
      {
        max_temp14 = max_min_temp_info.max_temp;
        min_temp14 = max_min_temp_info.min_temp;
        (void)tiny1c_vdcmd_get_point_temp(max_min_temp_info.max_temp_point, &max_temp14);
        (void)tiny1c_vdcmd_get_point_temp(max_min_temp_info.min_temp_point, &min_temp14);
        g_extrema_cache_max_temp14 = max_temp14;
        g_extrema_cache_min_temp14 = min_temp14;
        g_extrema_cache_max_temp_x = max_min_temp_info.max_temp_point.x;
        g_extrema_cache_max_temp_y = max_min_temp_info.max_temp_point.y;
        g_extrema_cache_min_temp_x = max_min_temp_info.min_temp_point.x;
        g_extrema_cache_min_temp_y = max_min_temp_info.min_temp_point.y;
        g_extrema_cache_valid = 1U;
      }

      center_point = app_threadx_build_center_point(tiny1c_thermal_app_get_frame_width(),
                                                    tiny1c_thermal_app_get_frame_height());
      if (tiny1c_vdcmd_get_point_temp(center_point, &center_temp14) == IR_SUCCESS)
      {
        tiny1c_thermal_app_set_center_temp_centi_c(app_threadx_temp14_to_compensated_centi_celsius(center_temp14));
      }
    }

    tx_thread_sleep((CFG_EXTREMA_QUERY_PERIOD_TICKS > 0U) ? CFG_EXTREMA_QUERY_PERIOD_TICKS : 1U);
  }
}

/**
  * @brief  Low-priority battery polling thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_battery_thread_entry(ULONG thread_input)
{
  uint8_t battery_initialized = 0U;

  TX_PARAMETER_NOT_USED(thread_input);

  (void)memset(&g_battery_device, 0, sizeof(g_battery_device));

  for (;;)
  {
    HAL_StatusTypeDef hal_status = HAL_ERROR;

    if (app_i2c4_bus_lock() == TX_SUCCESS)
    {
      if (battery_initialized == 0U)
      {
        hal_status = BQ27441_Init(&g_battery_device,
                                  &hi2c4,
                                  BQ27441_DEFAULT_ADDR,
                                  CFG_BATTERY_I2C_TIMEOUT_MS);
        if (hal_status == HAL_OK)
        {
          battery_initialized = 1U;
          hal_status = BQ27441_Update(&g_battery_device);
        }
      }
      else
      {
        hal_status = BQ27441_Update(&g_battery_device);
      }

      app_i2c4_bus_unlock();
    }

    if (hal_status == HAL_OK)
    {
      g_battery_cache_percent = g_battery_device.soc;
      g_battery_cache_charge_state = (uint8_t)g_battery_device.charge_status;
      g_battery_cache_valid = 1U;
    }
    else if (battery_initialized == 0U)
    {
      g_battery_cache_valid = 0U;
    }

    tx_thread_sleep((CFG_BATTERY_POLL_PERIOD_TICKS > 0U) ? CFG_BATTERY_POLL_PERIOD_TICKS : 1U);
  }
}

/**
  * @brief  GUI rendering thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_gui_thread_entry(ULONG thread_input)
{
  uint16_t preview_width;
  uint16_t preview_height;
  uint32_t frame_counter_last = 0U;
  uint32_t overlay_update_tick_last = 0U;

  TX_PARAMETER_NOT_USED(thread_input);

  rgblcd_init();
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  setup_ui(&guider_ui);
  events_init(&guider_ui);
  custom_init(&guider_ui);
  lv_obj_invalidate(lv_scr_act());
  lv_refr_now(NULL);

  preview_width = tiny1c_thermal_app_get_preview_width();
  preview_height = tiny1c_thermal_app_get_preview_height();
  g_gui_preview_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
  g_gui_preview_img_dsc.header.always_zero = 0U;
  g_gui_preview_img_dsc.header.w = preview_width;
  g_gui_preview_img_dsc.header.h = preview_height;
  g_gui_preview_img_dsc.data_size = (uint32_t)preview_width * (uint32_t)preview_height * sizeof(uint16_t);
  g_gui_preview_img_dsc.data = (const uint8_t *)tiny1c_thermal_app_get_rgb565_frame();
  lv_img_set_src(guider_ui.WidgetsDemo_preview_img, &g_gui_preview_img_dsc);

  g_gui_fullscreen_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
  g_gui_fullscreen_img_dsc.header.always_zero = 0U;
  g_gui_fullscreen_img_dsc.header.w = CFG_GUI_FULLSCREEN_WIDTH;
  g_gui_fullscreen_img_dsc.header.h = CFG_GUI_FULLSCREEN_HEIGHT;
  g_gui_fullscreen_img_dsc.data_size = CFG_GUI_FULLSCREEN_WIDTH * CFG_GUI_FULLSCREEN_HEIGHT * sizeof(uint16_t);
  g_gui_fullscreen_img_dsc.data = (const uint8_t *)g_gui_fullscreen_rgb565_frame;
  lv_img_set_src(guider_ui.WidgetsDemo_fullscreen_preview_img, &g_gui_fullscreen_img_dsc);
  if (CFG_THERMAL_AI_ENABLE != 0U)
  {
    app_threadx_gui_init_ai_overlays();
  }

  for (;;)
  {
    uint32_t frame_counter_now;
    char time_text[16];
    char battery_text[16];
    char ai_text[24];
    uint32_t seconds;

    frame_counter_now = tiny1c_thermal_app_get_frame_counter();
    if ((frame_counter_now != 0U) && (frame_counter_now != frame_counter_last))
    {
      frame_counter_last = frame_counter_now;
      lv_obj_invalidate(guider_ui.WidgetsDemo_preview_img);
      if (thermal_gui_is_fullscreen_active() != 0U)
      {
        app_threadx_gui_update_fullscreen_image();
        lv_obj_invalidate(guider_ui.WidgetsDemo_fullscreen_preview_img);
      }
    }

    if ((HAL_GetTick() - overlay_update_tick_last) >= CFG_GUI_OVERLAY_UPDATE_PERIOD_MS)
    {
      app_threadx_gui_update_preview(&guider_ui);
      overlay_update_tick_last = HAL_GetTick();
    }

    seconds = HAL_GetTick() / 1000U;
    (void)snprintf(time_text, sizeof(time_text), "%02lu:%02lu",
                   (unsigned long)((seconds / 60U) % 60U),
                   (unsigned long)(seconds % 60U));
    app_threadx_gui_set_badge_text(guider_ui.WidgetsDemo_status_time, time_text);
    app_threadx_gui_format_battery_text(battery_text, sizeof(battery_text));
    app_threadx_gui_set_badge_text(guider_ui.WidgetsDemo_status_power, battery_text);
    app_threadx_gui_format_ai_status_text(ai_text, sizeof(ai_text));
    app_threadx_gui_set_badge_text(guider_ui.WidgetsDemo_status_uart, ai_text);

    lv_timer_handler();
    tx_thread_sleep(5U);
  }
}

/**
  * @brief  Report whether any owner currently holds UART file mode.
  * @retval uint8_t 1 when file mode is active, otherwise 0.
  */
static uint8_t app_threadx_uart_file_mode_active_internal(void)
{
  return (g_uart_file_hold_mask != 0U) ? 1U : 0U;
}

/**
  * @brief  Set one UART file-mode hold bit.
  * @param  hold_mask Owner bit mask.
  * @retval None
  */
static void app_threadx_uart_file_hold_set(uint32_t hold_mask)
{
  app_uart_request_file_mode(hold_mask);
}

/**
  * @brief  Clear one UART file-mode hold bit.
  * @param  hold_mask Owner bit mask.
  * @retval None
  */
static void app_threadx_uart_file_hold_clear(uint32_t hold_mask)
{
  app_uart_release_file_mode(hold_mask);
}

/**
  * @brief  Wait until any in-flight UART DMA transmission completes.
  * @retval None
  */
static void app_threadx_uart_wait_for_tx_idle(void)
{
  while (g_uart_stream_tx_busy != 0U)
  {
    tx_thread_sleep(1U);
  }
}

/**
  * @brief  Send one text response over USART1 using blocking mode.
  * @param  text_ptr Null-terminated ASCII text.
  * @retval UINT HAL/FileX-style status code.
  */
static UINT app_threadx_uart_send_text(const char *text_ptr)
{
  HAL_StatusTypeDef hal_status;
  uint32_t text_length;

  if (text_ptr == NULL)
  {
    return FX_PTR_ERROR;
  }

  text_length = (uint32_t)strlen(text_ptr);
  if (text_length == 0U)
  {
    return FX_SUCCESS;
  }

  app_threadx_uart_wait_for_tx_idle();
  g_uart_stream_tx_busy = 1U;
  hal_status = HAL_UART_Transmit(&huart1, (uint8_t *)(uintptr_t)text_ptr, (uint16_t)text_length, 5000U);
  g_uart_stream_tx_busy = 0U;
  return (hal_status == HAL_OK) ? FX_SUCCESS : FX_IO_ERROR;
}

/**
  * @brief  Format and send one text response over USART1.
  * @param  format_ptr Printf-style format string.
  * @retval UINT HAL/FileX-style status code.
  */
static UINT app_threadx_uart_send_text_fmt(const char *format_ptr, ...)
{
  int text_length;
  va_list args;

  if (format_ptr == NULL)
  {
    return FX_PTR_ERROR;
  }

  va_start(args, format_ptr);
  text_length = vsnprintf((char *)g_uart_command_tx_line, sizeof(g_uart_command_tx_line), format_ptr, args);
  va_end(args);
  if ((text_length < 0) || ((uint32_t)text_length >= sizeof(g_uart_command_tx_line)))
  {
    return FX_BUFFER_ERROR;
  }

  return app_threadx_uart_send_text((const char *)g_uart_command_tx_line);
}

/**
  * @brief  Encode one binary chunk into Base64 text.
  * @param  input_ptr Source byte buffer.
  * @param  input_size Number of source bytes.
  * @param  output_ptr Output text buffer.
  * @param  output_capacity Size of @p output_ptr in bytes.
  * @retval uint32_t Number of output bytes written without terminator.
  */
static uint32_t app_threadx_base64_encode(const uint8_t *input_ptr,
                                          uint32_t input_size,
                                          char *output_ptr,
                                          uint32_t output_capacity)
{
  static const char g_base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uint32_t input_index = 0U;
  uint32_t output_index = 0U;

  if ((input_ptr == NULL) || (output_ptr == NULL))
  {
    return 0U;
  }

  while (input_index < input_size)
  {
    uint32_t remain = input_size - input_index;
    uint8_t b0 = input_ptr[input_index++];
    uint8_t b1 = (remain > 1U) ? input_ptr[input_index++] : 0U;
    uint8_t b2 = (remain > 2U) ? input_ptr[input_index++] : 0U;

    if ((output_index + 4U) >= output_capacity)
    {
      return 0U;
    }

    output_ptr[output_index++] = g_base64_table[(b0 >> 2) & 0x3FU];
    output_ptr[output_index++] = g_base64_table[((b0 & 0x03U) << 4) | ((b1 >> 4) & 0x0FU)];
    output_ptr[output_index++] = (remain > 1U) ? g_base64_table[((b1 & 0x0FU) << 2) | ((b2 >> 6) & 0x03U)] : '=';
    output_ptr[output_index++] = (remain > 2U) ? g_base64_table[b2 & 0x3FU] : '=';
  }

  output_ptr[output_index] = '\0';
  return output_index;
}

/**
  * @brief  Send the current snapshot file list over the serial file protocol.
  * @retval UINT FileX status code.
  */
static UINT app_threadx_uart_send_snapshot_list(void)
{
  UINT status;
  uint32_t entry_count = 0U;
  uint32_t entry_index;

  status = app_filex_snapshot_get_list((CHAR *)g_uart_file_snapshot_names,
                                       CFG_UART_FILE_MAX_LISTED_SNAPSHOTS,
                                       CFG_UART_FILE_NAME_STRIDE,
                                       &entry_count);
  if (status != FX_SUCCESS)
  {
    return status;
  }

  status = app_threadx_uart_send_text_fmt("LIST %lu\n", (unsigned long)entry_count);
  if (status != FX_SUCCESS)
  {
    return status;
  }

  for (entry_index = 0U; entry_index < entry_count; entry_index++)
  {
    status = app_threadx_uart_send_text_fmt("NAME %s\n", g_uart_file_snapshot_names[entry_index]);
    if (status != FX_SUCCESS)
    {
      return status;
    }
  }

  return app_threadx_uart_send_text("LIST_END\n");
}

/**
  * @brief  Send one named snapshot as raw RGB565 chunks over the serial file protocol.
  * @param  file_name_ptr Snapshot file name in THMxxxxx.BMP format.
  * @retval UINT FileX/HAL status code.
  */
static UINT app_threadx_uart_send_snapshot_named(const CHAR *file_name_ptr)
{
  UINT status;
  uint16_t width = 0U;
  uint16_t height = 0U;
  uint32_t total_bytes;
  const uint8_t *payload_ptr;
  uint32_t payload_offset = 0U;
  uint32_t sequence = 0U;

  if (file_name_ptr == NULL)
  {
    return FX_PTR_ERROR;
  }

  status = app_filex_snapshot_load_rgb565(file_name_ptr,
                                          g_uart_file_rgb565_frame,
                                          CFG_UART_FILE_MAX_PIXELS,
                                          &width,
                                          &height);
  if (status != FX_SUCCESS)
  {
    return status;
  }

  total_bytes = (uint32_t)width * (uint32_t)height * sizeof(uint16_t);
  payload_ptr = (const uint8_t *)(const void *)g_uart_file_rgb565_frame;

  status = app_threadx_uart_send_text_fmt("FILE_BEGIN %s %u %u %lu\n",
                                          file_name_ptr,
                                          (unsigned int)width,
                                          (unsigned int)height,
                                          (unsigned long)total_bytes);
  if (status != FX_SUCCESS)
  {
    return status;
  }

  while (payload_offset < total_bytes)
  {
    uint32_t chunk_bytes = total_bytes - payload_offset;
    uint32_t encoded_bytes;

    if (chunk_bytes > CFG_UART_FILE_BASE64_CHUNK_BYTES)
    {
      chunk_bytes = CFG_UART_FILE_BASE64_CHUNK_BYTES;
    }

    encoded_bytes = app_threadx_base64_encode(&payload_ptr[payload_offset],
                                              chunk_bytes,
                                              (char *)g_uart_file_chunk_base64,
                                              sizeof(g_uart_file_chunk_base64));
    if (encoded_bytes == 0U)
    {
      return FX_BUFFER_ERROR;
    }

    status = app_threadx_uart_send_text_fmt("DATA %lu %s\n",
                                            (unsigned long)sequence,
                                            g_uart_file_chunk_base64);
    if (status != FX_SUCCESS)
    {
      return status;
    }

    payload_offset += chunk_bytes;
    sequence++;
  }

  return app_threadx_uart_send_text("FILE_END\n");
}

/**
  * @brief  Send the latest available snapshot over the serial file protocol.
  * @retval UINT FileX/HAL status code.
  */
static UINT app_threadx_uart_send_snapshot_latest(void)
{
  UINT status;
  uint32_t entry_count = 0U;

  status = app_filex_snapshot_get_list((CHAR *)g_uart_file_snapshot_names,
                                       CFG_UART_FILE_MAX_LISTED_SNAPSHOTS,
                                       CFG_UART_FILE_NAME_STRIDE,
                                       &entry_count);
  if (status != FX_SUCCESS)
  {
    return status;
  }
  if (entry_count == 0U)
  {
    return FX_NO_MORE_ENTRIES;
  }

  return app_threadx_uart_send_snapshot_named(g_uart_file_snapshot_names[entry_count - 1U]);
}

/**
  * @brief  Process one ASCII file-protocol command line.
  * @param  command_line_ptr Null-terminated command text.
  * @retval None
  */
static void app_threadx_uart_process_command_line(const uint8_t *command_line_ptr)
{
  UINT status = FX_SUCCESS;
  const char *command_ptr = (const char *)command_line_ptr;
  const char *actual_mode_text = NULL;

  if ((command_line_ptr == NULL) || (command_line_ptr[0] == '\0'))
  {
    return;
  }

  if (strcmp(command_ptr, "FILE_ENTER") == 0)
  {
    app_threadx_uart_file_hold_set(APP_UART_FILE_HOLD_WEB);
    status = app_filex_prepare();
    if (status == FX_SUCCESS)
    {
      actual_mode_text = (app_threadx_uart_file_mode_active_internal() != 0U) ? "FILE_MODE" : "STREAM_MODE";
      (void)app_threadx_uart_send_text_fmt("OK %s\n", actual_mode_text);
    }
    else
    {
      app_threadx_uart_file_hold_clear(APP_UART_FILE_HOLD_WEB);
      (void)app_threadx_uart_send_text_fmt("ERR %s\n", app_filex_status_to_string(status));
    }
    return;
  }

  if (strcmp(command_ptr, "FILE_EXIT") == 0)
  {
    app_threadx_uart_file_hold_clear(APP_UART_FILE_HOLD_WEB);
    actual_mode_text = (app_threadx_uart_file_mode_active_internal() != 0U) ? "FILE_MODE" : "STREAM_MODE";
    (void)app_threadx_uart_send_text_fmt("OK %s\n", actual_mode_text);
    return;
  }

  if ((strcmp(command_ptr, "FILE_LIST") == 0) || (strcmp(command_ptr, "FILE_GET_LATEST") == 0) ||
      (strncmp(command_ptr, "FILE_GET ", 9) == 0))
  {
    app_threadx_uart_file_hold_set(APP_UART_FILE_HOLD_WEB);
    g_uart_web_file_hold_expire_tick = tx_time_get() + ((CFG_UART_FILE_WEB_TIMEOUT_TICKS > 0U) ? CFG_UART_FILE_WEB_TIMEOUT_TICKS : 1U);
  }

  if (strcmp(command_ptr, "FILE_LIST") == 0)
  {
    status = app_threadx_uart_send_snapshot_list();
  }
  else if (strcmp(command_ptr, "FILE_GET_LATEST") == 0)
  {
    status = app_threadx_uart_send_snapshot_latest();
  }
  else if (strncmp(command_ptr, "FILE_GET ", 9) == 0)
  {
    status = app_threadx_uart_send_snapshot_named((const CHAR *)(command_ptr + 9));
  }
  else
  {
    status = FX_INVALID_PATH;
  }

  if (status != FX_SUCCESS)
  {
    (void)app_threadx_uart_send_text_fmt("ERR %s\n", app_filex_status_to_string(status));
  }
}

/**
  * @brief  UART command thread entry for gallery/file transfer commands.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_uart_command_thread_entry(ULONG thread_input)
{
  uint32_t command_length = 0U;

  TX_PARAMETER_NOT_USED(thread_input);

  for (;;)
  {
    uint8_t rx_byte = 0U;
    HAL_StatusTypeDef hal_status;
    ULONG tick_now;

    if ((g_uart_file_hold_mask & APP_UART_FILE_HOLD_WEB) != 0U)
    {
      tick_now = tx_time_get();
      if (((int32_t)(tick_now - g_uart_web_file_hold_expire_tick) >= 0) && (g_uart_web_file_hold_expire_tick != 0U))
      {
        app_threadx_uart_file_hold_clear(APP_UART_FILE_HOLD_WEB);
      }
    }

    hal_status = HAL_UART_Receive(&huart1, &rx_byte, 1U, CFG_UART_COMMAND_RX_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
      tx_thread_sleep(1U);
      continue;
    }

    if (rx_byte == '\r')
    {
      continue;
    }

    if (rx_byte == '\n')
    {
      g_uart_command_rx_line[command_length] = '\0';
      if (command_length > 0U)
      {
        app_threadx_uart_process_command_line(g_uart_command_rx_line);
      }
      command_length = 0U;
      continue;
    }

    if (command_length < (sizeof(g_uart_command_rx_line) - 1U))
    {
      g_uart_command_rx_line[command_length++] = rx_byte;
    }
    else
    {
      command_length = 0U;
      (void)app_threadx_uart_send_text("ERR Buffer\n");
    }
  }
}

/**
  * @brief  UART streaming thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_uart_stream_thread_entry(ULONG thread_input)
{
  uint32_t frame_counter_last = 0U;

  TX_PARAMETER_NOT_USED(thread_input);

  for (;;)
  {
    uint32_t frame_counter_now;
    const uint16_t *temp14_frame;
    uint16_t center_temp14;
    int32_t center_temp_centi_c;
    float corrected_center_temp_c;
    uint32_t tx_bytes;
    IrPoint_t center_point;

    if (g_tiny1c_app_started == 0U)
    {
      tx_thread_sleep(10U);
      continue;
    }

    if (app_threadx_uart_file_mode_active_internal() != 0U)
    {
      tx_thread_sleep(10U);
      continue;
    }

    if (g_uart_stream_tx_busy != 0U)
    {
      tx_thread_sleep(1U);
      continue;
    }

    frame_counter_now = tiny1c_thermal_app_get_frame_counter();
    if ((frame_counter_now == 0U) || (frame_counter_now == frame_counter_last))
    {
      tx_thread_sleep(CFG_UART_STREAM_PERIOD_TICKS);
      continue;
    }

    temp14_frame = tiny1c_thermal_app_get_temp14_frame();
    if (temp14_frame == NULL)
    {
      tx_thread_sleep(CFG_UART_STREAM_PERIOD_TICKS);
      continue;
    }

    center_temp14 = temp14_frame[((CFG_UART_STREAM_SOURCE_HEIGHT / 2U) * CFG_UART_STREAM_SOURCE_WIDTH) +
                                 (CFG_UART_STREAM_SOURCE_WIDTH / 2U)];
    center_point = app_threadx_build_center_point(CFG_UART_STREAM_SOURCE_WIDTH, CFG_UART_STREAM_SOURCE_HEIGHT);
    (void)tiny1c_vdcmd_get_point_temp(center_point, &center_temp14);
    center_temp_centi_c = app_threadx_temp14_to_centi_celsius(center_temp14);
    corrected_center_temp_c = (float)center_temp_centi_c / 100.0f;

    if (app_threadx_libirtemp_temp_correct(CFG_LIBIRTEMP_TEST_EMISSIVITY,
                                           CFG_LIBIRTEMP_TEST_TAU_Q14,
                                           CFG_LIBIRTEMP_TEST_AMBIENT_TEMP_C,
                                           corrected_center_temp_c,
                                           &corrected_center_temp_c) == 0)
    {
      tiny1c_thermal_app_set_center_temp_centi_c(app_threadx_float_to_centi_celsius(corrected_center_temp_c));
    }
    else
    {
      tiny1c_thermal_app_set_center_temp_centi_c(center_temp_centi_c);
    }

    tx_bytes = app_threadx_uart_stream_build_packet(g_uart_stream_tx_buffer, temp14_frame, frame_counter_now);
    if (tx_bytes == 0U)
    {
      tx_thread_sleep(CFG_UART_STREAM_PERIOD_TICKS);
      continue;
    }

    app_threadx_uart_stream_clean_dcache(g_uart_stream_tx_buffer, tx_bytes);
    if (HAL_UART_Transmit_DMA(&huart1, g_uart_stream_tx_buffer, tx_bytes) == HAL_OK)
    {
      g_uart_stream_tx_busy = 1U;
      frame_counter_last = frame_counter_now;
    }

    tx_thread_sleep(CFG_UART_STREAM_PERIOD_TICKS);
  }
}

/**
  * @brief  Build one downsampled temperature packet for UART transmission.
  * @param  tx_buffer Pointer to the transmission buffer.
  * @param  temp14_frame Pointer to the latest 14-bit temperature frame.
  * @param  frame_counter Processed frame sequence number.
  * @retval uint32_t Total packet length in bytes.
  */
static uint32_t app_threadx_uart_stream_build_packet(uint8_t *tx_buffer, const uint16_t *temp14_frame, uint32_t frame_counter)
{
  app_threadx_uart_stream_header_t *header_ptr;
  uint16_t *payload_ptr;
  uint32_t y_out;
  uint32_t x_out;
  uint16_t min_temp14 = 0xFFFFU;
  uint16_t max_temp14 = 0U;
  uint16_t center_temp14;

  if ((tx_buffer == NULL) || (temp14_frame == NULL))
  {
    return 0U;
  }

  header_ptr = (app_threadx_uart_stream_header_t *)tx_buffer;
  payload_ptr = (uint16_t *)(void *)(tx_buffer + sizeof(app_threadx_uart_stream_header_t));
  center_temp14 = 0U;

  for (y_out = 0U; y_out < CFG_UART_STREAM_FRAME_HEIGHT; y_out++)
  {
    uint32_t y_base = y_out * CFG_UART_STREAM_DOWNSAMPLE_STEP_Y;

    for (x_out = 0U; x_out < CFG_UART_STREAM_FRAME_WIDTH; x_out++)
    {
      uint32_t x_base = x_out * CFG_UART_STREAM_DOWNSAMPLE_STEP_X;
      uint32_t y_inner;
      uint32_t x_inner;
      uint32_t sum_temp14 = 0U;
      uint16_t temp14_value;

      for (y_inner = 0U; y_inner < CFG_UART_STREAM_DOWNSAMPLE_STEP_Y; y_inner++)
      {
        for (x_inner = 0U; x_inner < CFG_UART_STREAM_DOWNSAMPLE_STEP_X; x_inner++)
        {
          sum_temp14 += app_threadx_temp14_get_preview_oriented_sample(
              temp14_frame,
              CFG_UART_STREAM_SOURCE_WIDTH,
              CFG_UART_STREAM_SOURCE_HEIGHT,
              (uint16_t)(x_base + x_inner),
              (uint16_t)(y_base + y_inner));
        }
      }

      temp14_value = (uint16_t)(sum_temp14 / (CFG_UART_STREAM_DOWNSAMPLE_STEP_X * CFG_UART_STREAM_DOWNSAMPLE_STEP_Y));
      payload_ptr[(y_out * CFG_UART_STREAM_FRAME_WIDTH) + x_out] = temp14_value;

      if (temp14_value < min_temp14)
      {
        min_temp14 = temp14_value;
      }

      if (temp14_value > max_temp14)
      {
        max_temp14 = temp14_value;
      }
    }
  }

  center_temp14 = payload_ptr[((CFG_UART_STREAM_FRAME_HEIGHT / 2U) * CFG_UART_STREAM_FRAME_WIDTH) +
                              (CFG_UART_STREAM_FRAME_WIDTH / 2U)];

  header_ptr->sync_word = CFG_UART_STREAM_SYNC_WORD;
  header_ptr->packet_type = CFG_UART_STREAM_PACKET_TYPE_TEMP14;
  header_ptr->frame_counter = frame_counter;
  header_ptr->frame_width = CFG_UART_STREAM_FRAME_WIDTH;
  header_ptr->frame_height = CFG_UART_STREAM_FRAME_HEIGHT;
  header_ptr->payload_bytes = (uint16_t)CFG_UART_STREAM_PAYLOAD_BYTES;
  header_ptr->center_temp14 = center_temp14;
  header_ptr->min_temp14 = min_temp14;
  header_ptr->max_temp14 = max_temp14;

  return (uint32_t)(sizeof(app_threadx_uart_stream_header_t) + CFG_UART_STREAM_PAYLOAD_BYTES);
}

/**
  * @brief  Clean D-Cache for a DMA transmission buffer.
  * @param  buffer_addr Buffer start address.
  * @param  buffer_size Buffer length in bytes.
  * @retval None
  */
static void app_threadx_uart_stream_clean_dcache(void *buffer_addr, uint32_t buffer_size)
{
  app_threadx_dcache_clean_by_addr(buffer_addr, buffer_size);
}

/**
  * @brief  UART DMA transmission complete callback.
  * @param  uart_handle Pointer to the UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *uart_handle)
{
  if ((uart_handle != NULL) && (uart_handle->Instance == USART1))
  {
    g_uart_stream_tx_busy = 0U;
  }
}

/**
  * @brief  UART error callback.
  * @param  uart_handle Pointer to the UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart_handle)
{
  if ((uart_handle != NULL) && (uart_handle->Instance == USART1))
  {
    g_uart_stream_tx_busy = 0U;
  }
}

/* USER CODE END 1 */
