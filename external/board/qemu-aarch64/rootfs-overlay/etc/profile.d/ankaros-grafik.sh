# Türkçe ve UTF-8. Qt UTF-8 olmayan bir dil ayarında uyarı verir ve tarihleri İngilizce yazar.
export LANG="${LANG:-tr_TR.UTF-8}"

# Qt uygulamaları X ya da Wayland olmadan, doğrudan ekran kartına (DRM/KMS) çizer. Masaüstündeki
# Terminal'de ise Wayland'li kalır (uygulamalar compositor'da pencere olarak açılır).
if [ -z "$WAYLAND_DISPLAY" ]; then
  export QT_QPA_PLATFORM=eglfs
  export QT_QPA_EGLFS_INTEGRATION=eglfs_kms
fi
# Varsayılan klavye düzeni (libinput + xkbcommon). Kullanıcı Ayarlar › Klavye ve dil'den seçtiyse oturum
# yöneticisi XKB_DEFAULT_LAYOUT'u kendisi verir (docs/oturum.md).
export XKB_DEFAULT_LAYOUT="${XKB_DEFAULT_LAYOUT:-tr}"

# Wayland ve Qt'nin çalışma zamanı dosyaları için kullanıcıya özel klasör
export XDG_RUNTIME_DIR="/run/user/$(id -u)"
[ -d "$XDG_RUNTIME_DIR" ] || { mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"; }
