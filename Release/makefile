################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USER/subdir.mk
-include Startup/subdir.mk
-include SYSTEM/usart/subdir.mk
-include SYSTEM/sys/subdir.mk
-include SYSTEM/delay/subdir.mk
-include Middlewares/LVGL/guiguider/images/subdir.mk
-include Middlewares/LVGL/guiguider/guider_fonts/subdir.mk
-include Middlewares/LVGL/guiguider/subdir.mk
-include Middlewares/LVGL/GUI_APP/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/widgets/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/misc/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/hal/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/font/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/win/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/tileview/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/tabview/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/spinner/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/spinbox/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/span/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/msgbox/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/meter/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/menu/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/list/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/led/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/keyboard/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/imgbtn/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/colorwheel/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/chart/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/calendar/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/widgets/animimg/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/themes/mono/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/themes/default/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/themes/basic/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/others/snapshot/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/others/monkey/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/others/gridnav/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/sjpg/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/rlottie/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/qrcode/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/png/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/gif/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/freetype/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/ffmpeg/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/libs/bmp/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/layouts/grid/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/layouts/flex/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/extra/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/sw/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/stm32_dma2d/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/sdl/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/nxp_vglite/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/nxp_pxp/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/draw/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/src/core/subdir.mk
-include Middlewares/LVGL/GUI/lvgl/examples/porting/subdir.mk
-include Middlewares/FatFs/subdir.mk
-include HARDWARE/USART6/subdir.mk
-include HARDWARE/USART2/subdir.mk
-include HARDWARE/TIMER/subdir.mk
-include HARDWARE/SDIO/subdir.mk
-include HARDWARE/RTC/subdir.mk
-include HARDWARE/MPU6050/eMPL/subdir.mk
-include HARDWARE/MPU6050/subdir.mk
-include HARDWARE/LCD/subdir.mk
-include HARDWARE/IIC/subdir.mk
-include HARDWARE/EXTI/subdir.mk
-include HARDWARE/ESP8266/subdir.mk
-include HARDWARE/ADC/subdir.mk
-include HARDWARE/subdir.mk
-include FreeRTOS/portable/MemMang/subdir.mk
-include FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
-include FreeRTOS/subdir.mk
-include FWLIB/src/subdir.mk
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


BUILD_ARTIFACT_NAME := VPetCube
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
VPetCube.elf \

MAP_FILES += \
VPetCube.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
VPetCube.list \

OBJCOPY_HEX += \
VPetCube.hex \


# All Target
all: main-build

# Main-build Target
main-build: VPetCube.elf secondary-outputs

# Tool invocations
VPetCube.elf VPetCube.map: $(OBJS) $(USER_OBJS) G:\git_workplace\VPetCube\STM32F405RGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "VPetCube.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"G:\git_workplace\VPetCube\STM32F405RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="VPetCube.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

VPetCube.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "VPetCube.list"
	@echo 'Finished building: $@'
	@echo ' '

VPetCube.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "VPetCube.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) VPetCube.elf VPetCube.hex VPetCube.list VPetCube.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
