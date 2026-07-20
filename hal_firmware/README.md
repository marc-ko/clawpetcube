# VPetCube HAL Firmware

This folder is the active STM32Cube HAL firmware for ClawPetCube. The legacy
SPL firmware in the repository remains a reference, but new dashboard work
should happen here.

## Target

- MCU: STM32F405RGTx
- Clock: HSE 8 MHz, SYSCLK 168 MHz, APB1 42 MHz, APB2 84 MHz
- Debug log: USART1 on PA9/PA10 at 115200 through the board USB-C TTL bridge
- Display: 240x240 ST7789-style SPI TFT on SPI1
- WiFi: ESP-12F / ESP8266 AT firmware on USART2
- Input: USART6 touch/input events
- Sensor: MPU6050 on I2C1
- Middleware: STM32CubeF4 HAL/CMSIS, FreeRTOS, FatFs, and legacy LVGL 8.1.1

## Current Firmware

Implemented and brought up:

- HAL peripheral init for USART1, USART2, USART6, SPI1 + DMA, I2C1, SDIO, RTC,
  TIM3, TIM7, ADC1, GPIO, and EXTI5.
- USART1 `printf` retarget for serial logs.
- ST7789 LCD compatibility driver using the original panel init sequence.
- LVGL display port and active `lv_timer_handler()` UI loop.
- TIM3/TIM7 MSP clock and IRQ setup; TIM3 now advances LVGL time correctly.
- FreeRTOS UI and ESP8266 monitor tasks.
- ESP8266 AT-command transport with bounded recovery from `busy` / TCP failure
  states.
- HKO HTTP time sync and STM32 RTC update.
- OpenClaw `/health`, `/status`, and `/message` polling through a configured
  plain HTTP cube endpoint.
- Compact OpenClaw dashboard with time/date, mascot, speech bubble, `CRON`,
  `HEALTH`, and rotating `PROC` / `DISK` / `MEM` detail states.
- Sensitive WiFi and API host commands masked in serial logs.

Still to validate:

- SDIO/FatFs mount on hardware.
- MPU6050 ID and interrupt.
- USART6 input event handling on hardware.
- Long-run dashboard stability.
- Final visual polish after new LCD photos are available.

## Local Configuration

Do not commit credentials or private hostnames. Use an ignored
`Core/Inc/app_config_local.h` or CubeIDE project symbols for local values:

```c
#define VPC_WIFI_SSID "..."
#define VPC_WIFI_PASSWORD "..."
#define VPC_OPENCLAW_HTTP_HOST "..."
#define VPC_OPENCLAW_HTTP_PORT "80"
```

`Core/Inc/app_config.h` keeps all of these empty by default so the repository
does not store real secrets.

## Build Target Check

Build the HAL firmware from the repository root:

```powershell
$env:Path='C:\ST\STM32CubeIDE_1.16.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623\tools\bin;' + $env:Path
make -C Release all
```

Expected outputs:

- `Release/VPetCube_HAL.elf`
- `Release/VPetCube_HAL.hex`

If the CubeIDE console compiles files from `../FWLIB/src/stm32f4xx_*.c`, defines
`USE_STDPERIPH_DRIVER`, or links a target named `VPetCube.elf`, it is using
stale generated metadata from the removed legacy SPL target instead of this HAL
target.

## Import / Regeneration Notes

1. Open `VPetCube_HAL.ioc` in STM32CubeIDE or CubeMX.
2. Generate code into this `hal_firmware` folder.
3. Preserve the hand-maintained files under `Core/`, `Board/`, and `Compat/`.
4. Keep include paths for:
   - `Board/Inc`
   - `Compat/Inc`
   - `Drivers/STM32F4xx_HAL_Driver/Inc`
   - `Drivers/CMSIS/Device/ST/STM32F4xx/Include`
   - `Drivers/CMSIS/Include`
   - `Middlewares/Third_Party/FreeRTOS/Source/include`
   - `Middlewares/Third_Party/FatFs/src`
   - legacy `../Middlewares/LVGL/GUI/lvgl`
   - legacy `../Middlewares/LVGL/guiguider`
5. Keep legacy LVGL/GUI Guider assets available. The old SPL hardware driver
   folders are no longer present in this checkout.

## Hardware Bring-Up Checklist

Already passed:

- USART1 boot logs at 115200.
- LCD reset/backlight/init and LVGL rendering.
- LVGL tick and screen refresh after the TIM3 MSP fix.
- ESP8266 AT response and WiFi join.
- HKO time sync and RTC update.
- OpenClaw health/status/message polling through the cube HTTP endpoint.

Next checks:

- Visually confirm the latest dashboard layout on the LCD.
- Run the dashboard for 10-30 minutes and watch for stale status or display
  artifacts.
- Confirm SDIO/FatFs mount.
- Confirm MPU6050 ID and EXTI5 interrupt.
- Confirm USART6 input events: `B1`..`B4`, `T1`, `T2`, `P`.

## Related Docs

- `../CLAWPETCUBE_PLAN.md`: project plan and current product direction.
- `OPENCLAW_BRINGUP_JOURNAL.md`: OpenClaw/ESP8266 verification history.
- `LVGL_BRINGUP_JOURNAL.md`: LCD/LVGL bring-up root cause and fix.
- `MIGRATION_STATUS.md`: subsystem status summary.
