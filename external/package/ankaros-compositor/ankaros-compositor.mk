################################################################################
#
# ankaros-compositor
#
################################################################################

ANKAROS_COMPOSITOR_VERSION = 0.1.0
# Kaynak, reponun compositor/ klasörü. Buildroot onu derleme klasörüne kopyalar.
ANKAROS_COMPOSITOR_SITE = $(BR2_EXTERNAL_ANKAROS_PATH)/../compositor
ANKAROS_COMPOSITOR_SITE_METHOD = local
ANKAROS_COMPOSITOR_LICENSE = MIT
ANKAROS_COMPOSITOR_SUPPORTS_IN_SOURCE_BUILD = NO
ANKAROS_COMPOSITOR_CMAKE_BACKEND = ninja

ANKAROS_COMPOSITOR_DEPENDENCIES = \
	host-qt6base \
	host-qt6declarative \
	host-qt6wayland \
	ankaros-shell \
	qt6base \
	qt6declarative \
	qt6wayland

ANKAROS_COMPOSITOR_CONF_OPTS = \
	-DQT_HOST_PATH=$(HOST_DIR)

# Başlat panelindeki ve alt çubuktaki uygulamaların listesi (docs/uygulama-listesi.md)
define ANKAROS_COMPOSITOR_INSTALL_APPS
	$(INSTALL) -D -m 644 $(@D)/apps.json $(TARGET_DIR)/etc/ankaros/apps.json
endef
ANKAROS_COMPOSITOR_POST_INSTALL_TARGET_HOOKS += ANKAROS_COMPOSITOR_INSTALL_APPS

# Açılışta masaüstünü başlatan servis (ankaros-init)
define ANKAROS_COMPOSITOR_INSTALL_SERVICE
	$(INSTALL) -D -m 644 $(ANKAROS_COMPOSITOR_PKGDIR)/ankaros-compositor.service \
		$(TARGET_DIR)/etc/ankaros/services/ankaros-compositor.service
endef
ANKAROS_COMPOSITOR_POST_INSTALL_TARGET_HOOKS += ANKAROS_COMPOSITOR_INSTALL_SERVICE

$(eval $(cmake-package))
