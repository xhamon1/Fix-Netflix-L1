[ "$ARCH" = arm64 ] || abort "! arm64 only"
set_perm "$MODPATH/patchelf" 0 0 0755
sh "$MODPATH/patch.sh" "$MODPATH" || abort "! /vendor/lib64/liboemcrypto.so missing or patching failed"
ui_print "- Patched. Reboot, then clear Netflix data and check Widevine again."
