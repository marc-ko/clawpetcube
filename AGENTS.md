# Repository Guidelines

## Project Structure & Module Organization

This repository contains the legacy VPetCube firmware and web UI that will inform the ClawPetCube rewrite.

- `USER/`: STM32F405 application entry point, interrupts, and system setup.
- `HARDWARE/`: board drivers for LCD/ST7789-style SPI TFT, ESP8266 AT UART, MPU6050, SDIO, RTC, ADC, timers, and USART6 external inputs.
- `SYSTEM/`, `CORE/`, `FWLIB/`, `Startup/`: STM32 support code, startup assembly, and Standard Peripheral Library files.
- `FreeRTOS/`, `Middlewares/`: RTOS, FatFs, LVGL 8.1.1, GUI Guider screens, fonts, and generated image assets.
- `elec3300/`: CodeIgniter/PHP web app and API used by the original cube.
- `Release/`: generated Eclipse/GNU MCU build output; avoid hand-editing unless regenerating the project.

## Build, Test, and Development Commands

- `cmake -S . -B build`: configure the firmware project from `CMakeLists.txt`.
- `cmake --build build`: build the STM32 firmware after configuration.
- `make -C Release`: build using the generated GNU MCU Eclipse makefiles, if the STM32 toolchain and `make` are installed.
- `php -S localhost:8080 -t elec3300`: run the legacy web UI locally for quick inspection.
- `composer install` from `elec3300/`: install PHP dependencies when working on the CodeIgniter app.

Firmware builds require an ARM embedded GCC toolchain such as `arm-none-eabi-gcc`.

## Coding Style & Naming Conventions

Keep C code close to the existing STM32 style: tabs or consistent indentation, `u8/u16/u32` typedefs, uppercase hardware macros, and module-prefixed functions such as `LCD_Init`, `ESP8266_GetUserInfo`, and `usart6_init`. Place driver code under the matching `HARDWARE/<MODULE>/` folder. For PHP, follow the existing CodeIgniter controller/model structure and concise method names.

## Testing Guidelines

There is no active automated firmware test suite. Verify firmware changes by compiling and, when hardware is available, flashing/running on the target board. For PHP changes, exercise affected routes manually, especially `elec3300/application/controllers/Api.php`.

## Commit & Pull Request Guidelines

The current history only shows `add: init`; use short imperative commit messages such as `fix: correct LCD pin mapping` or `docs: add contributor guide`. Pull requests should describe firmware/web changes separately, list hardware assumptions, mention tested commands, and include screenshots or device photos for UI/display changes.

## Security & Configuration Tips

Do not commit real WiFi credentials, API keys, or production hostnames. Existing secrets in legacy files should be treated as historical data and replaced with local configuration before reuse.
