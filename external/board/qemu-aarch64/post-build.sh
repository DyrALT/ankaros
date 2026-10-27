#!/bin/sh
# Hedef kök dosya sistemi hazırlandıktan sonra, imaj oluşturulmadan önce çalışır.
set -e
TARGET_DIR="$1"

cat > "${TARGET_DIR}/usr/lib/os-release" <<OSREL
NAME="ankaros"
ID=ankaros
PRETTY_NAME="ankaros 0.1 (Faz 0)"
VERSION_ID=0.1
OSREL
ln -sf ../usr/lib/os-release "${TARGET_DIR}/etc/os-release"
