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
- Hardware COM6 verification passed:

```text
HKT time: 2026-07-11 17:01:05 week 6
RTC sync OK
AT+CIPSTART="TCP","<cube-http-host>",80
OpenClaw health: alive
AT+CIPSTART="TCP","<cube-http-host>",80
OpenClaw status: gw=1 proc=3 cron=8/13 disk=70 mem=41
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

Current readable states:

- `CLAW TLS` / `Need proxy`: direct HTTPS path failed and no HTTP proxy is configured.
- `PROXY ERR` / `TCP fail`: HTTP proxy is configured, but ESP8266 cannot connect to it.
- `CLAW OK`: OpenClaw health/status parsed successfully.
- `CLAW WARN`: OpenClaw gateway is up but cron failures are reported.

## Firmware Fixes Made

- Increased ESP8266 RX buffer to 4096 bytes.
- Added HTTP host/port config hooks.
- Added compact JSON boolean parsing for both `"ok":true` and `"ok": true`.
- Preserved the HTTP response buffer for parsing instead of clearing it with post-request `AT+CIPCLOSE`.
- Fixed `esp8266_send_data()` to discard early non-matching frames such as `SEND OK` and keep waiting for the HTTP response body.
- Increased OpenClaw status wait time to 45 seconds because `/status` can take more than 10 seconds.

## Current Next Step

Ask the user to visually confirm the LCD right side now shows either:

- `CLAW OK` with `alive`, then
- `CLAW WARN` with compact stats like `P3 Cron 8/13` if cron failures are present.
