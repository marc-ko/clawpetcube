# OpenClaw ESP8266 Bring-Up Journal

Last updated: 2026-07-11

## Goal

Feed live OpenClaw health/status into the 240x240 LVGL dashboard through the existing STM32F405 + ESP8266 AT path.

## Verified Working

- ESP8266 joins WiFi.
- HKO HTTP time sync works through plain TCP port 80.
- STM32 RTC is updated from HKO time.
- PC-side OpenClaw HTTPS endpoints work:
  - private OpenClaw `/health`
  - private OpenClaw `/status`
- Cloudflare cube endpoint works over plain HTTP without redirect:
  - configured cube `/health`
  - configured cube `/status`
- Firmware points the ESP8266 plain TCP client at:
  - `VPC_OPENCLAW_HTTP_HOST` from ignored local config
  - `VPC_OPENCLAW_HTTP_PORT "80"`
- Hardware COM6 verification passed. These values are parsed from live OpenClaw status, not dummy UI data:

```text
HKT time: 2026-07-11 17:01:05 week 6
RTC sync OK
AT+CIPSTART="TCP","<cube-http-host>",80
OpenClaw health: alive
AT+CIPSTART="TCP","<cube-http-host>",80
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=41
```

Latest post-flash verification after the empty-card UI fix:

```text
HKT time: 2026-07-11 17:34:59 week 6
RTC sync OK
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw health: alive
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=40
```

Latest post-flash verification after the speech-bubble/status-row layout:

```text
HKT time: 2026-07-11 18:11:18 week 6
RTC sync OK
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw health: alive
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=40
```

Latest post-flash verification after colored compact cards:

```text
HKT time: 2026-07-11 18:25:31 week 6
RTC sync OK
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw health: alive
ESP8266 cmd: AT+CIPSTART=<configured>
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=39
```

Latest post-flash verification after bottom-right slideshow card:

```text
HKT time: 2026-07-11 18:35:10 week 6
RTC sync OK
OpenClaw health: alive
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=39
```

Latest post-flash verification after empty-normal speech bubble and percent disk/mem:

```text
HKT time: 2026-07-11 18:41:37 week 6
RTC sync OK
OpenClaw health: alive
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=40
```

Latest post-flash verification after cube message polling:

```text
HKT time: 2026-07-12 13:36:21 week 7
RTC sync OK
OpenClaw health: alive
OpenClaw message: received
OpenClaw status: gw=1 proc=3 cron=9/13 disk=70 mem=40
OpenClaw health: alive
OpenClaw message: received
```

Latest post-flash verification after timestamp-based expiry and UART message text:

```text
OpenClaw health: alive
OpenClaw message: received
OpenClaw message (expired) age=848m from Pomu: Good night Marco! ???? Don't stay up too late~
OpenClaw status: gw=1 proc=3 cron=9/13 disk=70 mem=40
```

## ESP8266 TLS Result

Direct ESP8266 SSL to Cloudflare is not currently usable.

Observed on hardware:

```text
AT+CIPSTART="SSL","<private-openclaw-host>",443
ERROR
CLOSED
```

Additional TLS setup was tested:

- `AT+CIPSSLSIZE=4096` accepted.
- `AT+CIPSSLCCONF=0` accepted.
- `AT+CIPSSLCCONF=0,0,0` unsupported on this firmware.
- `AT+CIPSSLCSNI="<private-openclaw-host>"` unsupported on this firmware.

Conclusion: this ESP8266 AT firmware cannot reliably connect to the Cloudflare HTTPS endpoint. The fixed path is a cube-specific Cloudflare Worker/route that serves plain HTTP on port 80 and fetches the real HTTPS OpenClaw API server-side.

## Fixed HTTP Path

Firmware supports:

```c
VPC_OPENCLAW_HTTP_HOST
VPC_OPENCLAW_HTTP_PORT
```

The ignored local config currently points to the Cloudflare cube endpoint:

```c
#define VPC_OPENCLAW_HTTP_HOST "<cube-http-host>"
#define VPC_OPENCLAW_HTTP_PORT "80"
```

PC-side endpoint tests pass:

