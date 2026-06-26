################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/User/Startup/startup_stm32n647x0hxq.s 

OBJS += \
./Application/User/Startup/startup_stm32n647x0hxq.o 

S_DEPS += \
./Application/User/Startup/startup_stm32n647x0hxq.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Startup/%.o: ../Application/User/Startup/%.s Application/User/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m55 -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -c -I../../../Appli/Core/Inc -I../../../Appli/AZURE_RTOS/App -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Drivers/CMSIS/Include -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Application-2f-User-2f-Startup

clean-Application-2f-User-2f-Startup:
	-$(RM) ./Application/User/Startup/startup_stm32n647x0hxq.d ./Application/User/Startup/startup_stm32n647x0hxq.o

.PHONY: clean-Application-2f-User-2f-Startup

