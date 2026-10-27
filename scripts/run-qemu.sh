#!/bin/sh
# ankaros imajını Mac'te QEMU ile, Apple Silicon donanım hızlandırmasıyla (HVF) açar.
# Çıkmak için: Ctrl-a ardından x
# İmajı değiştirmeden denemek için: ./scripts/run-qemu.sh -snapshot
#
# file.locking=off: imaj OrbStack'in paylaşılan dosya sisteminden okunuyor ve bu dosya sistemi
# QEMU'nun bayt kilitlerini desteklemiyor ("Failed to lock byte 100").
set -e
IMAGES="${IMAGES:-$HOME/OrbStack/ankaros-build/home/$USER/ankaros-output/images}"

exec qemu-system-aarch64 \
  -M virt -accel hvf -cpu host -smp 4 -m 2G \
  -nographic \
  -kernel "$IMAGES/Image" \
  -append "rootwait root=/dev/vda console=ttyAMA0" \
  -drive file="$IMAGES/rootfs.ext4",if=none,format=raw,id=hd0,file.locking=off \
  -device virtio-blk-device,drive=hd0 \
  -netdev user,id=eth0 -device virtio-net-device,netdev=eth0 \
  "$@"
