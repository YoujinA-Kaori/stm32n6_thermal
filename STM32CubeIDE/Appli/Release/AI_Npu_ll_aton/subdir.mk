################################################################################
# Manually maintained AI build rules.
################################################################################

C_SRCS += \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ecloader.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_cipher.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_dbgtrc.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_debug.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_lib.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_lib_sw_operators.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_osal_freertos.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_osal_threadx.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_osal_zephyr.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_profiler.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_reloc_callbacks.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_reloc_network.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_rt_main.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_runtime.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_aton_util.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_sw_float.c \
D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/ll_sw_integer.c

OBJS += \
./AI_Npu_ll_aton/ecloader.o \
./AI_Npu_ll_aton/ll_aton.o \
./AI_Npu_ll_aton/ll_aton_cipher.o \
./AI_Npu_ll_aton/ll_aton_dbgtrc.o \
./AI_Npu_ll_aton/ll_aton_debug.o \
./AI_Npu_ll_aton/ll_aton_lib.o \
./AI_Npu_ll_aton/ll_aton_lib_sw_operators.o \
./AI_Npu_ll_aton/ll_aton_osal_freertos.o \
./AI_Npu_ll_aton/ll_aton_osal_threadx.o \
./AI_Npu_ll_aton/ll_aton_osal_zephyr.o \
./AI_Npu_ll_aton/ll_aton_profiler.o \
./AI_Npu_ll_aton/ll_aton_reloc_callbacks.o \
./AI_Npu_ll_aton/ll_aton_reloc_network.o \
./AI_Npu_ll_aton/ll_aton_rt_main.o \
./AI_Npu_ll_aton/ll_aton_runtime.o \
./AI_Npu_ll_aton/ll_aton_util.o \
./AI_Npu_ll_aton/ll_sw_float.o \
./AI_Npu_ll_aton/ll_sw_integer.o

C_DEPS += \
./AI_Npu_ll_aton/ecloader.d \
./AI_Npu_ll_aton/ll_aton.d \
./AI_Npu_ll_aton/ll_aton_cipher.d \
./AI_Npu_ll_aton/ll_aton_dbgtrc.d \
./AI_Npu_ll_aton/ll_aton_debug.d \
./AI_Npu_ll_aton/ll_aton_lib.d \
./AI_Npu_ll_aton/ll_aton_lib_sw_operators.d \
./AI_Npu_ll_aton/ll_aton_osal_freertos.d \
./AI_Npu_ll_aton/ll_aton_osal_threadx.d \
./AI_Npu_ll_aton/ll_aton_osal_zephyr.d \
./AI_Npu_ll_aton/ll_aton_profiler.d \
./AI_Npu_ll_aton/ll_aton_reloc_callbacks.d \
./AI_Npu_ll_aton/ll_aton_reloc_network.d \
./AI_Npu_ll_aton/ll_aton_rt_main.d \
./AI_Npu_ll_aton/ll_aton_runtime.d \
./AI_Npu_ll_aton/ll_aton_util.d \
./AI_Npu_ll_aton/ll_sw_float.d \
./AI_Npu_ll_aton/ll_sw_integer.d

AI_RELEASE_INCLUDES := -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton
AI_RELEASE_DEFINES := -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1

AI_Npu_ll_aton/%.o AI_Npu_ll_aton/%.su AI_Npu_ll_aton/%.cyclo: D:/PracticeProject/Stm32/stm32n6_thermal/Middlewares/ST/AI/Npu/ll_aton/%.c AI_Npu_ll_aton/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 $(AI_RELEASE_DEFINES) -c $(AI_RELEASE_INCLUDES) -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-AI_Npu_ll_aton

clean-AI_Npu_ll_aton:
	-$(RM) ./AI_Npu_ll_aton/*.cyclo ./AI_Npu_ll_aton/*.d ./AI_Npu_ll_aton/*.o ./AI_Npu_ll_aton/*.su

.PHONY: clean-AI_Npu_ll_aton
