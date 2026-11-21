#!/bin/sh
# Rebuild the arm64 shim and the flashable zip. Needs: clang (+lld), patchelf (host), zip.
set -e
cd "$(dirname "$0")"
clang --target=aarch64-linux-android21 -fuse-ld=lld -shared -fPIC -O2 -fno-stack-protector \
  -nostdlib -ffreestanding -fno-builtin -Wl,-z,max-page-size=16384 \
  -Wl,-soname,liboemcrypto_shim.so src/liboemcrypto_shim.c -o module/liboemcrypto_shim.so
patchelf --add-needed libc.so module/liboemcrypto_shim.so
rm -f fix_netflix_l1.zip
(cd module && zip -q -r ../fix_netflix_l1.zip .)
echo "built fix_netflix_l1.zip"
