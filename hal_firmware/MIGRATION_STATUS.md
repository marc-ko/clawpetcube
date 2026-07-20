# HAL Migration Status

Last updated: 2026-07-21

## Summary

The HAL migration is no longer just a seed project. The STM32F405 board has a
working HAL firmware path with LCD/LVGL display refresh, FreeRTOS tasks, ESP8266
WiFi/time sync, and live OpenClaw dashboard polling.

The remaining work is mostly hardware validation, long-run testing, and UI
polish rather than basic HAL bring-up.

## Implemented In This Tree

- CubeIDE/CubeMX seed project file for STM32F405RGTx.
- Root CubeIDE project redirected to build the HAL target as
  `VPetCube_HAL.elf`.
- STM32CubeF4 HAL/CMSIS, FreeRTOS, and FatFs sources vendored under
  `hal_firmware/Drivers` and `hal_firmware/Middlewares`.
- HAL configuration, FreeRTOS configuration, FatFs configuration, GCC startup,
  and syscall stubs.
- HAL peripheral init for USART1, USART2, USART6, SPI1 + DMA, I2C1, SDIO, RTC,
  TIM3, TIM7, ADC1, GPIO, and EXTI5.
- USART1 `printf` retarget for USB-C TTL serial monitor logs.
- ST7789-style LCD compatibility driver using the original init sequence.
- LVGL 8 display port for the 240x240 LCD.
- TIM3 LVGL tick path and TIM7 USART idle-frame support.
- Compatibility include shims for legacy names such as `sys.h`, `delay.h`,
  `usart.h`, `lcd_init.h`, `mpu6050.h`, `rtc.h`, and `esp8266_common.h`.
- HAL I2C MPU6050 access layer compatible with the InvenSense DMP function
  names.
- ESP8266 AT-command transport over USART2, including recovery from repeated
  `busy` and TCP failure states.
- HKO HTTP time sync and RTC update.
- OpenClaw health/status/message polling over the configured cube HTTP endpoint.
- LVGL OpenClaw dashboard with clock/date, mascot, speech bubble, cron card,
  health card, and rotating detail/error card.
- Serial log masking for configured WiFi/API host commands.

## Hardware-Verified

From the bring-up journals and serial logs:

- ST-LINK/OpenOCD connects to STM32F405 at about 950 kHz SWD.
- Firmware flashes and runs on the board.
- LCD reset/backlight/init works.
- LVGL draws and updates after the TIM3 MSP fix.
- ESP8266 joins WiFi.
- HKO time sync updates RTC.
- OpenClaw health returns `alive` through the configured HTTP endpoint.
- OpenClaw status parses gateway, process, cron, disk, and memory values.
- OpenClaw message polling receives timestamped messages.

See `LVGL_BRINGUP_JOURNAL.md` and `OPENCLAW_BRINGUP_JOURNAL.md` for the serial
evidence and root-cause notes.

## Still Pending

- Fresh visual confirmation of the latest dashboard on the LCD.
- Long-run dashboard test for stale network state or display artifacts.
- SDIO/FatFs mount validation.
- MPU6050 ID and EXTI interrupt validation.
- USART6 touch/input validation.
- Optional SD/FatFs persistence.
- Optional SPI DMA flush performance revisit after the UI is stable.
- Later GUI Guider or polished asset integration if the small screen can carry
  it cleanly.

## Known Constraints

- The current ESP8266 AT firmware cannot reliably connect directly to the
  private Cloudflare HTTPS endpoint.
- The working OpenClaw path is a cube-specific plain HTTP endpoint on port 80
  that proxies private OpenClaw data server-side.
- Credentials and private hostnames must stay in ignored local config or CubeIDE
  project symbols, not in tracked source.
- The legacy SPL firmware source tree has been removed from this checkout. Use
  the plan and bring-up journals as the historical behavior reference.
