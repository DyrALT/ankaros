# Qt uygulamaları X ya da Wayland olmadan, doğrudan ekran kartına (DRM/KMS) çizer.
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_INTEGRATION=eglfs_kms
# Klavye düzeni (libinput + xkbcommon). Farklı bir düzen için: XKB_DEFAULT_LAYOUT=us
export XKB_DEFAULT_LAYOUT="${XKB_DEFAULT_LAYOUT:-tr}"

# Wayland ve Qt'nin çalışma zamanı dosyaları için kullanıcıya özel klasör
export XDG_RUNTIME_DIR="/run/user/$(id -u)"
[ -d "$XDG_RUNTIME_DIR" ] || { mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"; }
