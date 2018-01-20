# c cross compiler for pocketstation
#
# ポケットステーション用のビルド環境をインストールする
#
# * install
# $(INSTDIR)へインストールする
#
# * uninstall
# $(INSTDIR)のディレクトリを削除する
#
# * 個別のビルド、インストール
# make build_bin install_bin
# make build_gcc install_gcc
#
# * DESTDIRを使用する場合
# binutilsのみ適当な場所にインストールしてから、DESTDIRを指定したビルドを実行する
# make build_bin install_bin
# make build_bin build_gcc install_bin install_gcc INSTDIR=/usr/local DESTDIR=$HOME/ppp PKS_BINDIR=$HOME/pks_toolchain/bin

# path of Makefile
basedir := $(shell cd $(dir $(word 1,$(MAKEFILE_LIST))); pwd)

# general
# absolute path
DESTDIR =
PACDIR := $(shell pwd)/package/
BUILDDIR := $(shell pwd)/_build/
SRCDIR := $(shell pwd)/_src/
INSTDIR := ${HOME}/pks_toolchain/
PKS_ARCH := arm-none-elf

# binutils
CONF_FLAG_BIN =
PAC_BIN := $(PACDIR)/binutils-2.27.tar.gz
SRCDIR_BIN := $(SRCDIR)/binutils-2.27/
BUILDDIR_BIN := $(BUILDDIR)/binutils/

# pocket station binutils for gcc build
PKS_BINDIR := $(INSTDIR)/bin
PKS_PREFIX := $(PKS_BINDIR)/$(PKS_ARCH)-
PKS_AR := $(PKS_PREFIX)ar
PKS_RANLIB := $(PKS_PREFIX)ranlib

# gcc
CONF_FLAG_GCC =
PAC_GCC := $(PACDIR)/gcc-3.4.6.tar.gz
SRCDIR_GCC := $(SRCDIR)/gcc-3.4.6/
BUILDDIR_GCC := $(BUILDDIR)/gcc/
PKS_AS := $(INSTDIR)/bin/$(PKS_ARCH)-as
PKS_LD := $(INSTDIR)/bin/$(PKS_ARCH)-ld



##########################
# all
##########################

all:
	@echo "nothing to do"
	@echo ""
	@echo "make install"
	@echo "    install dir: $(INSTDIR)"

# gccをビルドするためには、binutilsをインストールしている必要があるため
# make install でmake相当の処理も行う
install: src build_bin install_bin build_gcc install_gcc

# ソースコードの展開
src: src_bin src_gcc

# Makefileのルールによって作成した全てのファイルを削除する
distclean: clean cleansrc

# ビルド時に使用したファイルを削除する
clean: clean_bin clean_gcc
	$(RM) -r $(BUILDDIR)

# 展開したソースコードを削除する
cleansrc: cleansrc_bin cleansrc_gcc
	$(RM) -r $(SRCDIR)

.PHONY: all install src distclean clean cleansrc


##########################
# binutils
##########################

# ソースコード展開
$(SRCDIR_BIN): $(PAC_BIN)
	mkdir -p $(SRCDIR)
	tar zxvf $(PAC_BIN) -C $(SRCDIR)
	touch $@

# パッチ修正など
src_bin: $(SRCDIR_BIN)
	# ldscriptsのインストール先の変更
	-printf ',s@^scriptdir = .*@scriptdir=$$(prefix)/lib/$(PKS_ARCH)@\nwq\n' | ed $(SRCDIR_BIN)/ld/Makefile.in

# 実行ファイル生成
build_bin: src_bin prebuild_bin make_bin

# configure
prebuild_bin:
	mkdir -p $(BUILDDIR_BIN)
	(cd $(BUILDDIR_BIN) && \
		$(SRCDIR_BIN)configure --target='$(PKS_ARCH)' \
		--prefix='$(INSTDIR)' \
		--datarootdir='$(INSTDIR)/share/$(PKS_ARCH)' \
		--disable-nls --disable-shared \
		--with-soft-float --enable-multilib --enable-interwork \
		--disable-werror \
		$(CONF_FLAG_BIN))

# make
make_bin:
	$(MAKE) -C $(BUILDDIR_BIN)

# 実行ファイルインストール
install_bin:
	$(MAKE) -C $(BUILDDIR_BIN) install
	# 不要なファイルの削除
	$(RM) -r $(DESTDIR)$(INSTDIR)/$(PKS_ARCH)

clean_bin:
	$(RM) -r $(BUILDDIR_BIN)

cleansrc_bin:
	$(RM) -r $(SRCDIR_BIN)

.PHONY: src_bin build_bin prebuild_bin make_bin install_bin clean_bin cleansrc_bin


##########################
# gcc 3.4.6
##########################

# ソースコード展開
$(SRCDIR_GCC): $(PAC_GCC)
	mkdir -p $(SRCDIR)
	tar zxvf $(PAC_GCC) -C $(SRCDIR)
	touch $@

# パッチ修正など
src_gcc: $(SRCDIR_GCC)
	# multilib用の設定
	-printf "21,23s/^# *//\nwq\n" | ed $(SRCDIR_GCC)/gcc/config/arm/t-arm-elf
	# x86_64 collect2 のバグ修正
	-printf "1537s/O_CREAT);$$/O_CREAT, 0755);/\nwq\n" | ed $(SRCDIR_GCC)/gcc/collect2.c
	# libiberty.aをインストールしない
	-printf ",s/maybe-install-libiberty //\nwq\n" | ed $(SRCDIR_GCC)/Makefile.in
	# -Bでディレクトリとして存在しないパスの場合は"/"を付加しないように修正
	-printf "5837s/^#ifndef.*/#if 0/\nwq\n" | ed $(SRCDIR_GCC)/gcc/gcc.c

# 実行ファイル生成
build_gcc: src_gcc prebuild_gcc make_gcc

# configure
prebuild_gcc:
	mkdir -p $(BUILDDIR_GCC)
	(cd $(BUILDDIR_GCC) && \
		$(SRCDIR_GCC)configure --target='$(PKS_ARCH)' \
		--prefix='$(INSTDIR)' \
		--libexecdir='$(INSTDIR)/lib' \
		--datadir='$(INSTDIR)/share/$(PKS_ARCH)' \
		--mandir='$(INSTDIR)/share/$(PKS_ARCH)/man' \
		--infodir='$(INSTDIR)/share/$(PKS_ARCH)/info' \
		--with-as='$(PKS_AS)' \
		--with-ld='$(PKS_LD)' \
		--enable-languages=c \
		--disable-nls --disable-shared \
		--enable-multilib --enable-soft-float \
		--enable-interwork \
		--without-headers --with-newlib \
		$(CONF_FLAG_GCC))

# make
make_gcc:
	$(MAKE) -C $(BUILDDIR_GCC) FLAGS_FOR_TARGET=-B$(PKS_PREFIX) AR_FOR_TARGET=$(PKS_AR) RANLIB_FOR_TARGET=$(PKS_RANLIB)

# 実行ファイルインストール
install_gcc:
	$(MAKE) -C $(BUILDDIR_GCC) install RANLIB_FOR_TARGET=$(PKS_RANLIB)
	# 不要なファイルの削除
	$(RM) -r $(DESTDIR)$(INSTDIR)/$(PKS_ARCH)

clean_gcc:
	$(RM) -r $(BUILDDIR_GCC)

cleansrc_gcc:
	$(RM) -r $(SRCDIR_GCC)

.PHONY: src_gcc build_gcc prebuild_gcc make_gcc install_gcc clean_gcc cleansrc_gcc



