#!/bin/sh
# ankaros imajı için UTM'de GPU hızlandırmalı (virtio-gpu-gl, virgl) bir sanal makine oluşturur.
# Ayrıntılar ve elle kurulum: docs/utm.md
#
#   ./scripts/utm-olustur.sh            "ankaros" adıyla
#   ./scripts/utm-olustur.sh deneme     başka bir adla
#
# UTM'nin betik arayüzü (AppleScript) kernel ile açılışı ayarlayamıyor. Bu yüzden sanal makine önce
# AppleScript ile oluşturulur, sonra UTM kapatılıp yapılandırma dosyasına kernel, açılış argümanları
# ve virtio girdi aygıtları eklenir. Çalışan bir sanal makine varken betik UTM'yi kapatmaz, durur.
set -e
NAME="${1:-ankaros}"
IMAGES="${IMAGES:-$HOME/OrbStack/ankaros-build/home/$USER/ankaros-output/images}"
BUNDLE="$HOME/Library/Containers/com.utmapp.UTM/Data/Documents/$NAME.utm"

fail() {
  echo "$1" >&2
  exit 1
}

utm() {
  osascript -e "with timeout of 900 seconds" -e "tell application \"UTM\" to $1" -e "end timeout"
}

[ -d /Applications/UTM.app ] || fail "UTM kurulu değil: https://mac.getutm.app"
if [ ! -f "$IMAGES/Image" ] || [ ! -f "$IMAGES/rootfs.ext4" ]; then
  fail "İmaj bulunamadı: $IMAGES"
fi
[ ! -e "$BUNDLE" ] || fail "\"$NAME\" adında bir sanal makine zaten var. UTM'den sil ya da başka bir ad ver."
if pgrep -xq UTM && utm "get status of every virtual machine" | grep -qv stopped; then
  fail "UTM'de çalışan bir sanal makine var. Betik UTM'yi yeniden başlatacak, önce onları kapat."
fi

# UTM kendi klasörü dışındaki dosyalara sınırlı erişebilir (sandbox). Kök dosya sistemi geçici bir
# kopyadan alınır, UTM onu sanal makinenin klasörüne qcow2 olarak kopyalar.
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
cp -L "$IMAGES/rootfs.ext4" "$STAGE/rootfs.img"

echo "Sanal makine oluşturuluyor (ilk seferde macOS, UTM'nin denetlenmesi için izin sorabilir)..."
utm "make new virtual machine with properties {backend:qemu, configuration:{name:\"$NAME\", \
architecture:\"aarch64\", memory:2048, cpu cores:4, hypervisor:true, uefi:false, \
drives:{{interface:VirtIO, source:POSIX file \"$STAGE/rootfs.img\"}}, \
displays:{{hardware:\"virtio-gpu-gl-pci\"}}}}" >/dev/null

echo "Kernel ve açılış argümanları ekleniyor..."
utm "quit"
while pgrep -xq UTM; do sleep 1; done
cp "$IMAGES/Image" "$BUNDLE/Data/Image"
plutil -insert Drive.0 -json \
  '{"Identifier":"'"$(uuidgen)"'","ImageName":"Image","ImageType":"LinuxKernel","Interface":"None","InterfaceVersion":1,"ReadOnly":true}' \
  "$BUNDLE/config.plist"
# UTM her satırı boşluklardan böler, birden çok kelimelik değerler tırnak içinde yazılır.
# Çekirdekte USB girdi sürücüsü yok, klavye ve fare virtio aygıtlarıdır.
plutil -replace QEMU.AdditionalArguments -json \
  '["-append \"rootwait root=/dev/vda console=ttyAMA0\"","-device virtio-keyboard-pci","-device virtio-tablet-pci"]' \
  "$BUNDLE/config.plist"

open -g -a UTM
echo "Hazır: UTM'de \"$NAME\" sanal makinesini başlat."
