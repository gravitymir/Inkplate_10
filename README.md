# Cleere Auto Services — Inkplate 10 Clock

A minimal **NTP-less wall clock** for the [Soldered Inkplate 10](https://soldered.com/products/inkplate-10)
(9.7", 1200×825, 3-bit grayscale e-paper).

It shows the time and date during working hours, and the **Cleere Auto Services** logo
overnight. There is no hard-coded WiFi: on power-on the device opens a short-lived
access point with a web page where you set the working hours and synchronize the clock
straight from your phone/laptop browser.

---

## Features

- **Time + date clock**, refreshed once per minute, driven by the on-board RTC.
- **Configurable working hours** (default `08:00`–`23:00`), stored in flash (NVS) so they
  survive reboots and power loss.
- **WiFi setup portal on power-on** (access point + web page) for **~2 minutes**, then
  WiFi is turned off completely to save battery.
- **Browser time sync** — no NTP/internet needed; the RTC is set from the browser clock.
- **Sleep logo** — outside working hours the Cleere Auto Services logo is shown and the
  device deep-sleeps. The e-paper keeps the image on screen at **zero power**.
- **Branded setup screen** — the logo with the setup instructions laid over it.

---

## How it works

### On power-on / reset (cold boot)
1. The panel shows the **WiFi setup screen** (logo + instructions).
2. The device starts an access point:
   - **SSID:** `Inkplate-Clock`
   - **Password:** `clock1234`
3. Connect a phone/laptop to it and open **`http://192.168.4.1`**.
4. On the page you can:
   - Set **Work start (hour)** and **Work end (hour)** → *Save work time* (persisted to NVS).
   - Tap **Synchronize time with this browser** → sets the device RTC from your browser's
     current date/time.
5. After **~2 minutes** the access point and WiFi shut off and the clock takes over.

### Normal running
- From `workStart:00` until `workEnd:00` the clock shows time + date and updates every minute.
  `workEnd:00` (e.g. `23:00`) is the last clock minute shown.
- From `workEnd:01` (e.g. `23:01`) the logo is shown and the device deep-sleeps until
  `workStart` (e.g. `08:00`), then wakes and resumes the clock.
- The **physical button (GPIO36)** also wakes the device.
- The setup AP only opens on a real power-on/reset — **not** on the scheduled deep-sleep
  wake — so it does not waste battery every morning.

> After a full power loss the RTC may lose the time. Open the setup portal and tap
> *Synchronize* once to set it again.

---

## Project layout

```
soldered_clock/
├─ platformio.ini        # Inkplate library pinned to 6.0.0
├─ README.md
├─ src/
│  ├─ main.cpp           # boot flow, clock loop, sleep logo
│  ├─ portal.h           # WiFi AP + web config page + browser time sync
│  ├─ auxilary.h         # showTime() clock layout + timeToSleep()
│  ├─ display.h          # Inkplate object + fonts used by the clock
│  ├─ logo.h             # generated 3-bit grayscale logo bitmap
│  └─ Fonts/             # only the fonts the clock uses
└─ tools/
   ├─ gen_logo.py        # regenerate src/logo.h from logo_src.jpg
   └─ logo_src.jpg       # source logo image
```

## Configuration

| Setting | Where |
| --- | --- |
| AP SSID / password | `AP_SSID` / `AP_PASS` in `src/portal.h` |
| Setup window length | `PORTAL_DURATION_MS` in `src/portal.h` (currently 2 min) |
| Default working hours | defaults in `src/portal.h` / set via the web page |
| Logo image | `tools/logo_src.jpg` |

## Build & flash

Requires [PlatformIO](https://platformio.org/).

```bash
# build
pio run

# flash (replace COMx with your serial port)
pio run --target upload --upload-port COMx
```

On Windows the board may need manual boot mode: hold the **GPIO0 / PROG** button while the
uploader prints `Connecting...`, then release.

> **Note:** the Inkplate Arduino library is pinned to `6.0.0` in `platformio.ini`. Newer
> versions changed the drawing/RTC API and will not compile this code.

## Regenerating the logo

Replace `tools/logo_src.jpg` and run:

```bash
python tools/gen_logo.py            # writes src/logo.h (1200×600, 3-bit grayscale)
```

Pillow is required (`pip install Pillow`).
