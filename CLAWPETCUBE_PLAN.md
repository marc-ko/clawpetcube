# ClawPetCube Plan

Last updated: 2026-07-11

ClawPetCube is a physical, always-on desk monitor for OpenClaw. It repurposes the existing VPetCube hardware into a useful status device instead of a decorative virtual pet. The original plan considered replacing the board with an ESP32-S3, but the current hardware bring-up has proven that the existing STM32F405 board can run the LCD and LVGL reliably enough to continue.

## Product Goal

Build a compact desk gadget that shows OpenClaw service health and usage at a glance.

The screen should answer these questions without opening a browser:

- Is OpenClaw online, degraded, or offline?
- How much work did it process today?
- Are users hitting errors?
- Is latency acceptable?
- Which subsystem is failing if something is wrong?
- When was the status last updated?

The device should still feel like a small companion object, but its primary value is operational monitoring.

## Mascot Direction

Use the simple blocky pixel mascot from the provided reference image:

- salmon/orange rectangular body
- black vertical eyes
- short square side claws
- short square legs
- black background

The mascot should stay deliberately minimal so it reads clearly on the 240x240 LCD and can be drawn cheaply with LVGL rectangles before committing to image assets. Keep it smaller and left-weighted so the right side can hold a compact speech bubble. Operational status belongs below the mascot/bubble area, not inside the bubble.

## Scope Decision

### Active Plan: Keep The Existing STM32 Cube

Use the existing VPetCube-style hardware as the active development target:

- MCU: STM32F405RGTx
- Display: 240x240 ST7789-style SPI TFT
- WiFi: ESP-12F / ESP8266 over UART
- UI: LVGL 8.1.1 from the legacy tree
- RTOS: FreeRTOS
- Build: STM32CubeIDE / GNU Arm toolchain
- Firmware tree: `hal_firmware/`

Reason:

- The board already exists and is physically assembled.
- ST-LINK flashing/debugging works.
- LCD init and LVGL updates are now proven on hardware.
- Keeping the board avoids a hardware rewire before the product direction is validated.

### Deferred Option: ESP32-S3 Rebrain

The earlier reference plan proposed ESP32-S3 + LovyanGFX + LVGL v9. That remains a possible v2 or fallback path if the STM32/ESP8266 stack becomes too costly.

ESP32-S3 would simplify:

- WiFi
- NTP
- HTTPS/HTTP client
- OTA
- filesystem persistence
- modern LVGL tooling

But it is not the active path right now.

## Current Hardware Status

Known good:

- ST-LINK/OpenOCD connects to STM32F405 at about 950 kHz SWD.
- Firmware flashes through CubeIDE/OpenOCD.
- LCD reset/backlight/init works.
- ST7789 panel clears correctly.
- LVGL draws and updates on-screen.
- FreeRTOS task loop is alive.
- TIM3 now drives LVGL tick correctly.
- ESP8266 joins WiFi and fetches HKO time over HTTP.
- OpenClaw health/status fetch works through the configured cube HTTP endpoint.
- Serial logs mask local WiFi/API host commands instead of printing configured secrets or hostnames.

Still to validate:

- SDIO/FatFs mount.
- MPU6050 ID and interrupt.
- USART6 touch/input events.
- Long-run display stability.

## Current Firmware Status

The active firmware path is:

- `hal_firmware/Core/Src/app_tasks.c`
- `hal_firmware/Core/Src/main.c`
- `hal_firmware/Core/Src/lv_port_disp_hal.c`
- `hal_firmware/Board/Src/lcd_st7789_hal.c`
- `hal_firmware/Core/Src/stm32f4xx_hal_msp.c`

Current UI:

- Local LVGL OpenClaw dashboard.
- Shows real HKT time from ESP8266/HKO sync, subtle date, chosen blocky pixel mascot, and a right-side speech bubble.
- The speech bubble normally shows the latest pushed cube message. Serious time/network/OpenClaw errors override it.
- Pushed cube messages are keyed by the response `timestamp` and stay visible only while that server timestamp is less than about 45 minutes old. Expired messages hide the speech bubble instead of leaving stale text on-screen.
- Top-right alert pill is hidden during normal operation; it appears only for time/network stale/offline or OpenClaw health/fetch/gateway errors.
- Three compact cards sit below the mascot/bubble area:
  - `CRON`: ok/total count, blue normally and amber when cron has failures
  - `HEALTH`: current health text, green when `alive` and red on health/gateway failure
  - bottom-right detail/error card: rotates `PROC`, `DISK`, and `MEM` every 2 seconds during normal operation; disk/mem values include `%`; switches to `ERROR` on gateway/fetch/health failures
