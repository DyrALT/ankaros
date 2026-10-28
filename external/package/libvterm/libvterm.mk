################################################################################
#
# libvterm
#
################################################################################

LIBVTERM_VERSION = 0.3.3
LIBVTERM_SITE = https://launchpad.net/libvterm/trunk/v0.3/+download
LIBVTERM_LICENSE = MIT
LIBVTERM_LICENSE_FILES = LICENSE
LIBVTERM_INSTALL_STAGING = YES
# Yalnızca Terminal'e statik olarak bağlanır, imaja ayrı bir dosya kurulmaz
LIBVTERM_INSTALL_TARGET = NO

# Kendi Makefile'ı libtool istiyor. Kütüphane birkaç C dosyasından ibaret ve kaynak paketinde
# kodlama tabloları (.inc) hazır geldiği için doğrudan derlenip statik kütüphane yapılır.
define LIBVTERM_BUILD_CMDS
	cd $(@D) && for f in src/*.c; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -std=c99 -fPIC -Iinclude -c $$f -o $${f%.c}.o || exit 1; \
	done
	$(TARGET_AR) rcs $(@D)/libvterm.a $(@D)/src/*.o
	sed -e 's|@LIBDIR@|/usr/lib|;s|@INCDIR@|/usr/include|' \
		$(@D)/vterm.pc.in > $(@D)/vterm.pc
endef

define LIBVTERM_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/include/vterm.h $(STAGING_DIR)/usr/include/vterm.h
	$(INSTALL) -D -m 644 $(@D)/include/vterm_keycodes.h $(STAGING_DIR)/usr/include/vterm_keycodes.h
	$(INSTALL) -D -m 644 $(@D)/libvterm.a $(STAGING_DIR)/usr/lib/libvterm.a
	$(INSTALL) -D -m 644 $(@D)/vterm.pc $(STAGING_DIR)/usr/lib/pkgconfig/vterm.pc
endef

$(eval $(generic-package))
