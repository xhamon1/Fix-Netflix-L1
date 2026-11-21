# Fix Netflix L1

KernelSU / Magisk / APatch module that fixes Netflix falling back to Widevine L3 on devices that actually support L1.

Originally made for the **Poco X6 Pro (duchamp)**, but it may also work on other devices with the same problem (see [Other devices](#other-devices)).

## What it fixes

Widevine drops to L3 in Netflix after the app is closed and opened again.

This module works on both stock and custom ROMs:
- HyperOS (stock or modded)
- AOSP-based ROMs that don't include a built-in Netflix spoof

AOSP ROMs that already include the fix (e.g. Axion & Infinity X November Update) don't need this module.

**Tested on duchamp with:**
- HyperOS 3
- PixelOS (AOSP)

## Other devices

Although this module was originally made for Duchamp, it may also work on other devices that have a similar issue. Feel free to test it, it might just work for you.

It only takes effect if your device reports Widevine system ID `30770` (check it in Netflix → Settings → Playback Specification, or by clearing data and reopening the app). If your device reports a different ID, the module does nothing until you change the IDs in `liboemcrypto_shim.c` and rebuild.

Results on other devices are welcome, please open an issue and tell us what you tested.

## How it works

Netflix rejects Widevine system ID `30770` and falls back to L3. This module reports the system ID as `30771` instead, which Netflix accepts.

This is a systemless port of [this commit](https://github.com/Saikrishna1504/device_xiaomi_duchamp/commit/094a6a3d2571d6676cd6bb8056f596518c0f9390) from the AOSP device tree:

- A small shim library hooks `AParcel_writeString` in the Widevine DRM service and swaps `30770` for `30771`.
- On install, the module copies your `/vendor/lib64/liboemcrypto.so` and adds the shim as a dependency (using the bundled arm64 `patchelf`). The patched copy is overlaid, so the vendor partition is never modified.
- On every boot, `post-fs-data.sh` compares the hash of the stock file and re-patches it if it changed (e.g. after an OTA).

Unlike the original commit, the shim finds the hook slot by symbol name instead of a hardcoded offset, so it doesn't depend on one specific binary build. The original offset is kept as a fallback.

## Install

1. Download `fix_netflix_l1.zip` from [Releases](../../releases).
2. Flash it in your root manager (KernelSU, Magisk or APatch).
3. Reboot.
4. Clear Netflix data (Settings → Apps → Netflix → Storage → Clear data).
5. Open Netflix → Settings → Playback Specification. You should see `Widevine: L1` and `System ID: 30771`.

## Uninstall

Remove the module in your root manager and reboot.

## Requirements

- arm64 device that supports Widevine L1
- Root with module support
- `/vendor/lib64/liboemcrypto.so` present

## Notes

- This only changes the **system ID reported to apps**. It does not give L1 to a device that doesn't support it.
- If the patched library fails to load, Widevine may stop working. Remove the module to restore stock behavior.

## Build from source

Needs `clang` with `lld`, `patchelf` and `zip` on the host:

```sh
./build.sh
```

The output is `fix_netflix_l1.zip`.

## Credits

- [@Saikrishna1504](https://github.com/Saikrishna1504) for the original commit and finding the cause
- [patchelf](https://github.com/NixOS/patchelf) (GPL-3.0), bundled as a prebuilt arm64 binary

## Disclaimer

Use at your own risk. Not affiliated with Xiaomi, Netflix or Google.

---

## Bahasa Indonesia

Module untuk memperbaiki Widevine yang turun ke L3 di Netflix pada perangkat yang sebenarnya mendukung L1, yaitu setelah aplikasi ditutup lalu dibuka lagi.

Walaupun module ini awalnya dibuat untuk Duchamp, kemungkinan module ini juga bekerja di perangkat lain yang mengalami masalah serupa. Kamu bisa coba tes, siapa tahu berhasil. Module hanya berpengaruh kalau system ID perangkatmu `30770`.

**Bisa dipakai di:** stock atau custom ROM, termasuk HyperOS (stock atau mod) dan AOSP yang tidak punya fitur spoof Netflix bawaan.

**Sudah dites di:** duchamp dengan HyperOS 3 dan PixelOS (AOSP).

**Cara kerja:** Netflix menolak system ID `30770`, jadi module ini melaporkannya sebagai `30771` secara systemless.

**Cara pakai:** flash lewat root manager, reboot, clear data Netflix. Cek di Netflix → Settings → Playback Specification.

Module ini hanya mengubah system ID yang dilaporkan, bukan membuat HP yang tidak mendukung L1 menjadi L1.
