################################################################################
#
# xwayland-satellite
#
################################################################################

XWAYLAND_SATELLITE_VERSION = 0.8.3
XWAYLAND_SATELLITE_SITE = $(call github,Supreeeme,xwayland-satellite,v$(XWAYLAND_SATELLITE_VERSION))
XWAYLAND_SATELLITE_LICENSE = MPL-2.0, OFL-1.1 (OpenSans-Regular.ttf)
XWAYLAND_SATELLITE_LICENSE_FILES = LICENSE

# xcb-util-cursor-sys bağlamalarını derleme sırasında bindgen ile üretir, bindgen libclang ister.
# Derleme makinesinin clang'ı yerine Buildroot'unki kullanılır (tekrar üretilebilir derleme).
XWAYLAND_SATELLITE_DEPENDENCIES = host-clang libxcb xcb-util-cursor
XWAYLAND_SATELLITE_CARGO_ENV = \
	LIBCLANG_PATH=$(HOST_DIR)/lib \
	BINDGEN_EXTRA_CLANG_ARGS="--target=$(GNU_TARGET_NAME) --sysroot=$(STAGING_DIR)"

$(eval $(cargo-package))
