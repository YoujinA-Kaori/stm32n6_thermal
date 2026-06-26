################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/tests/src/test_cases/_test_template.c \
../lvgl/tests/src/test_cases/test_arc.c \
../lvgl/tests/src/test_cases/test_bar.c \
../lvgl/tests/src/test_cases/test_checkbox.c \
../lvgl/tests/src/test_cases/test_config.c \
../lvgl/tests/src/test_cases/test_demo_stress.c \
../lvgl/tests/src/test_cases/test_demo_widgets.c \
../lvgl/tests/src/test_cases/test_dropdown.c \
../lvgl/tests/src/test_cases/test_event.c \
../lvgl/tests/src/test_cases/test_font_loader.c \
../lvgl/tests/src/test_cases/test_fs.c \
../lvgl/tests/src/test_cases/test_line.c \
../lvgl/tests/src/test_cases/test_mem.c \
../lvgl/tests/src/test_cases/test_obj_tree.c \
../lvgl/tests/src/test_cases/test_screen_load.c \
../lvgl/tests/src/test_cases/test_slider.c \
../lvgl/tests/src/test_cases/test_snapshot.c \
../lvgl/tests/src/test_cases/test_style.c \
../lvgl/tests/src/test_cases/test_switch.c \
../lvgl/tests/src/test_cases/test_table.c \
../lvgl/tests/src/test_cases/test_textarea.c \
../lvgl/tests/src/test_cases/test_tiny_ttf.c \
../lvgl/tests/src/test_cases/test_txt.c 

OBJS += \
./lvgl/tests/src/test_cases/_test_template.o \
./lvgl/tests/src/test_cases/test_arc.o \
./lvgl/tests/src/test_cases/test_bar.o \
./lvgl/tests/src/test_cases/test_checkbox.o \
./lvgl/tests/src/test_cases/test_config.o \
./lvgl/tests/src/test_cases/test_demo_stress.o \
./lvgl/tests/src/test_cases/test_demo_widgets.o \
./lvgl/tests/src/test_cases/test_dropdown.o \
./lvgl/tests/src/test_cases/test_event.o \
./lvgl/tests/src/test_cases/test_font_loader.o \
./lvgl/tests/src/test_cases/test_fs.o \
./lvgl/tests/src/test_cases/test_line.o \
./lvgl/tests/src/test_cases/test_mem.o \
./lvgl/tests/src/test_cases/test_obj_tree.o \
./lvgl/tests/src/test_cases/test_screen_load.o \
./lvgl/tests/src/test_cases/test_slider.o \
./lvgl/tests/src/test_cases/test_snapshot.o \
./lvgl/tests/src/test_cases/test_style.o \
./lvgl/tests/src/test_cases/test_switch.o \
./lvgl/tests/src/test_cases/test_table.o \
./lvgl/tests/src/test_cases/test_textarea.o \
./lvgl/tests/src/test_cases/test_tiny_ttf.o \
./lvgl/tests/src/test_cases/test_txt.o 

C_DEPS += \
./lvgl/tests/src/test_cases/_test_template.d \
./lvgl/tests/src/test_cases/test_arc.d \
./lvgl/tests/src/test_cases/test_bar.d \
./lvgl/tests/src/test_cases/test_checkbox.d \
./lvgl/tests/src/test_cases/test_config.d \
./lvgl/tests/src/test_cases/test_demo_stress.d \
./lvgl/tests/src/test_cases/test_demo_widgets.d \
./lvgl/tests/src/test_cases/test_dropdown.d \
./lvgl/tests/src/test_cases/test_event.d \
./lvgl/tests/src/test_cases/test_font_loader.d \
./lvgl/tests/src/test_cases/test_fs.d \
./lvgl/tests/src/test_cases/test_line.d \
./lvgl/tests/src/test_cases/test_mem.d \
./lvgl/tests/src/test_cases/test_obj_tree.d \
./lvgl/tests/src/test_cases/test_screen_load.d \
./lvgl/tests/src/test_cases/test_slider.d \
./lvgl/tests/src/test_cases/test_snapshot.d \
./lvgl/tests/src/test_cases/test_style.d \
./lvgl/tests/src/test_cases/test_switch.d \
./lvgl/tests/src/test_cases/test_table.d \
./lvgl/tests/src/test_cases/test_textarea.d \
./lvgl/tests/src/test_cases/test_tiny_ttf.d \
./lvgl/tests/src/test_cases/test_txt.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/tests/src/test_cases/%.o lvgl/tests/src/test_cases/%.su lvgl/tests/src/test_cases/%.cyclo: ../lvgl/tests/src/test_cases/%.c lvgl/tests/src/test_cases/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-tests-2f-src-2f-test_cases

