# Türkçe ve UTF-8. Qt UTF-8 olmayan bir dil ayarında uyarı verir ve tarihleri İngilizce yazar.
export LANG="${LANG:-tr_TR.UTF-8}"

# Qt uygulamaları X ya da Wayland olmadan, doğrudan ekran kartına (DRM/KMS) çizer. Masaüstündeki
# Terminal'de ise Wayland'li kalır (uygulamalar compositor'da pencere olarak açılır).
if [ -z "$WAYLAND_DISPLAY" ]; then
  export QT_QPA_PLATFORM=eglfs
  export QT_QPA_EGLFS_INTEGRATION=eglfs_kms
  # Penceredeki uygulamaların GPU arabellekleri: wayland-egl (Mesa'nın wl_drm'si) ve
  # linux-dmabuf. Ekran kartı GPU belleği vermiyorsa (GPU'suz ortam, softpipe) Qt uyarı verip
  # bu yolları atlar, uygulamalar ekran belleğini paylaşılan bellekle gönderir.
  export QT_WAYLAND_HARDWARE_INTEGRATION="wayland-egl;linux-dmabuf-unstable-v1"
fi
# Varsayılan klavye düzeni (libinput + xkbcommon). Kullanıcı Ayarlar › Klavye ve dil'den seçtiyse oturum
# yöneticisi XKB_DEFAULT_LAYOUT'u kendisi verir (docs/oturum.md).
export XKB_DEFAULT_LAYOUT="${XKB_DEFAULT_LAYOUT:-tr}"

# Wayland ve Qt'nin çalışma zamanı dosyaları için kullanıcıya özel klasör
export XDG_RUNTIME_DIR="/run/user/$(id -u)"
[ -d "$XDG_RUNTIME_DIR" ] || { mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"; }

# QEMU'nun virtio-gpu'su kendi tercih ettiği modu (pencere boyutu, çoğu zaman 640x480) bildirir ve
# xres/yres'i yok sayar, görüntü pikselleşir. Mod bir KMS yapılandırmasıyla seçilir. Çözünürlük çekirdek
# açılış argümanından gelir (ankaros.mode=1920x1080, run-qemu.ps1 -Width/-Height verir), yoksa 1440x900.
# Dosya salt-okunur rootfs'e yazılamaz, bu yüzden çalışma zamanı klasörüne üretilir; yazılamazsa
# /etc/ankaros/kms.json (1440x900) kullanılır. Yalnızca x86_64'te: aarch64'te (UTM) çözünürlüğü UTM'nin
# xres/yres ayarı belirler, dosya 2880x1800 yerine 1440x900'ü zorlardı. Başka bir ekran adı ya da kart
# varsa Qt dosyayı yok sayıp tercih edilen moda döner.
if [ -z "$WAYLAND_DISPLAY" ] && [ "$(uname -m)" = x86_64 ]; then
  ankaros_mode=1440x900
  ankaros_val=$(sed -n 's/.*\(^\|[[:space:]]\)ankaros\.mode=\([^[:space:]]*\).*/\2/p' /proc/cmdline 2>/dev/null)
  if printf '%s' "$ankaros_val" | grep -Eq '^[0-9]{3,5}x[0-9]{3,5}$'; then ankaros_mode="$ankaros_val"; fi
  ankaros_kms=/etc/ankaros/kms.json
  if [ -d "$XDG_RUNTIME_DIR" ] && printf '{"device":"/dev/dri/card0","hwcursor":true,"outputs":[{"name":"Virtual1","mode":"%s"}]}\n' \
      "$ankaros_mode" > "$XDG_RUNTIME_DIR/kms.json" 2>/dev/null; then
    ankaros_kms="$XDG_RUNTIME_DIR/kms.json"
  fi
  [ -f "$ankaros_kms" ] && export QT_QPA_EGLFS_KMS_CONFIG="$ankaros_kms"
  unset ankaros_mode ankaros_val ankaros_kms
fi
