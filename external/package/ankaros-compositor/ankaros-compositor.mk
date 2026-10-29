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
	host-pkgconf \
	libxkbcommon \
	qt6base \
	qt6declarative \
	qt6wayland

ANKAROS_COMPOSITOR_CONF_OPTS = \
	-DQT_HOST_PATH=$(HOST_DIR)

$(eval $(cmake-package))
