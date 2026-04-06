# ESP32VariableTuner

A library for tuning variables dynamically on ESP32 via WebUI.

## Changelog / Notes

**feat(webui): serve Bootstrap locally to enable offline usage without CDN**
- Replaced external CDN links for Bootstrap CSS and JS with local endpoints (`/css/bootstrap.css`, `/js/bootstrap.js`).
- Added downloaded and gzip-compressed Bootstrap assets directly into the library as PROGMEM char arrays (`BootstrapData.h`) to drastically minimize Flash/SRAM footprint.
- Updated `VarMonitorServer` HTTP endpoints to serve these compressed files natively with `Content-Encoding: gzip` and aggressive cache headers.
- The web interface can now gracefully load full styling even when the ESP32 operates as an Access Point (AP mode) with no internet connection.

## WebUI Development

To modify the web interface, edit the files in the `webui_test/` directory. Do not edit `src/WebUI.h` directly.

1. Navigate to the `webui_test` folder: `cd webui_test`
2. Install dependencies (first time only): `npm install`
3. Edit `index.html` as needed.
4. Run `npm run pack` to minify and automatically generate `src/WebUI.h`.

## Unit tests

Use the dedicated PlatformIO test environment: `wemos_d1_uno32_test`.

Run tests (recommended):

```powershell
C:\Users\admin\.platformio\penv\Scripts\platformio.exe test -e wemos_d1_uno32_test --upload-port COM3 -vvv
```
## Flash size optimization

If you do not need the embedded Web UI (HTML/CSS/JS served from ESP32), you can build in API/WebSocket-only mode to reduce flash usage significantly.

- Add this build flag in PlatformIO:

```ini
build_flags =
	-DVM_EMBED_WEBUI=0
```

What changes with `VM_EMBED_WEBUI=0`:
- `/api/*` and `/ws` endpoints remain available.
- Root route `/` returns a plain text API-mode message.
- Embedded UI assets are not linked, reducing flash footprint.

Build tests only (no upload):

```powershell
C:\Users\admin\.platformio\penv\Scripts\platformio.exe test -e wemos_d1_uno32_test --without-uploading -v
```

Upload test firmware only (no test run):

```powershell
C:\Users\admin\.platformio\penv\Scripts\platformio.exe run -e wemos_d1_uno32_test -t upload --upload-port COM3 -v
```

If your board has no BOOT button, press reset at the start of `Connecting...` to enter download mode.
