################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/examples/layouts/grid/lv_example_grid_1.c \
../lvgl/examples/layouts/grid/lv_example_grid_2.c \
../lvgl/examples/layouts/grid/lv_example_grid_3.c \
../lvgl/examples/layouts/grid/lv_example_grid_4.c \
../lvgl/examples/layouts/grid/lv_example_grid_5.c \
../lvgl/examples/layouts/grid/lv_example_grid_6.c 

OBJS += \
./lvgl/examples/layouts/grid/lv_example_grid_1.o \
./lvgl/examples/layouts/grid/lv_example_grid_2.o \
./lvgl/examples/layouts/grid/lv_example_grid_3.o \
./lvgl/examples/layouts/grid/lv_example_grid_4.o \
./lvgl/examples/layouts/grid/lv_example_grid_5.o \
./lvgl/examples/layouts/grid/lv_example_grid_6.o 

C_DEPS += \
./lvgl/examples/layouts/grid/lv_example_grid_1.d \
./lvgl/examples/layouts/grid/lv_example_grid_2.d \
./lvgl/examples/layouts/grid/lv_example_grid_3.d \
./lvgl/examples/layouts/grid/lv_example_grid_4.d \
./lvgl/examples/layouts/grid/lv_example_grid_5.d \
./lvgl/examples/layouts/grid/lv_example_grid_6.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/examples/layouts/grid/%.o lvgl/examples/layouts/grid/%.su lvgl/examples/layouts/grid/%.cyclo: ../lvgl/examples/layouts/grid/%.c lvgl/examples/layouts/grid/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-examples-2f-layouts-2f-grid

clean-lvgl-2f-examples-2f-layouts-2f-grid:
	-$(RM) ./lvgl/examples/layouts/grid/lv_example_grid_1.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_1.d ./lvgl/examples/layouts/grid/lv_example_grid_1.o ./lvgl/examples/layouts/grid/lv_example_grid_1.su ./lvgl/examples/layouts/grid/lv_example_grid_2.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_2.d ./lvgl/examples/layouts/grid/lv_example_grid_2.o ./lvgl/examples/layouts/grid/lv_example_grid_2.su ./lvgl/examples/layouts/grid/lv_example_grid_3.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_3.d ./lvgl/examples/layouts/grid/lv_example_grid_3.o ./lvgl/examples/layouts/grid/lv_example_grid_3.su ./lvgl/examples/layouts/grid/lv_example_grid_4.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_4.d ./lvgl/examples/layouts/grid/lv_example_grid_4.o ./lvgl/examples/layouts/grid/lv_example_grid_4.su ./lvgl/examples/layouts/grid/lv_example_grid_5.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_5.d ./lvgl/examples/layouts/grid/lv_example_grid_5.o ./lvgl/examples/layouts/grid/lv_example_grid_5.su ./lvgl/examples/layouts/grid/lv_example_grid_6.cyclo ./lvgl/examples/layouts/grid/lv_example_grid_6.d ./lvgl/examples/layouts/grid/lv_example_grid_6.o ./lvgl/examples/layouts/grid/lv_example_grid_6.su

.PHONY: clean-lvgl-2f-examples-2f-layouts-2f-grid

