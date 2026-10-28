################################################################################
#
# ankaros-shell
#
################################################################################

ANKAROS_SHELL_VERSION = 0.1.0
# Kaynak, reponun shell/ klasörü: Ankaros.UI, Ankaros.System ve Ankaros.Shell modülleri (karar 0013).
# Buildroot onu derleme klasörüne kopyalar.
ANKAROS_SHELL_SITE = $(BR2_EXTERNAL_ANKAROS_PATH)/../shell
ANKAROS_SHELL_SITE_METHOD = local
ANKAROS_SHELL_LICENSE = MIT, OFL-1.1 (Inter yazı tipi)
ANKAROS_SHELL_LICENSE_FILES = qml/Ankaros/UI/fonts/Inter-LICENSE.txt
ANKAROS_SHELL_SUPPORTS_IN_SOURCE_BUILD = NO
ANKAROS_SHELL_CMAKE_BACKEND = ninja

ANKAROS_SHELL_DEPENDENCIES = \
	host-qt6base \
	host-qt6declarative \
	host-qt6shadertools \
	qt6base \
	qt6declarative \
	qt6shadertools

# Modül staging'e de kurulur, çünkü compositor derlenirken qmllint ve qmlcachegen onu bulmalı.
ANKAROS_SHELL_INSTALL_STAGING = YES

ANKAROS_SHELL_CONF_OPTS = \
	-DQT_HOST_PATH=$(HOST_DIR) \
	-DANKAROS_SHELL_SHARED=ON \
	-DANKAROS_SHELL_PREVIEW=ON

$(eval $(cmake-package))
