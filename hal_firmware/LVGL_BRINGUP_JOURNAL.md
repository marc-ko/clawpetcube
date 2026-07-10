# LVGL / ST7789 Bring-Up Journal

Date: 2026-07-11

This note records the LCD/LVGL debugging path for the STM32F405 HAL firmware.
It is meant as a practical memory for future board bring-up.

## Current Known-Good State

- OpenOCD can connect to the STM32F405 over ST-LINK/SWD at about 950 kHz.
- The ST7789-style LCD is initialized and can draw LVGL objects.
- The screen now updates once LVGL time is advanced correctly.
- Current diagnostic UI shows `UI TICK <n> s` and moves a progress bar.
- ESP8266 probing is temporarily disabled during display bring-up to avoid noisy serial retries.

## Main Root Cause

The UI task was running, but LVGL's tick was not advancing.

Evidence:

- The LCD showed the first rendered frame, such as `UI START 0` or `LOOP TEST 0`.
- Serial continued printing `UI tick 25`, `UI tick 26`, etc.
- That proved the FreeRTOS task was alive and not globally frozen.
- The screen still did not repaint after label/bar changes.

Why it happened:

- LVGL was configured with `LV_TICK_CUSTOM 0`, so it requires regular calls to `lv_tick_inc()`.
- The firmware intended to call `lv_tick_inc(1)` from `HAL_TIM_PeriodElapsedCallback()` when TIM3 fires.
- `MX_TIM3_Init()` existed, and `HAL_TIM_Base_Start_IT(&htim3)` was called.
- But there was no `HAL_TIM_Base_MspInit()` implementation for TIM3/TIM7.
- Therefore the TIM3 peripheral clock and TIM3 IRQ were not enabled.
- Result: TIM3 never generated the 1 ms interrupt, so LVGL's internal refresh timer never became due after the first draw.

Fix:

```c
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM3_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    } else if (htim->Instance == TIM7) {
        __HAL_RCC_TIM7_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
}
```

Relevant files:

- `hal_firmware/Core/Src/stm32f4xx_hal_msp.c`
- `hal_firmware/Core/Src/main.c`
- `hal_firmware/Core/Src/app_tasks.c`

## Important False Leads

### It Was Not A FreeRTOS Scheduler Freeze

At one point the screen was stuck at `LOOP TEST 0`, so `vTaskDelay()` looked suspicious.
But serial output from the ESP8266 monitor task kept printing AT retries.
Later, serial `UI tick n` messages proved the UI task loop itself was running.

Conclusion: the scheduler was alive.

### It Was Not The Debugger Breakpoint

OpenOCD printed messages like:

```text
halted due to breakpoint
undefined debug reason 8 (UNDEFINED) - target needs reset
```

Address mapping showed these were around reset/startup, not a user firmware breakpoint.
They are debugger launch behavior and were not the reason the LCD stopped updating.

### LVGL Flush Was A Risk, But Not The Final Blocking Cause

The original LVGL display flush used SPI DMA. That path can hang LVGL if `lv_disp_flush_ready()`
is not called with the exact driver pointer from the flush callback.

During bring-up we changed the LVGL flush path to blocking SPI:

- `disp_flush()` writes pixels synchronously.
- It immediately calls `lv_disp_flush_ready(drv)`.

This is slower but simpler and removes DMA callback timing as a variable.
DMA can be re-enabled later after the UI is stable.

Relevant files:

- `hal_firmware/Core/Src/lv_port_disp_hal.c`
- `hal_firmware/Board/Src/lcd_st7789_hal.c`
- `hal_firmware/Board/Inc/lcd_st7789_hal.h`

## Other Fixes Made During Bring-Up

### OpenOCD / SWD

Initial OpenOCD failed with:

```text
Error: init mode failed (unable to connect to the target)
localhost:3333: Connection timed out
```

The debug config was relaxed:

- SWD speed reduced to 1000 kHz requested, OpenOCD uses 950 kHz.
- Reset config changed away from connect-under-reset.

This allowed stable target detection:

```text
SWD DPIDR 0x2ba01477
Cortex-M4 r0p1 processor detected
flash size = 1024 KiB
```

### LCD Noise / Black Screen

The LCD initially showed random color noise.
Fixes:

- Slowed SPI1 prescaler from `/2` to `/8`.
- Cleared panel RAM to black at the end of `LCD_Init()`.
- Confirmed color bars first, then LVGL progress bar.

### LVGL FatFs Link Error

The build failed with:

```text
undefined reference to `lv_fs_fatfs_init'
```

For display bring-up, LVGL FatFs support was disabled:

```c
#define LV_USE_FS_FATFS 0
```

FatFs is for SD-card or filesystem-backed assets. It is not needed for the current basic LVGL screen.

## Debugging Rules From This Session

- If serial logs continue, do not assume FreeRTOS is dead.
- If the first LVGL frame appears but later object changes do not repaint, check LVGL tick first.
- For LVGL 8 with `LV_TICK_CUSTOM 0`, confirm `lv_tick_inc()` is called periodically.
- A valid timer handle is not enough; HAL timer MSP must enable the timer clock and NVIC.
- During LCD bring-up, prefer blocking SPI flush first. Reintroduce DMA only after the UI loop is proven.
- Keep ESP8266/network probing disabled while debugging display timing, because AT retries make logs noisy and can mask UI evidence.

## Next Steps

1. Keep the diagnostic `UI TICK` screen until one or two more flashes are stable.
2. Restore UI timing to normal FreeRTOS style:
   - `lv_timer_handler()`
   - `vTaskDelay(pdMS_TO_TICKS(10))`
3. Re-enable monitor task only after display timing remains stable.
4. Replace diagnostic UI with the generated GUI Guider init screen or desktop shell.
5. Revisit SPI DMA flush for performance after the generated UI is stable.
