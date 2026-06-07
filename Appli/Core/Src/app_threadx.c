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
#include "libirtemp.h"
#include "thermal_ai_runtime.h"
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CFG_THERMAL_THREAD_STACK_SIZE         4096U
#define CFG_THERMAL_THREAD_PRIORITY           10U
#define CFG_GUI_THREAD_STACK_SIZE             8192U
#define CFG_GUI_THREAD_PRIORITY               12U
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
static ULONG g_thermal_thread_stack[CFG_THERMAL_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_gui_thread_stack[CFG_GUI_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_uart_stream_thread_stack[CFG_UART_STREAM_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_uart_command_thread_stack[CFG_UART_COMMAND_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG g_extrema_query_thread_stack[CFG_EXTREMA_QUERY_THREAD_STACK_SIZE / sizeof(ULONG)];
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID app_threadx_thermal_thread_entry(ULONG thread_input);
static VOID app_threadx_gui_thread_entry(ULONG thread_input);
static VOID app_threadx_uart_stream_thread_entry(ULONG thread_input);
static VOID app_threadx_uart_command_thread_entry(ULONG thread_input);
static VOID app_threadx_extrema_query_thread_entry(ULONG thread_input);
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
static IrPoint_t app_threadx_build_center_point(uint16_t frame_width, uint16_t frame_height);
static uint16_t app_threadx_rgb565_average4(uint16_t c00,
                                            uint16_t c10,
                                            uint16_t c01,
                                            uint16_t c11);
static void app_threadx_gui_set_badge_text(lv_obj_t *badge, const char *text);
static void app_threadx_gui_update_preview(lv_ui *ui);
static void app_threadx_gui_update_fullscreen_image(void);
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
  * @brief  Refresh the preview panel from the latest thermal frame.
  * @param  ui UI context.
  * @retval None
  */
static void app_threadx_gui_update_preview(lv_ui *ui)
{
  const uint16_t *temp14_frame;
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
  * @brief  Thermal rendering thread entry.
  * @param  thread_input Unused thread input parameter.
  * @retval None
  */
static VOID app_threadx_thermal_thread_entry(ULONG thread_input)
{
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
    tiny1c_thermal_app_process();
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

  for (;;)
  {
    uint32_t frame_counter_now;
    char time_text[16];
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

      if ((HAL_GetTick() - overlay_update_tick_last) >= CFG_GUI_OVERLAY_UPDATE_PERIOD_MS)
      {
        app_threadx_gui_update_preview(&guider_ui);
        overlay_update_tick_last = HAL_GetTick();
      }
    }

    seconds = HAL_GetTick() / 1000U;
    (void)snprintf(time_text, sizeof(time_text), "%02lu:%02lu",
                   (unsigned long)((seconds / 60U) % 60U),
                   (unsigned long)(seconds % 60U));
    app_threadx_gui_set_badge_text(guider_ui.WidgetsDemo_status_time, time_text);

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
  uint8_t mirror_enable;
  uint8_t flip_enable;

  if ((tx_buffer == NULL) || (temp14_frame == NULL))
  {
    return 0U;
  }

  header_ptr = (app_threadx_uart_stream_header_t *)tx_buffer;
  payload_ptr = (uint16_t *)(void *)(tx_buffer + sizeof(app_threadx_uart_stream_header_t));
  mirror_enable = tiny1c_thermal_app_get_preview_mirror_enabled();
  flip_enable = tiny1c_thermal_app_get_preview_flip_enabled();
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
        uint32_t source_y = y_base + y_inner;
        uint32_t mapped_source_y = (flip_enable != 0U) ? ((CFG_UART_STREAM_SOURCE_HEIGHT - 1U) - source_y) : source_y;
        uint32_t row_index = mapped_source_y * CFG_UART_STREAM_SOURCE_WIDTH;

        for (x_inner = 0U; x_inner < CFG_UART_STREAM_DOWNSAMPLE_STEP_X; x_inner++)
        {
          uint32_t source_x = x_base + x_inner;
          uint32_t mapped_source_x = (mirror_enable != 0U) ? ((CFG_UART_STREAM_SOURCE_WIDTH - 1U) - source_x) : source_x;

          sum_temp14 += temp14_frame[row_index + mapped_source_x];
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