- Updates once per second.
- Uses blocking SPI flush for stability.

Known important fix:

- `HAL_TIM_Base_MspInit()` was missing.
- TIM3 clock/IRQ were not enabled.
- LVGL first frame rendered, but later updates did not refresh because `lv_tick_inc()` was never called.
- This is documented in `hal_firmware/LVGL_BRINGUP_JOURNAL.md`.

## Build And Flash

Use PowerShell:

```powershell
$env:Path='C:\ST\STM32CubeIDE_1.16.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623\tools\bin;' + $env:Path
make -C Release all
```

Output:

- `Release/VPetCube_HAL.elf`
- `Release/VPetCube_HAL.hex`

Important:

- Use `make -C Release all`.
- Do not use plain `make -C Release`; the generated makefile behavior can be misleading.
- If CubeIDE compiles `FWLIB/` or links `VPetCube.elf`, it is building the legacy SPL target, not the HAL target.

## Target User Experience

The final screen should be a compact operations dashboard, not a toy-only pet screen.

Primary screen:

- Top-left HKT time, readable but not oversized.
- Date shown subtly below the time.
- Small status indicator:
  - ONLINE: green
  - SYNC/WIFI: blue
  - STALE: yellow
  - OFFLINE/UNSYNCED: red
- Minimal blocky pixel mascot as the companion/status character.
- Brand/title should be omitted or very small when space is tight.
- Later OpenClaw dashboard status:
  - Large status indicator:
  - ONLINE: green
  - DEGRADED: yellow
  - OFFLINE: red
- Today summary:
  - sessions
  - files processed
  - errors
  - success rate
  - average latency
- Last update time or last error only when it matters; avoid routine sync noise on the small screen.
- Mascot expression/color can change based on status.

Secondary screen or cycling panel:

- Backend health
- AI/model health
- storage/queue health
- recent failures
- small trend chart or sparkline

Idle behavior:

- If OpenClaw is healthy, the device should feel calm and readable.
- If degraded/offline, the visual status should be obvious from a distance.
- If network is down, keep last known data and show stale/offline state.

## OpenClaw Data Contract

OpenClaw should expose a lightweight JSON endpoint, either directly or through a sidecar exporter.

Important ESP8266 constraint:

- The current ESP8266 AT firmware rejects direct SSL to the private OpenClaw host.
- The normal private OpenClaw HTTP endpoint redirects to HTTPS through Cloudflare.
- Therefore direct polling from the STM32/ESP8266 uses a cube-specific plain HTTP Cloudflare endpoint.
- Firmware has config hooks for this endpoint: `VPC_OPENCLAW_HTTP_HOST` and `VPC_OPENCLAW_HTTP_PORT`.
- Current verified endpoint is configured only in ignored local config.
- Hardware COM6 verification passed with `OpenClaw health: alive` and `OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=41`.

Current endpoint:

```http
GET /health
GET /status
GET /message
```

`/message` returns the current speech-bubble message:

```json
{
  "message": "short text for the cube",
  "from": "Pomu",
  "id": 2,
  "timestamp": "2026-07-12T13:36:00+08:00"
}
```

The firmware ignores `id` for display lifetime and uses `timestamp` as the message identity.

Proposed JSON:

```json
{
  "status": "online",
  "updated_at": "2026-07-11T12:34:56+08:00",
  "today": {
    "sessions": 142,
    "files": 87,
    "errors": 3
  },
  "health": {
    "success_rate": 98.7,
    "avg_latency_ms": 245
  },
  "components": {
    "backend": "ok",
    "ai_model": "ok",
    "storage": "ok",
    "queue": "ok"
  },
  "last_error": ""
}
```

Offline behavior:

- Request timeout or connection failure means `offline`.
- Keep last known stats in memory.
- Later persist last known good data to SD/FatFs or internal flash if needed.
- Show the error text if it fits; otherwise show a short error class.

## Firmware Architecture

Suggested task split:

- UI task:
  - owns all LVGL object creation and updates
  - calls `lv_timer_handler()`
  - does not block on WiFi/network calls
- Network task:
  - talks to ESP8266 over USART2
  - polls OpenClaw stats
  - parses JSON or compact line protocol
  - publishes latest status to shared state/queue