```powershell
curl.exe -i http://<cube-http-host>/health
curl.exe -i http://<cube-http-host>/status
```

Important Cloudflare requirement: `http://<cube-http-host>/...` must return HTTP 200 directly. It must not redirect to HTTPS, because the ESP8266 AT firmware cannot complete modern Cloudflare HTTPS.

## UI State

The dashboard no longer shows the stale `CLAW TLS / Need proxy` state for the normal path.

The current screen uses a compact LVGL dashboard:

- top-left HKT time/date
- left-side animated pixel mascot
- right-side speech bubble; it shows pushed cube messages during normal operation and short alert text for time/network/health/fetch/gateway failures
- bottom compact cards: `CRON`, `HEALTH`, and a bottom-right detail/error card
- compact cards have tinted backgrounds, colored borders, and colored value text: cron blue/amber, health green/red, error neutral/red
- bottom-right card rotates `PROC`, `DISK`, and `MEM` every 2 seconds during normal operation; disk/mem values include `%`; it switches to `ERROR` on gateway/fetch/health failures
- top-right alert pill appears only for stale/offline time/network, health not `alive`, or a fetch/gateway error; cron status stays inside the `CRON` card
- right-side speech bubble shows the latest pushed cube message during normal operation; time/network/OpenClaw alerts override it

Current readable states:

- `CLAW TLS` / `Need proxy`: direct HTTPS path failed and no HTTP proxy is configured.
- `PROXY ERR` / `TCP fail`: HTTP proxy is configured, but ESP8266 cannot connect to it.
- Normal: top-right alert is hidden; cron state is shown in the `CRON` card.
- Alert: top-right pill appears for stale/offline time/network, health not `alive`, or a fetch/gateway error.

## Firmware Fixes Made

- Increased ESP8266 RX buffer to 4096 bytes.
- Added HTTP host/port config hooks.
- Added compact JSON boolean parsing for both `"ok":true` and `"ok": true`.
- Preserved the HTTP response buffer for parsing instead of clearing it with post-request `AT+CIPCLOSE`.
- Fixed `esp8266_send_data()` to discard early non-matching frames such as `SEND OK` and keep waiting for the HTTP response body.
- Increased OpenClaw status wait time to 45 seconds because `/status` can take more than 10 seconds.
- Masked sensitive ESP8266 debug commands in serial logs, including WiFi join, configured TCP/SSL host, and SSL SNI.
- Made card placeholders explicit (`WAIT`, `...`, `--`) until real status data arrives.
- Reworked the UI so the right side is a speech bubble and operational data is below it as `CRON`, `HEALTH`, and a rotating detail/error card.
- Decoupled cron state from the top-right alert; cron can color the `CRON` card without changing the corner status.
- Added colored compact card styling while keeping the top-right alert reserved for actual health/fetch/gateway problems.
- Added bottom-right slideshow behavior for normal operation: `PROC`, `DISK`, and `MEM`, with `ERROR` reserved for real failures. Cron stays only in the `CRON` card.
- Changed the speech bubble from empty-normal to message-normal; real errors still override it with a short message.
- Added `%` suffixes to disk and memory values in the slideshow.
- Added configured cube `/message` polling every 30 seconds.
- Added timestamp-based message display lifetime: the message endpoint `timestamp` is compared against RTC HKT time, and messages older than 45 minutes are treated as expired.
- Added UART message text logging with `(fresh)` or `(expired)` prefix and age in minutes.
- Hidden the whole speech bubble when the current message is expired and no alert text is active.
- Kept message fetch failures out of the main UI alert path so the bubble does not become noisy.

## Current Next Step

Ask the user to visually confirm the LCD now shows:

- right-side speech bubble, not a status card
- pushed cube message in the speech bubble during normal operation
- bottom `CRON` card with ok/total
- bottom `HEALTH` card, normally `alive`
- bottom-right card rotating `PROC`, `DISK`, and `MEM` every 2 seconds
- bottom-right card switching to `ERROR` only on gateway/fetch/health failure
- top-right alert pill hidden unless stale/offline, health is not `alive`, or a fetch/gateway error occurs
