# Duchamp Widevine L1 Fix

KernelSU / Magisk / APatch module that fixes Netflix falling back to Widevine L3 on the **Poco X6 Pro (duchamp)**, even though the device supports L1.

Works on HyperOS / stock ROM with an unlocked bootloader and root.

## The problem

Netflix rejects Widevine system ID `30770` and falls back to L3, even on a completely untouched device.

## How it works

This is a systemless port of [this commit](https://github.com/Saikrishna1504/device_xiaomi_duchamp/commit/094a6a3d2571d6676cd6bb8056f596518c0f9390) from the AOSP device tree.

- A small shim library hooks `AParcel_writeString` in the Widevine DRM service and reports `30771` instead of `30770`.
- On install, the module copies your `/vendor/lib64/liboemcrypto.so` and adds the shim as a dependency (using the bundled arm64 `patchelf`). The patched copy is overlaid, so the vendor partition is never modified.
- On every boot, `post-fs-data.sh` compares the hash of the stock file and re-patches it if it changed (e.g. after an OTA).

Unlike the original commit, the shim finds the hook slot by symbol name instead of a hardcoded offset, so it does not depend on one specific binary build. The original offset is kept as a fallback.

## Install

1. Download `duchamp_widevine_l1.zip` from [Releases](../../releases).
2. Flash it in your root manager (KernelSU, Magisk or APatch).
3. Reboot.
4. Clear Netflix data (Settings → Apps → Netflix → Storage → Clear data).
5. Open Netflix → Settings → Playback Specification. You should see `Widevine: L1` and `System ID: 30771`.

## Uninstall

Remove the module in your root manager and reboot.

## Requirements

- Poco X6 Pro (duchamp), arm64
- Root with module support
- `/vendor/lib64/liboemcrypto.so` present

## Notes

- This only changes the **system ID reported to apps**. It does not give L1 to a device that does not support it.
- Other devices with the same Netflix issue may have a different system ID. The module will do nothing unless it is `30770`. Change the IDs in `src/liboemcrypto_shim.c` and rebuild if you want to experiment.
- If the patched library fails to load, Widevine may stop working. Remove the module to restore stock behavior.
- Tested on HyperOS 3.

## Build from source

Needs `clang` with `lld`, `patchelf` and `zip` on the host:

```sh
./build.sh
```

The output is `duchamp_widevine_l1.zip`.

## Credits

- [@Saikrishna1504](https://github.com/Saikrishna1504) for the original commit and finding the cause
- [patchelf](https://github.com/NixOS/patchelf) (GPL-3.0), bundled as a prebuilt arm64 binary

## Disclaimer

Use at your own risk. Not affiliated with Xiaomi, Netflix or Google.

---

## Bahasa Indonesia

Module untuk memperbaiki Netflix yang turun ke Widevine L3 di Poco X6 Pro (duchamp). Netflix menolak system ID `30770`, jadi module ini melaporkannya sebagai `30771` secara systemless.

**Cara pakai:** flash lewat root manager, reboot, clear data Netflix. Cek di Netflix → Settings → Playback Specification.

Module ini hanya mengubah system ID yang dilaporkan, bukan membuat HP yang tidak mendukung L1 menjadi L1.
