#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=unstrung

PKG_VERSION:=1.00
PKG_RELEASE:=1

#PKG_SOURCE_URL:=http://unstrung.sandelman.ca/downloads
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)11.tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/unstrung
  TITLE:=RPL mesh-over routing daemon
  SECTION:=net
  CATEGORY:=Network
  URL:=http://unstrung.sandelman.ca
  SUBMENU:=Routing
  MENU:=1
  DEPENDS:=+libstdcpp +libc
  VARIANT:=
  MAINTAINER:=Michael Richardson <mcr@sandelman.ca>
endef

define Build/Configure
	echo >${PKG_BUILD_DIR}/Makefile.local
	echo CROSSFLAGS= >>${PKG_BUILD_DIR}/Makefile.local
	echo EMBEDDED=openwrt >>${PKG_BUILD_DIR}/Makefile.local
	echo USE_OBJDIR=true  >>${PKG_BUILD_DIR}/Makefile.local
endef

define Package/unstrung/install
	$(INSTALL_DIR) \
		$(1)/sbin \
		$(1)/etc/init.d \
		$(1)/etc/config

	$(INSTALL_BIN) ./files/sunshine.init $(1)/etc/init.d/sunshine
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/sbin/sunshine \
		$(1)/sbin/
endef

$(eval $(call BuildPackage,unstrung))
