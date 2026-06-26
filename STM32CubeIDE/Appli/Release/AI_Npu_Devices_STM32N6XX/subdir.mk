################################################################################
# Manually maintained AI build rules.
################################################################################

C_SRCS += \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/Devices/STM32N6XX/mcu_cache.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/Devices/STM32N6XX/npu_cache.c

OBJS += \
./AI_Npu_Devices_STM32N6XX/mcu_cache.o \
./AI_Npu_Devices_STM32N6XX/npu_cache.o

C_DEPS += \
./AI_Npu_Devices_STM32N6XX/mcu_cache.d \
./AI_Npu_Devices_STM32N6XX/npu_cache.d

AI_RELEASE_INCLUDES := -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton
AI_RELEASE_DEFINES := -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1

AI_Npu_Devices_STM32N6XX/%.o AI_Npu_Devices_STM32N6XX/%.su AI_Npu_Devices_STM32N6XX/%.cyclo: D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/Devices/STM32N6XX/%.c AI_Npu_Devices_STM32N6XX/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 $(AI_RELEASE_DEFINES) -c $(AI_RELEASE_INCLUDES) -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-AI_Npu_Devices_STM32N6XX

clean-AI_Npu_Devices_STM32N6XX:
	-$(RM) ./AI_Npu_Devices_STM32N6XX/*.cyclo ./AI_Npu_Devices_STM32N6XX/*.d ./AI_Npu_Devices_STM32N6XX/*.o ./AI_Npu_Devices_STM32N6XX/*.su

.PHONY: clean-AI_Npu_Devices_STM32N6XX