- Input task:
  - handles USART6 touchpad/button events
  - changes screen/page/mode
- Sensor task:
  - optional MPU6050 motion input
- Persistence task:
  - optional SD/FatFs cache and logs

The UI task and ESP8266 monitor task are active. Keep other peripheral tasks off until the live dashboard has run cleanly on hardware.

## UI Implementation Plan

Phase 1: Stable Local LVGL Shell

- Keep local desktop shell in `app_tasks.c`.
- Confirm it updates for several minutes.
- Confirm no display corruption or hard fault.
- Keep blocking SPI flush.

Phase 2: OpenClaw Dashboard Layout

- Live dashboard cards are now implemented locally in `app_tasks.c`.
- Current widgets:
  - top-left HKT time/date
  - top-right network pill
  - left-side animated pixel mascot
  - right-side speech bubble
  - bottom compact cards for cron, health, and recent error
- Keep all UI code local until the layout is stable on the physical LCD.

Phase 3: Generated UI Integration

- Bring in GUI Guider screens gradually.
- Do not include the full `Middlewares/LVGL/guiguider` folder all at once.
- Full generated desktop currently pulls weather/user/RTC/SD image dependencies.
- Add one screen or one widget group at a time and verify link/build after each step.

Phase 4: Live Data

- ESP8266 WiFi, HKO time sync, OpenClaw health, OpenClaw status, and cube message polling are active.
- Parsed status is fed into the UI through shared text/state fields guarded by FreeRTOS critical sections.
- Parsed cube messages feed the right-side speech bubble unless a serious alert is active.
- Offline/stale state handling exists at a basic level; refine it after long-run testing.

Phase 5: Polish

- Add animations.
- Add status color themes.
- Add page switching with touch/buttons.
- Add SD/FatFs cache if needed.
- Revisit SPI DMA flush for performance.

## Backend / Exporter Plan

If OpenClaw does not already have a stats API, add a small exporter:

Options:

- FastAPI sidecar
- Flask sidecar
- direct endpoint inside OpenClaw server
- nginx-served generated JSON file if the first version should be very simple

Exporter responsibilities:

- Track daily sessions/files/errors.
- Track success/failure count.
- Track average latency.
- Track last error.
- Report component health.
- Return compact JSON.

Security:

- Keep it LAN-only at first.
- Optional shared token header later.
- Do not expose it publicly without auth.

## Hardware Bring-Up Checklist

Already passed:

- ST-LINK connects.
- STM32F405 detected.
- Flash works.
- LCD clears and draws.
- LVGL tick/render loop works.

Next:

- Visually confirm the live OpenClaw bubble/status cards and pushed message behavior on the LCD after the latest flash.
- Run the dashboard for 10-30 minutes and watch for stale status or display artifacts.
- Confirm SDIO card detect/mount.
- Confirm MPU6050 ID.
- Confirm USART6 input events.

## Known Risks

- ESP8266 direct HTTPS to Cloudflare is not viable on this AT firmware; the cube-specific plain HTTP endpoint is the current transport.
- Blocking SPI flush may be too slow for complex UI, but it is acceptable for bring-up.
- GUI Guider generated code has legacy dependencies that can cause link errors.
- CubeIDE may regenerate `Release/` makefiles or object lists.
- FatFs/LVGL filesystem support is disabled for now.
- Current UI is LVGL 8.1.1, not LVGL 9.

## Repo Docs

- `hal_firmware/README.md`: HAL target and hardware checklist.
- `hal_firmware/MIGRATION_STATUS.md`: migration status.
- `hal_firmware/LVGL_BRINGUP_JOURNAL.md`: LCD/LVGL debug history.
- `AGENTS.md`: repository conventions.

## Immediate Next Step

Visually confirm the flashed live dashboard on hardware.

Expected:

- HKT time/date in the top-left.
- Right-side area is a speech bubble, not a status card.
- Bottom compact cards show `CRON`, `HEALTH`, and a rotating detail/error card.
- The bottom-right card cycles `PROC`, `DISK`, and `MEM` during normal operation, then switches to `ERROR` only on failures. Cron stays only in the `CRON` card.
- Top-right pill appears only for stale/offline time/network, health not `alive`, or a fetch/gateway error. Cron status stays in the `CRON` card.
- Mascot floats on the left without colliding with the cards.

If stable, the next development task is a small design pass for the speech copy and then wiring more OpenClaw fields only if they fit cleanly.
