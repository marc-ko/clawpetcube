# HAL Migration Status

## Implemented In This Tree

- CubeIDE/CubeMX seed project file for STM32F405RGTx.
- Root CubeIDE `.project`/`.cproject` redirected to build the HAL tree from `hal_firmware/Release` as `VPetCube_HAL.elf`.
- Official STM32CubeF4 HAL/CMSIS, FreeRTOS, and FatFs sources are vendored under `hal_firmware/Drivers` and `hal_firmware/Middlewares`.
- HAL configuration, FreeRTOS configuration, FatFs configuration, GCC startup, and syscall stubs are present in the HAL tree.
- HAL-style peripheral init for USART1, USART2, USART6, SPI1 + DMA, I2C1, SDIO, RTC, TIM3, TIM7, ADC1, GPIO, and EXTI5.
- USART1 `printf` retarget for USB-C TTL serial monitor logs.
- ST7789-style LCD compatibility driver using the original init sequence and HAL SPI DMA pixel transfer.
- LVGL 8 display port skeleton for the 240x240 display.
- Compatibility include shims for legacy names such as `sys.h`, `delay.h`, `usart.h`, `lcd_init.h`, `mpu6050.h`, `rtc.h`, and `esp8266_common.h`.
- HAL I2C MPU6050 access layer compatible with the InvenSense DMP driver function names.
- ESP8266 AT-command transport skeleton over USART2.
- FreeRTOS task skeleton for LCD/UI bring-up, MPU probe, ESP8266 probe, RTC init, and USART6 event logging.

## Not Hardware-Verified Here

This environment does not include the original board. The firmware must still be flashed and validated on hardware before replacing the SPL firmware.

The latest provided build log completed with 0 errors, but it built the legacy root target: the command line compiled `FWLIB/src/stm32f4xx_*.c`, defined `USE_STDPERIPH_DRIVER`, and linked `VPetCube.elf` from `Release/`. That result confirms the SPL reference still builds; it does not validate this HAL migration target.

The HAL-side source set was smoke-compiled and smoke-linked locally with the STM32CubeIDE GCC 12.3 toolchain after vendoring HAL/CMSIS/FreeRTOS/FatFs. Hardware behavior is still unverified.

## Known Follow-Up Work

- Replace the safe default ESP8266 time/weather/user placeholders with the final OpenClaw/FlashAPI client after board bring-up.
- Link the legacy LVGL 8.1.1 and GUI Guider sources into the HAL project and enable `lv_timer_handler()` in the UI task.
- Confirm the CubeMX `.ioc` opens cleanly; if CubeMX rewrites pin/peripheral settings, preserve the pin map from `README.md`.
- Run the hardware checklist before treating this as a replacement for the SPL firmware.
