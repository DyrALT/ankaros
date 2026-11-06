#!/bin/sh
# ankaros imajını QEMU ile açar: Mac'te HVF, Linux'ta KVM donanım hızlandırmasıyla (yoksa taklit).
# Mimari imajdan anlaşılır: bzImage varsa x86_64, Image varsa aarch64.
#
#   ./scripts/run-qemu.sh             yalnızca seri konsol (bu terminalde)
#   ./scripts/run-qemu.sh --gui       ek olarak bir ekran penceresi, klavye ve fare
#   ./scripts/run-qemu.sh -snapshot   imajı değiştirmeden dene (diğer QEMU seçenekleri de geçer)
#
# Çıkmak için: Ctrl-a ardından x
#
# Ses (docs/ses.md): $ANKAROS_AUDIO ses sürücüsü (Mac'te coreaudio, başka yerde none).
#
# Veri diski (karar 0017): $ANKAROS_DATA (varsayılan ~/.local/share/ankaros/veri.qcow2), yoksa 16 GB'lık
# boş bir disk oluşturulur. İlk açılışta ankaros onu biçimlendirip /var/lib/ankaros'a bağlar.
#
# --gui modunda sanal GPU 2D'dir (virtio-gpu-pci), Mesa yazılımsal çizer (softpipe): çalışır ama
# yavaştır. GPU hızlandırmalı test için Mac'te UTM kullanılır (bkz. docs/utm.md).
#
# file.locking=off: imaj OrbStack'in paylaşılan dosya sisteminden okunuyor ve bu dosya sistemi
# QEMU'nun bayt kilitlerini desteklemiyor ("Failed to lock byte 100").
set -e
if [ "$(uname)" = Darwin ]; then
  IMAGES="${IMAGES:-$HOME/OrbStack/ankaros-build/home/$USER/ankaros-output/images}"
  GUI=cocoa,zoom-to-fit=on
else
  IMAGES="${IMAGES:-$HOME/ankaros-output/images}"
  GUI=gtk
fi

HOST_ARCH=$(uname -m)
[ "$HOST_ARCH" = arm64 ] && HOST_ARCH=aarch64
if [ -f "$IMAGES/bzImage" ]; then
  ARCH=x86_64
  KERNEL=bzImage
  CONSOLE=ttyS0
  MACHINE="-M q35 -vga none"
  ROOT_DEVICE=virtio-blk-pci
  NET_DEVICE=virtio-net-pci
else
  ARCH=aarch64
  KERNEL=Image
  CONSOLE=ttyAMA0
  MACHINE="-M virt"
  ROOT_DEVICE=virtio-blk-device
  NET_DEVICE=virtio-net-device
fi

# Donanım hızlandırması yalnızca aynı mimaride ve varsa
ACCEL="-accel tcg -cpu max"
if [ "$ARCH" = "$HOST_ARCH" ]; then
  if [ "$(uname)" = Darwin ]; then
    ACCEL="-accel hvf -cpu host"
  elif [ -w /dev/kvm ]; then
    ACCEL="-accel kvm -cpu host"
  fi
fi

DATA="${ANKAROS_DATA:-$HOME/.local/share/ankaros/veri.qcow2}"
if [ ! -f "$DATA" ]; then
  mkdir -p "$(dirname "$DATA")"
  qemu-img create -q -f qcow2 "$DATA" 16G
fi

# Ses (docs/ses.md): virtio-sound-pci (QEMU 8.2+). Mac'te coreaudio, başka yerde ses çıkmaz (none).
# $ANKAROS_AUDIO ile değişir (ör. pa, pipewire, alsa).
if [ -z "$ANKAROS_AUDIO" ]; then
  [ "$(uname)" = Darwin ] && ANKAROS_AUDIO=coreaudio || ANKAROS_AUDIO=none
fi

DISPLAY_ARGS="-nographic"
if [ "$1" = "--gui" ]; then
  shift
  DISPLAY_ARGS="-display $GUI -serial mon:stdio
    -device virtio-gpu-pci,xres=1440,yres=900
    -device virtio-keyboard-pci -device virtio-tablet-pci"
fi

# shellcheck disable=SC2086 # DISPLAY_ARGS bilerek bölünüyor
exec "qemu-system-$ARCH" \
  $MACHINE $ACCEL -smp 4 -m 2G \
  $DISPLAY_ARGS \
  -kernel "$IMAGES/$KERNEL" \
  -append "rootwait root=/dev/vda console=$CONSOLE" \
  -drive file="$IMAGES/rootfs.ext4",if=none,format=raw,id=hd0,file.locking=off \
  -device "$ROOT_DEVICE",drive=hd0 \
  -drive file="$DATA",if=none,format=qcow2,id=veri \
  -device virtio-blk-pci,drive=veri \
  -audiodev "$ANKAROS_AUDIO",id=ses -device virtio-sound-pci,audiodev=ses \
  -netdev user,id=eth0 -device "$NET_DEVICE",netdev=eth0 \
  "$@"
