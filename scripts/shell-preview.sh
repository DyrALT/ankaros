#!/bin/sh
# Masaüstü kabuğunu derleyip önizleme penceresinde açar (macOS ya da masaüstü Linux).
# Pencerede Cmd+R (Linux'ta Ctrl+R) kabuğu yeniden derleyip yeniden açar. Kapatmak için Cmd+Q.
set -e
REPO="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$REPO/build/shell"

if [ ! -f "$BUILD/build.ninja" ]; then
  QT_PREFIX="$(brew --prefix qt 2>/dev/null || true)"
  cmake -S "$REPO/shell" -B "$BUILD" -G Ninja ${QT_PREFIX:+-DCMAKE_PREFIX_PATH="$QT_PREFIX"}
fi

if [ "$(uname)" = Darwin ]; then
  APP="$BUILD/preview/ankaros-shell-preview.app/Contents/MacOS/ankaros-shell-preview"
else
  APP="$BUILD/preview/ankaros-shell-preview"
fi

while :; do
  until cmake --build "$BUILD"; do
    printf '\nDerleme başarısız. Hatayı düzeltip Enter tuşuna basın (çıkmak için Ctrl+C).\n'
    read -r _
  done
  # Uygulamanın çıkış kodu ne yapılacağını söyler: 42 yeniden derle ve aç, diğerleri çık.
  # Ek argümanlar (ör. --window) her açılışta uygulamaya geçer.
  set +e
  "$APP" "$@"
  CODE=$?
  set -e
  [ "$CODE" -eq 42 ] || exit "$CODE"
done
