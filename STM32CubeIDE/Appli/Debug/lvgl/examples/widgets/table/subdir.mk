################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/examples/widgets/table/lv_example_table_1.c \
../lvgl/examples/widgets/table/lv_example_table_2.c 

OBJS += \
./lvgl/examples/widgets/table/lv_example_table_1.o \
./lvgl/examples/widgets/table/lv_example_table_2.o 

C_DEPS += \
./lvgl/examples/widgets/table/lv_example_table_1.d \
./lvgl/examples/widgets/table/lv_example_table_2.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/examples/widgets/table/%.o lvgl/examples/widgets/table/%.su lvgl/examples/widgets/table/%.cyclo: ../lvgl/examples/widgets/table/%.c lvgl/examples/widgets/table/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-examples-2f-widgets-2f-table

clean-lvgl-2f-examples-2f-widgets-2f-table:
	-$(RM) ./lvgl/examples/widgets/table/lv_example_table_1.cyclo ./lvgl/examples/widgets/table/lv_example_table_1.d ./lvgl/examples/widgets/table/lv_example_table_1.o ./lvgl/examples/widgets/table/lv_example_table_1.su ./lvgl/examples/widgets/table/lv_example_table_2.cyclo ./lvgl/examples/widgets/table/lv_example_table_2.d ./lvgl/examples/widgets/table/lv_example_table_2.o ./lvgl/examples/widgets/table/lv_example_table_2.su

.PHONY: clean-lvgl-2f-examples-2f-widgets-2f-table

