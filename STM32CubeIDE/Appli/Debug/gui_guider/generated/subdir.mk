################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gui_guider/generated/events_init.c \
../gui_guider/generated/gui_guider.c \
../gui_guider/generated/setup_scr_WidgetsDemo.c \
../gui_guider/generated/widgets_init.c 

OBJS += \
./gui_guider/generated/events_init.o \
./gui_guider/generated/gui_guider.o \
./gui_guider/generated/setup_scr_WidgetsDemo.o \
./gui_guider/generated/widgets_init.o 

C_DEPS += \
./gui_guider/generated/events_init.d \
./gui_guider/generated/gui_guider.d \
./gui_guider/generated/setup_scr_WidgetsDemo.d \
./gui_guider/generated/widgets_init.d 


# Each subdirectory must supply rules for building sources it contributes
gui_guider/generated/%.o gui_guider/generated/%.su gui_guider/generated/%.cyclo: ../gui_guider/generated/%.c gui_guider/generated/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-gui_guider-2f-generated

clean-gui_guider-2f-generated:
	-$(RM) ./gui_guider/generated/events_init.cyclo ./gui_guider/generated/events_init.d ./gui_guider/generated/events_init.o ./gui_guider/generated/events_init.su ./gui_guider/generated/gui_guider.cyclo ./gui_guider/generated/gui_guider.d ./gui_guider/generated/gui_guider.o ./gui_guider/generated/gui_guider.su ./gui_guider/generated/setup_scr_WidgetsDemo.cyclo ./gui_guider/generated/setup_scr_WidgetsDemo.d ./gui_guider/generated/setup_scr_WidgetsDemo.o ./gui_guider/generated/setup_scr_WidgetsDemo.su ./gui_guider/generated/widgets_init.cyclo ./gui_guider/generated/widgets_init.d ./gui_guider/generated/widgets_init.o ./gui_guider/generated/widgets_init.su

.PHONY: clean-gui_guider-2f-generated

