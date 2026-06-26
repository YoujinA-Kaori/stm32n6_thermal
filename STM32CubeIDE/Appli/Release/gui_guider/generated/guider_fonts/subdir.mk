################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.c \
../gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.c \
../gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c 

OBJS += \
./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.o \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.o \
./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.o 

C_DEPS += \
./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.d \
./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.d \
./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.d 


# Each subdirectory must supply rules for building sources it contributes
gui_guider/generated/guider_fonts/%.o gui_guider/generated/guider_fonts/%.su gui_guider/generated/guider_fonts/%.cyclo: ../gui_guider/generated/guider_fonts/%.c gui_guider/generated/guider_fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_THREADX -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.o: D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.c gui_guider/generated/guider_fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DLL_ATON_DUMP_DEBUG_API -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_OSAL=LL_ATON_OSAL_THREADX -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_SW_FALLBACK -DLL_ATON_EB_DBG_INFO -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 -c -I../../../Appli/Core/Inc -I../../../Secure_nsclib -I../../../Drivers/STM32N6xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/BSP" -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -I../../../../../STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/lvgl/porting" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/custom" -I"D:/PracticeProject/Stm32/stm32n6_thermal/STM32CubeIDE/Appli/gui_guider/generated" -I../../../Appli/AZURE_RTOS/App -I../../../Middlewares/ST/threadx/common/inc -I../../../Middlewares/ST/threadx/ports/cortex_m55/gnu/inc -I../../../Appli/FileX/App -I../../../Appli/FileX/Target -I../../../Middlewares/ST/filex/common/inc -I../../../Middlewares/ST/filex/ports/generic/inc -I../../../Middlewares/ST/AI/Npu/Devices/STM32N6XX -I../../../ExtMemLoader/X-CUBE-AI/App -I../../../Middlewares/ST/AI/Inc -I../../../Middlewares/ST/AI/Npu/ll_aton -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-gui_guider-2f-generated-2f-guider_fonts

clean-gui_guider-2f-generated-2f-guider_fonts:
	-$(RM) ./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.cyclo ./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.d ./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.o ./gui_guider/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_18.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_19.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_23.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_25.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_26.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_32.su ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.cyclo ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.d ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.o ./gui_guider/generated/guider_fonts/lv_font_montserratMedium_39.su ./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.cyclo ./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.d ./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.o ./gui_guider/generated/guider_fonts/lv_font_thermal_cn_18.su

.PHONY: clean-gui_guider-2f-generated-2f-guider_fonts

