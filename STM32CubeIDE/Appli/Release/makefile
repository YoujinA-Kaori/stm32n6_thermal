################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := cmd /c del /f /q

# All of the sources participating in the build are defined here
-include sources.mk
-include lvgl/src/widgets/subdir.mk
-include lvgl/src/misc/subdir.mk
-include lvgl/src/hal/subdir.mk
-include lvgl/src/font/subdir.mk
-include lvgl/src/extra/widgets/win/subdir.mk
-include lvgl/src/extra/widgets/tileview/subdir.mk
-include lvgl/src/extra/widgets/tabview/subdir.mk
-include lvgl/src/extra/widgets/spinner/subdir.mk
-include lvgl/src/extra/widgets/spinbox/subdir.mk
-include lvgl/src/extra/widgets/span/subdir.mk
-include lvgl/src/extra/widgets/msgbox/subdir.mk
-include lvgl/src/extra/widgets/meter/subdir.mk
-include lvgl/src/extra/widgets/menu/subdir.mk
-include lvgl/src/extra/widgets/list/subdir.mk
-include lvgl/src/extra/widgets/led/subdir.mk
-include lvgl/src/extra/widgets/keyboard/subdir.mk
-include lvgl/src/extra/widgets/imgbtn/subdir.mk
-include lvgl/src/extra/widgets/colorwheel/subdir.mk
-include lvgl/src/extra/widgets/chart/subdir.mk
-include lvgl/src/extra/widgets/calendar/subdir.mk
-include lvgl/src/extra/widgets/animimg/subdir.mk
-include lvgl/src/extra/themes/mono/subdir.mk
-include lvgl/src/extra/themes/default/subdir.mk
-include lvgl/src/extra/themes/basic/subdir.mk
-include lvgl/src/extra/others/snapshot/subdir.mk
-include lvgl/src/extra/others/msg/subdir.mk
-include lvgl/src/extra/others/monkey/subdir.mk
-include lvgl/src/extra/others/imgfont/subdir.mk
-include lvgl/src/extra/others/ime/subdir.mk
-include lvgl/src/extra/others/gridnav/subdir.mk
-include lvgl/src/extra/others/fragment/subdir.mk
-include lvgl/src/extra/libs/tiny_ttf/subdir.mk
-include lvgl/src/extra/libs/sjpg/subdir.mk
-include lvgl/src/extra/libs/rlottie/subdir.mk
-include lvgl/src/extra/libs/qrcode/subdir.mk
-include lvgl/src/extra/libs/png/subdir.mk
-include lvgl/src/extra/libs/gif/subdir.mk
-include lvgl/src/extra/libs/fsdrv/subdir.mk
-include lvgl/src/extra/libs/freetype/subdir.mk
-include lvgl/src/extra/libs/ffmpeg/subdir.mk
-include lvgl/src/extra/libs/bmp/subdir.mk
-include lvgl/src/extra/layouts/grid/subdir.mk
-include lvgl/src/extra/layouts/flex/subdir.mk
-include lvgl/src/extra/subdir.mk
-include lvgl/src/draw/swm341_dma2d/subdir.mk
-include lvgl/src/draw/sw/subdir.mk
-include lvgl/src/draw/stm32_dma2d/subdir.mk
-include lvgl/src/draw/sdl/subdir.mk
-include lvgl/src/draw/renesas/subdir.mk
-include lvgl/src/draw/nxp/vglite/subdir.mk
-include lvgl/src/draw/nxp/pxp/subdir.mk
-include lvgl/src/draw/arm2d/subdir.mk
-include lvgl/src/draw/subdir.mk
-include lvgl/src/core/subdir.mk
-include lvgl/porting/subdir.mk
-include gui_guider/generated/images/subdir.mk
-include gui_guider/generated/guider_fonts/subdir.mk
-include gui_guider/generated/subdir.mk
-include gui_guider/custom/subdir.mk
-include ThreadX_Port_Src/subdir.mk
-include ThreadX_Common_Src/subdir.mk
-include Middlewares/AI/Npu/ll_aton/subdir.mk
-include Middlewares/AI/Npu/Devices/STM32N6XX/subdir.mk
-include FileX_SD_Driver_Src/subdir.mk
-include FileX_Common_Src/subdir.mk
-include Drivers/STM32N6xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include BSP/Tiny1C/VDCMD/subdir.mk
-include BSP/Tiny1C/subdir.mk
-include BSP/TOUCH/subdir.mk
-include BSP/SYS/subdir.mk
-include BSP/SD_NAND/subdir.mk
-include BSP/RGBLCD/subdir.mk
-include BSP/LED/subdir.mk
-include BSP/KEY/subdir.mk
-include BSP/HyperRAM/subdir.mk
-include BSP/EEPROM/subdir.mk
-include BSP/BQ27441/subdir.mk
-include Application/User/Startup/subdir.mk
-include Application/User/FileX/Target/subdir.mk
-include Application/User/FileX/App/subdir.mk
-include Application/User/Core/subdir.mk
-include Application/User/AZURE_RTOS/App/subdir.mk
-include AI_App/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := stm32n6_thermal_Appli
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
stm32n6_thermal_Appli.elf \

MAP_FILES += \
stm32n6_thermal_Appli.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
stm32n6_thermal_Appli.list \

OBJCOPY_HEX += \
stm32n6_thermal_Appli.hex \

OBJCOPY_BIN += \
stm32n6_thermal_Appli.bin \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: stm32n6_thermal_Appli.elf secondary-outputs

# Tool invocations
stm32n6_thermal_Appli.elf stm32n6_thermal_Appli.map: $(OBJS) $(USER_OBJS) D:\PracticeProject\Stm32\stm32n6_thermal\STM32CubeIDE\Appli\STM32N647X0HXQ_ROMxspi2_RAMxspi1.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "stm32n6_thermal_Appli.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m55 -T"D:\PracticeProject\Stm32\stm32n6_thermal\STM32CubeIDE\Appli\STM32N647X0HXQ_ROMxspi2_RAMxspi1.ld" --specs=nosys.specs -Wl,-Map="stm32n6_thermal_Appli.map" -Wl,--gc-sections -static -Wl,--cmse-implib -Wl,--out-implib=./secure_nsclib.o --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

stm32n6_thermal_Appli.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "stm32n6_thermal_Appli.list"
	@echo 'Finished building: $@'
	@echo ' '

stm32n6_thermal_Appli.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "stm32n6_thermal_Appli.hex"
	@echo 'Finished building: $@'
	@echo ' '

stm32n6_thermal_Appli.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "stm32n6_thermal_Appli.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout stm32n6_thermal_Appli.bin stm32n6_thermal_Appli.elf stm32n6_thermal_Appli.hex stm32n6_thermal_Appli.list stm32n6_thermal_Appli.map
	-@echo ' '

post-build:
	powershell -ExecutionPolicy Bypass -File "../postbuild.ps1" "D:\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.400.202601091506\tools\bin"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
