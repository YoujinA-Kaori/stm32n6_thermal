################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/widgets/lv_arc.c \
../lvgl/src/widgets/lv_bar.c \
../lvgl/src/widgets/lv_btn.c \
../lvgl/src/widgets/lv_btnmatrix.c \
../lvgl/src/widgets/lv_canvas.c \
../lvgl/src/widgets/lv_checkbox.c \
../lvgl/src/widgets/lv_dropdown.c \
../lvgl/src/widgets/lv_img.c \
../lvgl/src/widgets/lv_label.c \
../lvgl/src/widgets/lv_line.c \
../lvgl/src/widgets/lv_objx_templ.c \
../lvgl/src/widgets/lv_roller.c \
../lvgl/src/widgets/lv_slider.c \
../lvgl/src/widgets/lv_switch.c \
../lvgl/src/widgets/lv_table.c \
../lvgl/src/widgets/lv_textarea.c 

OBJS += \
./lvgl/src/widgets/lv_arc.o \
./lvgl/src/widgets/lv_bar.o \
./lvgl/src/widgets/lv_btn.o \
./lvgl/src/widgets/lv_btnmatrix.o \
./lvgl/src/widgets/lv_canvas.o \
./lvgl/src/widgets/lv_checkbox.o \
./lvgl/src/widgets/lv_dropdown.o \
./lvgl/src/widgets/lv_img.o \
./lvgl/src/widgets/lv_label.o \
./lvgl/src/widgets/lv_line.o \
./lvgl/src/widgets/lv_objx_templ.o \
./lvgl/src/widgets/lv_roller.o \
./lvgl/src/widgets/lv_slider.o \
./lvgl/src/widgets/lv_switch.o \
./lvgl/src/widgets/lv_table.o \
./lvgl/src/widgets/lv_textarea.o 

C_DEPS += \
./lvgl/src/widgets/lv_arc.d \
./lvgl/src/widgets/lv_bar.d \
./lvgl/src/widgets/lv_btn.d \
./lvgl/src/widgets/lv_btnmatrix.d \
./lvgl/src/widgets/lv_canvas.d \
./lvgl/src/widgets/lv_checkbox.d \
./lvgl/src/widgets/lv_dropdown.d \
./lvgl/src/widgets/lv_img.d \
./lvgl/src/widgets/lv_label.d \
./lvgl/src/widgets/lv_line.d \
./lvgl/src/widgets/lv_objx_templ.d \
./lvgl/src/widgets/lv_roller.d \
./lvgl/src/widgets/lv_slider.d \
./lvgl/src/widgets/lv_switch.d \
./lvgl/src/widgets/lv_table.d \
./lvgl/src/widgets/lv_textarea.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/widgets/%.o lvgl/src/widgets/%.su lvgl/src/widgets/%.cyclo: ../lvgl/src/widgets/%.c lvgl/src/widgets/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_THREADX -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-src-2f-widgets

clean-lvgl-2f-src-2f-widgets:
	-$(RM) ./lvgl/src/widgets/lv_arc.cyclo ./lvgl/src/widgets/lv_arc.d ./lvgl/src/widgets/lv_arc.o ./lvgl/src/widgets/lv_arc.su ./lvgl/src/widgets/lv_bar.cyclo ./lvgl/src/widgets/lv_bar.d ./lvgl/src/widgets/lv_bar.o ./lvgl/src/widgets/lv_bar.su ./lvgl/src/widgets/lv_btn.cyclo ./lvgl/src/widgets/lv_btn.d ./lvgl/src/widgets/lv_btn.o ./lvgl/src/widgets/lv_btn.su ./lvgl/src/widgets/lv_btnmatrix.cyclo ./lvgl/src/widgets/lv_btnmatrix.d ./lvgl/src/widgets/lv_btnmatrix.o ./lvgl/src/widgets/lv_btnmatrix.su ./lvgl/src/widgets/lv_canvas.cyclo ./lvgl/src/widgets/lv_canvas.d ./lvgl/src/widgets/lv_canvas.o ./lvgl/src/widgets/lv_canvas.su ./lvgl/src/widgets/lv_checkbox.cyclo ./lvgl/src/widgets/lv_checkbox.d ./lvgl/src/widgets/lv_checkbox.o ./lvgl/src/widgets/lv_checkbox.su ./lvgl/src/widgets/lv_dropdown.cyclo ./lvgl/src/widgets/lv_dropdown.d ./lvgl/src/widgets/lv_dropdown.o ./lvgl/src/widgets/lv_dropdown.su ./lvgl/src/widgets/lv_img.cyclo ./lvgl/src/widgets/lv_img.d ./lvgl/src/widgets/lv_img.o ./lvgl/src/widgets/lv_img.su ./lvgl/src/widgets/lv_label.cyclo ./lvgl/src/widgets/lv_label.d ./lvgl/src/widgets/lv_label.o ./lvgl/src/widgets/lv_label.su ./lvgl/src/widgets/lv_line.cyclo ./lvgl/src/widgets/lv_line.d ./lvgl/src/widgets/lv_line.o ./lvgl/src/widgets/lv_line.su ./lvgl/src/widgets/lv_objx_templ.cyclo ./lvgl/src/widgets/lv_objx_templ.d ./lvgl/src/widgets/lv_objx_templ.o ./lvgl/src/widgets/lv_objx_templ.su ./lvgl/src/widgets/lv_roller.cyclo ./lvgl/src/widgets/lv_roller.d ./lvgl/src/widgets/lv_roller.o ./lvgl/src/widgets/lv_roller.su ./lvgl/src/widgets/lv_slider.cyclo ./lvgl/src/widgets/lv_slider.d ./lvgl/src/widgets/lv_slider.o ./lvgl/src/widgets/lv_slider.su ./lvgl/src/widgets/lv_switch.cyclo ./lvgl/src/widgets/lv_switch.d ./lvgl/src/widgets/lv_switch.o ./lvgl/src/widgets/lv_switch.su ./lvgl/src/widgets/lv_table.cyclo ./lvgl/src/widgets/lv_table.d ./lvgl/src/widgets/lv_table.o ./lvgl/src/widgets/lv_table.su ./lvgl/src/widgets/lv_textarea.cyclo ./lvgl/src/widgets/lv_textarea.d ./lvgl/src/widgets/lv_textarea.o ./lvgl/src/widgets/lv_textarea.su

.PHONY: clean-lvgl-2f-src-2f-widgets

