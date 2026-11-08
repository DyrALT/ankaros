#!/bin/sh
# Hedef kök dosya sistemi hazırlandıktan sonra, imaj oluşturulmadan önce çalışır.
set -e
TARGET_DIR="$1"
# Sürüm tek yerden gelir: depo kökündeki VERSION (git describe CI'daki sığ klonda etiketsiz kalabilir)
VERSION="$(tr -d '[:space:]' < "$(dirname "$0")/../../../VERSION")"

cat > "${TARGET_DIR}/usr/lib/os-release" <<OSREL
NAME="ankaros"
ID=ankaros
PRETTY_NAME="ankaros ${VERSION}"
VERSION_ID=${VERSION}
OSREL
ln -sf ../usr/lib/os-release "${TARGET_DIR}/etc/os-release"
