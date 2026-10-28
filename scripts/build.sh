#!/bin/sh
# ankaros imajını derler. Argümanlar doğrudan Buildroot'un make komutuna gider
# (ör. ./scripts/build.sh menuconfig).
#
# macOS'ta derlemeyi OrbStack'teki ankaros-build makinesinde çalıştırır, Linux'ta (CI dahil)
# doğrudan çalışır. Çıktı repo dışında, büyük/küçük harfe duyarlı bir dosya sisteminde tutulur:
#   ANKAROS_OUT  (varsayılan ~/ankaros-output)  Buildroot çıktısı, imajlar images/ altında
#   BR2_DL_DIR   (varsayılan ~/ankaros-dl)      indirilen kaynak paketleri
# Mac'ten çıktıya: ~/OrbStack/ankaros-build/home/$USER/ankaros-output
set -e
REPO="$(cd "$(dirname "$0")/.." && pwd)"

if [ "$(uname)" = Darwin ]; then
  exec orb -m ankaros-build "$REPO/scripts/build.sh" "$@"
fi

DEFCONFIG=ankaros_qemu_aarch64_defconfig
OUT="${ANKAROS_OUT:-$HOME/ankaros-output}"
export BR2_DL_DIR="${BR2_DL_DIR:-$HOME/ankaros-dl}"

cd "$REPO/buildroot"
# İlk derlemede ve defconfig değiştiğinde (ör. git pull sonrası) yapılandırma yeniden uygulanır.
# menuconfig ile yapılan ve savedefconfig ile kaydedilmemiş değişiklikler bu durumda kaybolur.
if [ ! -f "$OUT/.config" ] \
  || [ -n "$(find "$REPO/external/configs/$DEFCONFIG" -newer "$OUT/.config")" ]; then
  make BR2_EXTERNAL="$REPO/external" O="$OUT" "$DEFCONFIG"
fi
make O="$OUT" "$@"
