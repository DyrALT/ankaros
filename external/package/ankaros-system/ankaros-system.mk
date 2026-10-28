################################################################################
#
# ankaros-system
#
################################################################################

ANKAROS_SYSTEM_VERSION = 0.1.0
# Kaynak, reponun system/ klasörü. Buildroot onu derleme klasörüne kopyalar.
ANKAROS_SYSTEM_SITE = $(BR2_EXTERNAL_ANKAROS_PATH)/../system
ANKAROS_SYSTEM_SITE_METHOD = local
ANKAROS_SYSTEM_LICENSE = MIT
ANKAROS_SYSTEM_SUPPORTS_IN_SOURCE_BUILD = NO
ANKAROS_SYSTEM_CMAKE_BACKEND = ninja
ANKAROS_SYSTEM_DEPENDENCIES = libxcrypt
# Birim testleri CI'da ve geliştirme makinesinde çalışır, imaja girmez.
ANKAROS_SYSTEM_CONF_OPTS = -DANKAROS_SYSTEM_TESTS=OFF

define ANKAROS_SYSTEM_INSTALL_SERVICES
	$(INSTALL) -d -m 755 $(TARGET_DIR)/etc/ankaros/services
	$(INSTALL) -m 644 $(@D)/services/*.service $(TARGET_DIR)/etc/ankaros/services/
endef
ANKAROS_SYSTEM_POST_INSTALL_TARGET_HOOKS += ANKAROS_SYSTEM_INSTALL_SERVICES

# init olarak seçildiyse: çekirdeğin başlattığı /sbin/init ankaros-init olur. /var altındaki geçici
# klasörler RAM'deki /tmp ve /run'a bağlanır (BusyBox init'in iskeletiyle aynı düzen), günlükler
# /var/log yani /tmp altında tutulur. /var/cache diskte kalır (yazı tipi önbelleği).
ifeq ($(BR2_INIT_ANKAROS),y)
define ANKAROS_SYSTEM_INSTALL_INIT
	ln -snf ../usr/sbin/ankaros-init $(TARGET_DIR)/sbin/init
	mkdir -p $(TARGET_DIR)/var/lib
	for d in log spool tmp; do ln -snf ../tmp $(TARGET_DIR)/var/$$d; done
	ln -snf ../run $(TARGET_DIR)/var/run
	ln -snf ../run/lock $(TARGET_DIR)/var/lock
endef
ANKAROS_SYSTEM_POST_INSTALL_TARGET_HOOKS += ANKAROS_SYSTEM_INSTALL_INIT
endif

$(eval $(cmake-package))
