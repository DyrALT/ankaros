#!/bin/sh
# Derlemenin ilerlemesini gösterir: kaç paketin bittiği ve şu an hangisinin derlendiği.
# Yüzde paket sayısına göredir, süreye göre değil (gcc, glibc ve linux diğerlerinden çok daha uzun sürer).
# Kullanım: ./scripts/progress.sh        (tek seferlik)
#           watch -n 5 ./scripts/progress.sh   (ankaros-build makinesinde, canlı)
if [ "$(uname)" = Darwin ]; then
  exec orb -m ankaros-build "$(cd "$(dirname "$0")" && pwd)/progress.sh" "$@"
fi
REPO="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${ANKAROS_OUT:-$HOME/ankaros-output}"

TOTAL=$(make -s -C "$REPO/buildroot" O="$OUT" show-info 2>/dev/null | python3 -c '
import json, sys
print(sum(1 for p in json.load(sys.stdin).values() if p.get("virtual") is not True and p.get("type") != "rootfs"))')
DONE=$(find "$OUT/build" -mindepth 2 -maxdepth 2 -name .stamp_installed 2>/dev/null | wc -l)
# ls -t: en son değişen klasörü bulmak için. Klasör adları paket-sürüm biçiminde, find gerekmez.
# shellcheck disable=SC2012
# buildroot-config ve buildroot-fs paket değil, Buildroot'un kendi çalışma klasörleri
CURRENT=$(ls -td "$OUT"/build/*/ 2>/dev/null | while read -r d; do
  case "$(basename "$d")" in buildroot-*) continue ;; esac
  [ -f "$d.stamp_installed" ] || { basename "$d"; break; }; done)

echo "Paketler: $DONE / $TOTAL  (%$((DONE * 100 / TOTAL)))"
[ -n "$CURRENT" ] && echo "Şu an:    $CURRENT"
if [ -f "$OUT/images/rootfs.ext4" ]; then echo "İmaj hazır: $OUT/images"; fi
