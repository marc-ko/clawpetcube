# Repository Guidelines

## Project Structure & Module Organization

This repository now uses the STM32Cube HAL firmware as the active ClawPetCube
target. The old Standard Peripheral Library firmware source tree has been
removed after migration.

- `hal_firmware/`: active STM32F405 HAL application, board ports, compatibility
  shims, vendored STM32 HAL/CMSIS, FreeRTOS, and FatFs sources.
- `Middlewares/LVGL/`: legacy LVGL 8.1.1 and GUI Guider sources still referenced
  by the HAL dashboard build.
- `CLAWPETCUBE_PLAN.md`: source-of-truth product and technical plan.
- `hal_firmware/OPENCLAW_BRINGUP_JOURNAL.md`: OpenClaw/ESP8266 bring-up notes.
- `hal_firmware/LVGL_BRINGUP_JOURNAL.md`: LCD/LVGL bring-up notes.
- `hal_firmware/MIGRATION_STATUS.md`: HAL migration status by subsystem.
- `assets/`: current README/demo images.
- `Release/`: generated Eclipse/GNU MCU build output and supported make target.

## Build, Test, And Development Commands

- `make -C Release all`: build the active HAL firmware target after CubeIDE has generated the managed makefiles.
- `php -S localhost:8080 -t elec3300`: run the legacy web UI only if that app is
  restored or present locally.
- `composer install` from `elec3300/`: install PHP dependencies only when working
  on the legacy web UI.

Firmware builds require an ARM embedded GCC toolchain such as
`arm-none-eabi-gcc`. The known working Windows setup prepends the STM32CubeIDE
bundled `make.exe` and GCC 12.3 toolchain to `PATH` before running
`make -C Release all`.

Expected HAL outputs:

- `Release/ClawPetCube_HAL.elf`
- `Release/ClawPetCube_HAL.hex`

If a build compiles `FWLIB/src/stm32f4xx_*.c`, defines `USE_STDPERIPH_DRIVER`,
or links `VPetCube.elf`, it is using stale generated metadata from the removed
legacy SPL target.

## Coding Style & Naming Conventions

Keep C code close to the existing STM32 style: tabs or consistent indentation,
`u8/u16/u32` typedefs, uppercase hardware macros, and module-prefixed functions
such as `LCD_Init`, `ESP8266_GetOpenClawStatus`, and `USART2_MarkFrameDone`.
Place HAL board support code under `hal_firmware/Board/` and application logic
under `hal_firmware/Core/`.

## Testing Guidelines

There is no active automated firmware test suite. Verify firmware changes by
compiling with `make -C Release all` and, when hardware is available,
flashing/running on the target board. For UI/display changes, include LCD
photos or serial evidence where possible.

## Commit & Pull Request Guidelines

Use short imperative commit messages such as `fix: recover ESP8266 TCP state` or
`docs: update HAL bring-up status`. Pull requests should describe firmware and
documentation changes separately, list hardware assumptions, mention tested
commands, and include screenshots or device photos for UI/display changes.

## Security & Configuration Tips

Do not commit real WiFi credentials, API keys, or production hostnames. Keep
local values in ignored `hal_firmware/Core/Inc/app_config_local.h` or CubeIDE
project symbols.