clean-lvgl-2f-tests-2f-src-2f-test_cases:
	-$(RM) ./lvgl/tests/src/test_cases/_test_template.cyclo ./lvgl/tests/src/test_cases/_test_template.d ./lvgl/tests/src/test_cases/_test_template.o ./lvgl/tests/src/test_cases/_test_template.su ./lvgl/tests/src/test_cases/test_arc.cyclo ./lvgl/tests/src/test_cases/test_arc.d ./lvgl/tests/src/test_cases/test_arc.o ./lvgl/tests/src/test_cases/test_arc.su ./lvgl/tests/src/test_cases/test_bar.cyclo ./lvgl/tests/src/test_cases/test_bar.d ./lvgl/tests/src/test_cases/test_bar.o ./lvgl/tests/src/test_cases/test_bar.su ./lvgl/tests/src/test_cases/test_checkbox.cyclo ./lvgl/tests/src/test_cases/test_checkbox.d ./lvgl/tests/src/test_cases/test_checkbox.o ./lvgl/tests/src/test_cases/test_checkbox.su ./lvgl/tests/src/test_cases/test_config.cyclo ./lvgl/tests/src/test_cases/test_config.d ./lvgl/tests/src/test_cases/test_config.o ./lvgl/tests/src/test_cases/test_config.su ./lvgl/tests/src/test_cases/test_demo_stress.cyclo ./lvgl/tests/src/test_cases/test_demo_stress.d ./lvgl/tests/src/test_cases/test_demo_stress.o ./lvgl/tests/src/test_cases/test_demo_stress.su ./lvgl/tests/src/test_cases/test_demo_widgets.cyclo ./lvgl/tests/src/test_cases/test_demo_widgets.d ./lvgl/tests/src/test_cases/test_demo_widgets.o ./lvgl/tests/src/test_cases/test_demo_widgets.su ./lvgl/tests/src/test_cases/test_dropdown.cyclo ./lvgl/tests/src/test_cases/test_dropdown.d ./lvgl/tests/src/test_cases/test_dropdown.o ./lvgl/tests/src/test_cases/test_dropdown.su ./lvgl/tests/src/test_cases/test_event.cyclo ./lvgl/tests/src/test_cases/test_event.d ./lvgl/tests/src/test_cases/test_event.o ./lvgl/tests/src/test_cases/test_event.su ./lvgl/tests/src/test_cases/test_font_loader.cyclo ./lvgl/tests/src/test_cases/test_font_loader.d ./lvgl/tests/src/test_cases/test_font_loader.o ./lvgl/tests/src/test_cases/test_font_loader.su ./lvgl/tests/src/test_cases/test_fs.cyclo ./lvgl/tests/src/test_cases/test_fs.d ./lvgl/tests/src/test_cases/test_fs.o ./lvgl/tests/src/test_cases/test_fs.su ./lvgl/tests/src/test_cases/test_line.cyclo ./lvgl/tests/src/test_cases/test_line.d ./lvgl/tests/src/test_cases/test_line.o ./lvgl/tests/src/test_cases/test_line.su ./lvgl/tests/src/test_cases/test_mem.cyclo ./lvgl/tests/src/test_cases/test_mem.d ./lvgl/tests/src/test_cases/test_mem.o ./lvgl/tests/src/test_cases/test_mem.su ./lvgl/tests/src/test_cases/test_obj_tree.cyclo ./lvgl/tests/src/test_cases/test_obj_tree.d ./lvgl/tests/src/test_cases/test_obj_tree.o ./lvgl/tests/src/test_cases/test_obj_tree.su ./lvgl/tests/src/test_cases/test_screen_load.cyclo ./lvgl/tests/src/test_cases/test_screen_load.d ./lvgl/tests/src/test_cases/test_screen_load.o ./lvgl/tests/src/test_cases/test_screen_load.su ./lvgl/tests/src/test_cases/test_slider.cyclo ./lvgl/tests/src/test_cases/test_slider.d ./lvgl/tests/src/test_cases/test_slider.o ./lvgl/tests/src/test_cases/test_slider.su ./lvgl/tests/src/test_cases/test_snapshot.cyclo ./lvgl/tests/src/test_cases/test_snapshot.d ./lvgl/tests/src/test_cases/test_snapshot.o ./lvgl/tests/src/test_cases/test_snapshot.su ./lvgl/tests/src/test_cases/test_style.cyclo ./lvgl/tests/src/test_cases/test_style.d ./lvgl/tests/src/test_cases/test_style.o ./lvgl/tests/src/test_cases/test_style.su ./lvgl/tests/src/test_cases/test_switch.cyclo ./lvgl/tests/src/test_cases/test_switch.d ./lvgl/tests/src/test_cases/test_switch.o ./lvgl/tests/src/test_cases/test_switch.su ./lvgl/tests/src/test_cases/test_table.cyclo ./lvgl/tests/src/test_cases/test_table.d ./lvgl/tests/src/test_cases/test_table.o ./lvgl/tests/src/test_cases/test_table.su ./lvgl/tests/src/test_cases/test_textarea.cyclo ./lvgl/tests/src/test_cases/test_textarea.d ./lvgl/tests/src/test_cases/test_textarea.o ./lvgl/tests/src/test_cases/test_textarea.su ./lvgl/tests/src/test_cases/test_tiny_ttf.cyclo ./lvgl/tests/src/test_cases/test_tiny_ttf.d ./lvgl/tests/src/test_cases/test_tiny_ttf.o ./lvgl/tests/src/test_cases/test_tiny_ttf.su ./lvgl/tests/src/test_cases/test_txt.cyclo ./lvgl/tests/src/test_cases/test_txt.d ./lvgl/tests/src/test_cases/test_txt.o ./lvgl/tests/src/test_cases/test_txt.su

.PHONY: clean-lvgl-2f-tests-2f-src-2f-test_cases

