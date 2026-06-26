################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/examples/event/lv_example_event_1.c \
../lvgl/examples/event/lv_example_event_2.c \
../lvgl/examples/event/lv_example_event_3.c \
../lvgl/examples/event/lv_example_event_4.c 

OBJS += \
./lvgl/examples/event/lv_example_event_1.o \
./lvgl/examples/event/lv_example_event_2.o \
./lvgl/examples/event/lv_example_event_3.o \
./lvgl/examples/event/lv_example_event_4.o 

C_DEPS += \
./lvgl/examples/event/lv_example_event_1.d \
./lvgl/examples/event/lv_example_event_2.d \
./lvgl/examples/event/lv_example_event_3.d \
./lvgl/examples/event/lv_example_event_4.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/examples/event/%.o lvgl/examples/event/%.su lvgl/examples/event/%.cyclo: ../lvgl/examples/event/%.c lvgl/examples/event/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-examples-2f-event

clean-lvgl-2f-examples-2f-event:
	-$(RM) ./lvgl/examples/event/lv_example_event_1.cyclo ./lvgl/examples/event/lv_example_event_1.d ./lvgl/examples/event/lv_example_event_1.o ./lvgl/examples/event/lv_example_event_1.su ./lvgl/examples/event/lv_example_event_2.cyclo ./lvgl/examples/event/lv_example_event_2.d ./lvgl/examples/event/lv_example_event_2.o ./lvgl/examples/event/lv_example_event_2.su ./lvgl/examples/event/lv_example_event_3.cyclo ./lvgl/examples/event/lv_example_event_3.d ./lvgl/examples/event/lv_example_event_3.o ./lvgl/examples/event/lv_example_event_3.su ./lvgl/examples/event/lv_example_event_4.cyclo ./lvgl/examples/event/lv_example_event_4.d ./lvgl/examples/event/lv_example_event_4.o ./lvgl/examples/event/lv_example_event_4.su

.PHONY: clean-lvgl-2f-examples-2f-event

