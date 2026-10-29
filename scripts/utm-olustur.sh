#!/bin/sh
# ankaros imajı için UTM'de GPU hızlandırmalı (virtio-gpu-gl, virgl) bir sanal makine oluşturur.
# Ayrıntılar ve elle kurulum: docs/utm.md
#
#   ./scripts/utm-olustur.sh                    "ankaros" adıyla
#   ./scripts/utm-olustur.sh deneme             başka bir adla
#   ./scripts/utm-olustur.sh --guncelle deneme  var olan sanal makinenin diskini ve kernel'ini yeni
#                                               imajla değiştir (sanal makinedeki değişiklikler silinir)
#
# UTM'nin betik arayüzü (AppleScript) kernel ile açılışı ayarlayamıyor. Bu yüzden sanal makine önce
# AppleScript ile oluşturulur, sonra UTM kapatılıp yapılandırma dosyasına kernel, açılış argümanları
# ve virtio girdi aygıtları eklenir. Çalışan bir sanal makine varken betik UTM'yi kapatmaz, durur.
set -e
UPDATE=0
if [ "$1" = "--guncelle" ]; then
  UPDATE=1
  shift
fi
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

# Çekirdeğin açılış argümanları ve girdi aygıtları. UTM her satırı boşluklardan böler, birden çok
# kelimelik değerler tırnak içinde yazılır. Çekirdekte USB girdi sürücüsü yok, klavye ve fare virtio
# aygıtlarıdır. Son ikisi ekranı sabit 2880x1800 yapar (set_display).
ARGUMENTS='["-append \"rootwait root=/dev/vda console=ttyAMA0\"","-device virtio-keyboard-pci","-device virtio-tablet-pci","-global virtio-gpu-gl-pci.xres=2880","-global virtio-gpu-gl-pci.yres=1800"]'

# Ekran: Retina modu açık, çözünürlük sabit 2880x1800. Retina modunda görüntü Mac'in gerçek
# çözünürlüğünde gösterilir (1440x900 noktalık pencere), ankaros 2560 pikselden geniş ekranda
# arayüzü 2 kat ölçekle çizer. Dinamik çözünürlük kapalı: açık olunca sanal makine açıldığı andaki
# pencere boyutunu alıyor (küçük pencerede ölçek 1, her şey minicik) ve compositor sonradan değişen
# çözünürlüğe uymuyor. Retina modu kapalıyken yarı çözünürlükteki görüntü büyütülür, bulanık görünür.
set_display() {
  plutil -replace Display.0.NativeResolution -bool YES "$BUNDLE/config.plist"
  plutil -replace Display.0.DynamicResolution -bool NO "$BUNDLE/config.plist"
}

# Veri diski (karar 0017): sanal makinenin klasöründe 16 GB'lık, yer kapladıkça büyüyen bir disk.
# Sistem diskinden sonra eklenir, ankaros onu /dev/vdb olarak görür ve ilk açılışta biçimlendirir.
# Güncellemede dokunulmaz, kurulan uygulamalar korunur. Sanal makinede yoksa eklenir.
add_data_disk() {
  plutil -extract Drive json -o - "$BUNDLE/config.plist" | grep -q '"veri.qcow2"' && return
  qemu-img create -q -f qcow2 "$BUNDLE/Data/veri.qcow2" 16G
  count=$(plutil -extract Drive json -o - "$BUNDLE/config.plist" | python3 -c 'import json,sys; print(len(json.load(sys.stdin)))')
  plutil -insert "Drive.$count" -json \
    '{"Identifier":"'"$(uuidgen)"'","ImageName":"veri.qcow2","ImageType":"Disk","Interface":"VirtIO","InterfaceVersion":1,"ReadOnly":false}' \
    "$BUNDLE/config.plist"
  echo "Veri diski eklendi (veri.qcow2, 16 GB)."
}

[ -d /Applications/UTM.app ] || fail "UTM kurulu değil: https://mac.getutm.app"
if [ ! -f "$IMAGES/Image" ] || [ ! -f "$IMAGES/rootfs.ext4" ]; then
  fail "İmaj bulunamadı: $IMAGES"
fi
if [ "$UPDATE" = 1 ]; then
  [ -d "$BUNDLE" ] || fail "\"$NAME\" adında bir sanal makine yok."
  command -v qemu-img >/dev/null || fail "qemu-img gerekli: brew install qemu"
  if pgrep -xq UTM && utm "get status of virtual machine \"$NAME\"" | grep -qv stopped; then
    fail "\"$NAME\" çalışıyor. Önce UTM'den kapat."
  fi
  DISK=$(plutil -extract Drive json -o - "$BUNDLE/config.plist" \
    | python3 -c 'import json,sys; print(next(d["ImageName"] for d in json.load(sys.stdin) if d.get("ImageType") == "Disk"))')
  echo "Disk ($DISK) ve kernel yeni imajla değiştiriliyor, sanal makinedeki değişiklikler silinir..."
  # OrbStack klasörü dosya kilidini desteklemiyor, kilitlemeden okunur (run-qemu.sh'deki gibi)
  qemu-img convert -O qcow2 --image-opts \
    "driver=raw,file.driver=file,file.filename=$IMAGES/rootfs.ext4,file.locking=off" \
    "$BUNDLE/Data/$DISK"
  cp "$IMAGES/Image" "$BUNDLE/Data/Image"
  # Eski sanal makinelerin ekran ayarları ve argümanları da güncellenir. UTM açıkken yapılandırmayı
  # kendi kopyasıyla ezer, bu yüzden yalnızca UTM kapalıyken değiştirilir.
  if pgrep -xq UTM; then
    echo "Not: UTM açık olduğu için ekran ayarları ve veri diski güncellenmedi. UTM'yi kapatıp betiği"
    echo "yeniden çalıştır."
  else
    plutil -replace QEMU.AdditionalArguments -json "$ARGUMENTS" "$BUNDLE/config.plist"
    set_display
    add_data_disk
  fi
  echo "Hazır: UTM'de \"$NAME\" sanal makinesini başlat."
  exit 0
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
plutil -replace QEMU.AdditionalArguments -json "$ARGUMENTS" "$BUNDLE/config.plist"
set_display
command -v qemu-img >/dev/null || fail "qemu-img gerekli: brew install qemu"
add_data_disk

open -g -a UTM
echo "Hazır: UTM'de \"$NAME\" sanal makinesini başlat."
