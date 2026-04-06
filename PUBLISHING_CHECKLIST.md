# PlatformIO Registry publishing checklist

## 1) Required files (already present)

- `library.json`
- `library.properties`
- `README.md`
- `LICENSE`
- `src/` and at least one example in `examples/`

## 2) Must update before publish

- Replace placeholder repository URL in:
  - `library.properties` (`url=`)
  - `library.json` (`repository.url`)
- Ensure `version` is semantic and matches release tag (e.g. `1.0.0` and Git tag `v1.0.0`).
- Verify public API include compiles from a clean test project.

## 3) Validate manifest locally

From this folder (`lib/ESP32VariableTuner`):

```powershell
pio pkg pack
```

This should generate a distributable archive without manifest errors.

## 4) Publish to PlatformIO Registry

Login first:

```powershell
pio account login
```

Publish:

```powershell
pio pkg publish --owner <your-platformio-owner>
```

If publishing from CI/tag pipeline, run the same command after creating a release tag.

## 5) Post-publish quick checks

- Confirm package page appears in PlatformIO Registry.
- Install in a fresh test project using `lib_deps` from registry name.
- Build for one ESP32 target (`wemos_d1_uno32`) to verify dependency resolution.
