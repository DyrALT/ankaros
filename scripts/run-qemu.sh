#!/bin/sh
# ankaros imajını Mac'te QEMU ile, Apple Silicon donanım hızlandırmasıyla (HVF) açar.
#
#   ./scripts/run-qemu.sh             yalnızca seri konsol (bu terminalde)
#   ./scripts/run-qemu.sh --gui       ek olarak bir ekran penceresi, klavye ve fare
#   ./scripts/run-qemu.sh -snapshot   imajı değiştirmeden dene (diğer QEMU seçenekleri de geçer)
#
# Çıkmak için: Ctrl-a ardından x
#
# --gui modunda sanal GPU 2D'dir (virtio-gpu-pci), Mesa yazılımsal çizer (softpipe): çalışır ama
# yavaştır. GPU hızlandırmalı test için UTM kullanılır (bkz. docs/faz-1-plani.md).
#
# file.locking=off: imaj OrbStack'in paylaşılan dosya sisteminden okunuyor ve bu dosya sistemi
# QEMU'nun bayt kilitlerini desteklemiyor ("Failed to lock byte 100").
set -e
IMAGES="${IMAGES:-$HOME/OrbStack/ankaros-build/home/$USER/ankaros-output/images}"

DISPLAY_ARGS="-nographic"
if [ "$1" = "--gui" ]; then
  shift
  DISPLAY_ARGS="-display cocoa,zoom-to-fit=on -serial mon:stdio
    -device virtio-gpu-pci,xres=1440,yres=900
    -device virtio-keyboard-pci -device virtio-tablet-pci"
fi

# shellcheck disable=SC2086 # DISPLAY_ARGS bilerek bölünüyor
exec qemu-system-aarch64 \
  -M virt -accel hvf -cpu host -smp 4 -m 2G \
  $DISPLAY_ARGS \
  -kernel "$IMAGES/Image" \
  -append "rootwait root=/dev/vda console=ttyAMA0" \
  -drive file="$IMAGES/rootfs.ext4",if=none,format=raw,id=hd0,file.locking=off \
  -device virtio-blk-device,drive=hd0 \
  -netdev user,id=eth0 -device virtio-net-device,netdev=eth0 \
  "$@"
