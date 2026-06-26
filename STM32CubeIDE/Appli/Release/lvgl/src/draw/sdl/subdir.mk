################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/draw/sdl/lv_draw_sdl.c \
../lvgl/src/draw/sdl/lv_draw_sdl_arc.c \
../lvgl/src/draw/sdl/lv_draw_sdl_bg.c \
../lvgl/src/draw/sdl/lv_draw_sdl_composite.c \
../lvgl/src/draw/sdl/lv_draw_sdl_img.c \
../lvgl/src/draw/sdl/lv_draw_sdl_label.c \
../lvgl/src/draw/sdl/lv_draw_sdl_layer.c \
../lvgl/src/draw/sdl/lv_draw_sdl_line.c \
../lvgl/src/draw/sdl/lv_draw_sdl_mask.c \
../lvgl/src/draw/sdl/lv_draw_sdl_polygon.c \
../lvgl/src/draw/sdl/lv_draw_sdl_rect.c \
../lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c \
../lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c \
../lvgl/src/draw/sdl/lv_draw_sdl_utils.c 

OBJS += \
./lvgl/src/draw/sdl/lv_draw_sdl.o \
./lvgl/src/draw/sdl/lv_draw_sdl_arc.o \
./lvgl/src/draw/sdl/lv_draw_sdl_bg.o \
./lvgl/src/draw/sdl/lv_draw_sdl_composite.o \
./lvgl/src/draw/sdl/lv_draw_sdl_img.o \
./lvgl/src/draw/sdl/lv_draw_sdl_label.o \
./lvgl/src/draw/sdl/lv_draw_sdl_layer.o \
./lvgl/src/draw/sdl/lv_draw_sdl_line.o \
./lvgl/src/draw/sdl/lv_draw_sdl_mask.o \
./lvgl/src/draw/sdl/lv_draw_sdl_polygon.o \
./lvgl/src/draw/sdl/lv_draw_sdl_rect.o \
./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.o \
./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.o \
./lvgl/src/draw/sdl/lv_draw_sdl_utils.o 

C_DEPS += \
./lvgl/src/draw/sdl/lv_draw_sdl.d \
./lvgl/src/draw/sdl/lv_draw_sdl_arc.d \
./lvgl/src/draw/sdl/lv_draw_sdl_bg.d \
./lvgl/src/draw/sdl/lv_draw_sdl_composite.d \
./lvgl/src/draw/sdl/lv_draw_sdl_img.d \
./lvgl/src/draw/sdl/lv_draw_sdl_label.d \
./lvgl/src/draw/sdl/lv_draw_sdl_layer.d \
./lvgl/src/draw/sdl/lv_draw_sdl_line.d \
./lvgl/src/draw/sdl/lv_draw_sdl_mask.d \
./lvgl/src/draw/sdl/lv_draw_sdl_polygon.d \
./lvgl/src/draw/sdl/lv_draw_sdl_rect.d \
./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.d \
./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.d \
./lvgl/src/draw/sdl/lv_draw_sdl_utils.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/draw/sdl/%.o lvgl/src/draw/sdl/%.su lvgl/src/draw/sdl/%.cyclo: ../lvgl/src/draw/sdl/%.c lvgl/src/draw/sdl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_THREADX -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-src-2f-draw-2f-sdl

clean-lvgl-2f-src-2f-draw-2f-sdl:
	-$(RM) ./lvgl/src/draw/sdl/lv_draw_sdl.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl.d ./lvgl/src/draw/sdl/lv_draw_sdl.o ./lvgl/src/draw/sdl/lv_draw_sdl.su ./lvgl/src/draw/sdl/lv_draw_sdl_arc.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_arc.d ./lvgl/src/draw/sdl/lv_draw_sdl_arc.o ./lvgl/src/draw/sdl/lv_draw_sdl_arc.su ./lvgl/src/draw/sdl/lv_draw_sdl_bg.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_bg.d ./lvgl/src/draw/sdl/lv_draw_sdl_bg.o ./lvgl/src/draw/sdl/lv_draw_sdl_bg.su ./lvgl/src/draw/sdl/lv_draw_sdl_composite.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_composite.d ./lvgl/src/draw/sdl/lv_draw_sdl_composite.o ./lvgl/src/draw/sdl/lv_draw_sdl_composite.su ./lvgl/src/draw/sdl/lv_draw_sdl_img.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_img.d ./lvgl/src/draw/sdl/lv_draw_sdl_img.o ./lvgl/src/draw/sdl/lv_draw_sdl_img.su ./lvgl/src/draw/sdl/lv_draw_sdl_label.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_label.d ./lvgl/src/draw/sdl/lv_draw_sdl_label.o ./lvgl/src/draw/sdl/lv_draw_sdl_label.su ./lvgl/src/draw/sdl/lv_draw_sdl_layer.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_layer.d ./lvgl/src/draw/sdl/lv_draw_sdl_layer.o ./lvgl/src/draw/sdl/lv_draw_sdl_layer.su ./lvgl/src/draw/sdl/lv_draw_sdl_line.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_line.d ./lvgl/src/draw/sdl/lv_draw_sdl_line.o ./lvgl/src/draw/sdl/lv_draw_sdl_line.su ./lvgl/src/draw/sdl/lv_draw_sdl_mask.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_mask.d ./lvgl/src/draw/sdl/lv_draw_sdl_mask.o ./lvgl/src/draw/sdl/lv_draw_sdl_mask.su ./lvgl/src/draw/sdl/lv_draw_sdl_polygon.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_polygon.d ./lvgl/src/draw/sdl/lv_draw_sdl_polygon.o ./lvgl/src/draw/sdl/lv_draw_sdl_polygon.su ./lvgl/src/draw/sdl/lv_draw_sdl_rect.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_rect.d ./lvgl/src/draw/sdl/lv_draw_sdl_rect.o ./lvgl/src/draw/sdl/lv_draw_sdl_rect.su ./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.d ./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.o ./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.su ./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.d ./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.o ./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.su ./lvgl/src/draw/sdl/lv_draw_sdl_utils.cyclo ./lvgl/src/draw/sdl/lv_draw_sdl_utils.d ./lvgl/src/draw/sdl/lv_draw_sdl_utils.o ./lvgl/src/draw/sdl/lv_draw_sdl_utils.su

.PHONY: clean-lvgl-2f-src-2f-draw-2f-sdl

