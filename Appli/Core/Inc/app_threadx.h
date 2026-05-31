/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  MCD Application Team
  * @brief   ThreadX applicative header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_THREADX_H
#define __APP_THREADX_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
  APP_UART_FILE_HOLD_NONE = 0x00U,
  APP_UART_FILE_HOLD_GUI  = 0x01U,
  APP_UART_FILE_HOLD_WEB  = 0x02U
} app_uart_file_hold_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Main thread defines -------------------------------------------------------*/
/* USER CODE BEGIN MTD */

/* USER CODE END MTD */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);

/* USER CODE BEGIN EFP */
void app_uart_request_file_mode(uint32_t hold_mask);
void app_uart_release_file_mode(uint32_t hold_mask);
uint8_t app_uart_is_file_mode_active(void);

/* USER CODE END EFP */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H */
