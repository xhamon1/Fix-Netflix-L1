#!/system/bin/sh
D=${1:-${0%/*}}
SRC=${SRC:-/vendor/lib64/liboemcrypto.so}
P=${PATCHELF:-$D/patchelf}
OUT=$D/system/vendor/lib64
[ -f "$SRC" ] || exit 1
H=$(sha256sum "$SRC" | cut -d' ' -f1)
[ "$(cat "$D/src.sha256" 2>/dev/null)" = "$H" ] && [ -f "$OUT/liboemcrypto.so" ] && exit 0
rm -rf "$D/system"; mkdir -p "$OUT"
cp "$SRC" "$OUT/liboemcrypto.so" && cp "$D/liboemcrypto_shim.so" "$OUT/" \
  && "$P" --add-needed liboemcrypto_shim.so "$OUT/liboemcrypto.so" \
  && "$P" --print-needed "$OUT/liboemcrypto.so" | grep -q liboemcrypto_shim.so \
  || { rm -rf "$D/system" "$D/src.sha256"; exit 1; }
chmod 644 "$OUT"/*.so
chcon u:object_r:vendor_file:s0 "$OUT"/*.so 2>/dev/null
echo "$H" > "$D/src.sha256"
