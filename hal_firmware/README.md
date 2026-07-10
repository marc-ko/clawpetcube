# VPetCube HAL Firmware

This folder is the STM32CubeIDE/HAL migration target for the original VPetCube board. The legacy SPL firmware in the repository remains the behavior reference until this target is verified on hardware.

## Target

- MCU: STM32F405RGTx
- Clock: HSE 8 MHz, SYSCLK 168 MHz, APB1 42 MHz, APB2 84 MHz
- Debug log: USART1 on PA9/PA10 at 115200, exposed through the board USB-C TTL bridge
- Display: 240x240 ST7789-style SPI TFT on SPI1 with DMA TX
- Middleware: STM32CubeF4 HAL/CMSIS, FreeRTOS, FatFs; existing LVGL 8.1.1 source is referenced from the legacy tree

## Import Flow

The repository root `.project`/`.cproject` now points the active CubeIDE build at this `hal_firmware` tree. The legacy SPL folders are kept as reference-only and are no longer active source roots.

1. Open `VPetCube_HAL.ioc` in STM32CubeIDE or CubeMX.
2. Generate code into this `hal_firmware` folder.
3. Keep the generated HAL/Core files, then merge or keep the files already present under `Core/`, `Board/`, and `Compat/`.
4. Verify include paths for:
   - `Board/Inc`
   - `Compat/Inc`
   - `Drivers/STM32F4xx_HAL_Driver/Inc`
   - `Drivers/CMSIS/Device/ST/STM32F4xx/Include`
   - `Drivers/CMSIS/Include`
   - `Middlewares/Third_Party/FreeRTOS/Source/include`
   - `Middlewares/Third_Party/FatFs/src`
   - legacy `../Middlewares/LVGL/GUI/lvgl`
   - legacy `../Middlewares/LVGL/guiguider`
5. Keep legacy LVGL/GUI Guider assets available, but do not compile SPL hardware drivers from `../HARDWARE`.

## Build Target Check

If the CubeIDE console compiles files from `../FWLIB/src/stm32f4xx_*.c`, defines `USE_STDPERIPH_DRIVER`, or links a target named `VPetCube.elf` from the root `Release/` folder, the legacy SPL firmware is being built, not this HAL migration target.

The HAL target should build under `hal_firmware/Release`, produce `VPetCube_HAL.elf`, compile generated `hal_firmware/Drivers/STM32F4xx_HAL_Driver` sources plus this folder's `Core/`, `Board/`, and `Compat/` sources, and define `USE_HAL_DRIVER` only.

## Hardware Bring-Up Order

1. Confirm USART1 boot logs over USB-C serial monitor at 115200.
2. Confirm LCD reset/backlight and full-screen color fill.
3. Confirm LVGL tick and basic boot screen.
4. Confirm SDIO/FatFs mount.
5. Confirm ESP8266 AT response over USART2.
6. Confirm MPU6050 device ID over I2C1 and EXTI5 interrupt.
7. Confirm USART6 input events: `B1`..`B4`, `T1`, `T2`, `P`.

## Current Scope

This is firmware-only. PHP/CodeIgniter is intentionally not migrated. OpenClaw/FlashAPI integration should be added after this HAL target boots and passes original hardware validation.
