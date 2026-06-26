################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/draw/nxp/vglite/lv_draw_vglite.c \
../lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.c \
../lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.c \
../lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.c \
../lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.c \
../lvgl/src/draw/nxp/vglite/lv_vglite_buf.c \
../lvgl/src/draw/nxp/vglite/lv_vglite_utils.c 

OBJS += \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite.o \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.o \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.o \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.o \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.o \
./lvgl/src/draw/nxp/vglite/lv_vglite_buf.o \
./lvgl/src/draw/nxp/vglite/lv_vglite_utils.o 

C_DEPS += \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite.d \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.d \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.d \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.d \
./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.d \
./lvgl/src/draw/nxp/vglite/lv_vglite_buf.d \
./lvgl/src/draw/nxp/vglite/lv_vglite_utils.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/draw/nxp/vglite/%.o lvgl/src/draw/nxp/vglite/%.su lvgl/src/draw/nxp/vglite/%.cyclo: ../lvgl/src/draw/nxp/vglite/%.c lvgl/src/draw/nxp/vglite/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_THREADX -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-src-2f-draw-2f-nxp-2f-vglite

clean-lvgl-2f-src-2f-draw-2f-nxp-2f-vglite:
	-$(RM) ./lvgl/src/draw/nxp/vglite/lv_draw_vglite.cyclo ./lvgl/src/draw/nxp/vglite/lv_draw_vglite.d ./lvgl/src/draw/nxp/vglite/lv_draw_vglite.o ./lvgl/src/draw/nxp/vglite/lv_draw_vglite.su ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.cyclo ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.d ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.o ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.su ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.cyclo ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.d ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.o ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_blend.su ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.cyclo ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.d ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.o ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.su ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.cyclo ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.d ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.o ./lvgl/src/draw/nxp/vglite/lv_draw_vglite_rect.su ./lvgl/src/draw/nxp/vglite/lv_vglite_buf.cyclo ./lvgl/src/draw/nxp/vglite/lv_vglite_buf.d ./lvgl/src/draw/nxp/vglite/lv_vglite_buf.o ./lvgl/src/draw/nxp/vglite/lv_vglite_buf.su ./lvgl/src/draw/nxp/vglite/lv_vglite_utils.cyclo ./lvgl/src/draw/nxp/vglite/lv_vglite_utils.d ./lvgl/src/draw/nxp/vglite/lv_vglite_utils.o ./lvgl/src/draw/nxp/vglite/lv_vglite_utils.su

.PHONY: clean-lvgl-2f-src-2f-draw-2f-nxp-2f-vglite

