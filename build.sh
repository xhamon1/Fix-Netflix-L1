#!/bin/sh
# Rebuild the arm64 shim and the flashable zip. Needs: clang (+lld), patchelf (host), zip.
set -e
cd "$(dirname "$0")"
clang --target=aarch64-linux-android21 -fuse-ld=lld -shared -fPIC -O2 -fno-stack-protector \
  -nostdlib -ffreestanding -fno-builtin -Wl,-z,max-page-size=16384 \
  -Wl,-soname,liboemcrypto_shim.so liboemcrypto_shim.c -o liboemcrypto_shim.so
patchelf --add-needed libc.so liboemcrypto_shim.so
rm -f fix_netflix_l1.zip
zip -q fix_netflix_l1.zip module.prop customize.sh patch.sh post-fs-data.sh patchelf liboemcrypto_shim.so
echo "built fix_netflix_l1.zip"
