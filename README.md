# ClawPetCube

ClawPetCube is a physical desk monitor for OpenClaw, built by reusing the original
VPetCube STM32F405 hardware. The legacy virtual-pet project is still kept in this
repository as reference material, but the active product direction is now an
always-on 240x240 LCD dashboard that shows service health, work processed,
resource state, and pushed operator messages without opening a browser.

The active firmware is the STM32Cube HAL port in `hal_firmware/`. The earlier
ESP32-S3 rewrite idea is preserved in `CLAWPETCUBE_PLAN.md` as a deferred v2
option, not the current target.

<img src="assets/default.jpeg" alt="ClawPetCube default dashboard view" width="45%"/>
<img src="assets/sleepy.jpeg" alt="ClawPetCube sleepy dashboard view" width="45%"/>

## Current Status

The HAL firmware path is mostly implemented and has been brought up on the
STM32F405 board:

- ST-LINK flashing/debugging works for the STM32F405 target.
- The ST7789-style 240x240 LCD initializes and refreshes through LVGL 8.1.1.
- TIM3 drives the LVGL tick; the first-frame stall is fixed and documented.
- FreeRTOS runs the UI and ESP8266 monitor tasks.
- ESP8266 joins WiFi and syncs HKT time from HKO over HTTP.
- OpenClaw `/health`, `/status`, and `/message` are polled through a cube-specific
  plain HTTP endpoint.
- The live dashboard displays HKT time/date, a blocky mascot, a speech bubble,
  `CRON`, `HEALTH`, and a rotating detail/error card.
- Serial logs mask configured WiFi/API host secrets.

Still pending or needing longer validation:

- Fresh visual confirmation photos of the current LCD dashboard.
- Long-run dashboard stability on the physical cube.
- SDIO/FatFs mount.
- MPU6050 ID/interrupt validation.
- USART6 touch/input validation.
- Later UI polish, page switching, and optional persistence.

## Repository Layout

- `hal_firmware/`: active STM32Cube HAL firmware for ClawPetCube.
- `CLAWPETCUBE_PLAN.md`: source-of-truth product and technical plan.
- `hal_firmware/OPENCLAW_BRINGUP_JOURNAL.md`: verified OpenClaw/ESP8266 bring-up
  notes and latest serial evidence.
- `hal_firmware/LVGL_BRINGUP_JOURNAL.md`: LCD/LVGL debug history and timer fix.
- `hal_firmware/MIGRATION_STATUS.md`: HAL migration status by subsystem.
- `Middlewares/`: legacy LVGL 8.1.1 and GUI Guider sources still used by the
  HAL dashboard build.
- `elec3300/`: original CodeIgniter/PHP web UI and API.
- `Release/`: generated Eclipse/GNU MCU build output.

## Hardware

- MCU: STM32F405RGTx
- Display: 240x240 ST7789-style SPI TFT
- WiFi: ESP-12F / ESP8266 over UART
- Debug log: USART1 at 115200 through the board USB-C TTL bridge
- Storage: SD card over SDIO, not yet revalidated in the HAL path
- Sensor/input: MPU6050 over I2C and USART6 touch/input events, not yet
  revalidated in the HAL path
- Board: original self-made VPetCube PCB

## Firmware Behavior

The active UI is a compact OpenClaw operations dashboard:

- Top-left HKT time and date.
- Left-side blocky pixel mascot.
- Right-side speech bubble for pushed cube messages during normal operation.
- Serious time, network, OpenClaw health, fetch, or gateway errors override the
  speech bubble.
- Bottom cards for `CRON`, `HEALTH`, and a detail card.
- The detail card rotates `PROC`, `DISK`, and `MEM` in normal operation and
  switches to `ERROR` only for real failures.
- The top-right alert pill is hidden during normal operation and appears only
  for stale/offline/warning states.

The ESP8266 AT firmware cannot reliably connect directly to the private
Cloudflare HTTPS endpoint, so the working path is a cube-specific plain HTTP
endpoint that proxies the private OpenClaw data server-side.

Required local configuration values are intentionally not committed:

```c
#define VPC_WIFI_SSID "..."
#define VPC_WIFI_PASSWORD "..."
#define VPC_OPENCLAW_HTTP_HOST "..."
#define VPC_OPENCLAW_HTTP_PORT "80"
```

Place those in an ignored local `app_config_local.h` or define them in the
CubeIDE project symbols.

## Build

Use PowerShell with the STM32CubeIDE GNU Arm toolchain on `PATH`:

```powershell
$env:Path='C:\ST\STM32CubeIDE_1.16.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623\tools\bin;' + $env:Path
make -C Release all
```

Expected HAL outputs:

- `Release/VPetCube_HAL.elf`
- `Release/VPetCube_HAL.hex`

If the build compiles `FWLIB/src/stm32f4xx_*.c`, defines
`USE_STDPERIPH_DRIVER`, or links `VPetCube.elf`, it is using stale generated
metadata from the removed legacy SPL target instead of the HAL target.

## Legacy VPetCube Reference

The original VPetCube project was developed for HKUST ELEC3300 as a physical
virtual-pet cube. It included:

- CodeIgniter web UI in `elec3300/`.
- Work/study and feeding interactions.
- Weather fetch through ESP8266.
- Touchpad interaction through USART6.
- MPU6050 motion input work.

The old SPL firmware source tree has been removed from this checkout after the
HAL migration became the active path. The original behavior is still documented
in the project plan and bring-up journals.

## Documentation

Read these first when continuing the project:

1. `CLAWPETCUBE_PLAN.md`
2. `hal_firmware/README.md`
3. `hal_firmware/OPENCLAW_BRINGUP_JOURNAL.md`
4. `hal_firmware/LVGL_BRINGUP_JOURNAL.md`
5. `hal_firmware/MIGRATION_STATUS.md`

## License

This project is open source under the MIT License. See `LICENSE`.
