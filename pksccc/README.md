# PocketStation C Cross Compiler
ポケットステーション用gcc構築

* `Makefile`
  * binutils gccインストール用
* `etc/ldscripts/*`
  * ポケットステーション用リンカスクリプト


## 準備(必要なパッケージ)
* このパッケージにはbinutils、gccのパッケージが必要だが、含まれていない
* 以下のパッケージをダウンロードして package/ ディレクトリに置くこと
  * binutils-2.27.tar.gz
  * gcc-3.4.6.tar.gz
* 以下のコマンドを実行するとダウンロードできる
  1. `cd package`
  2. `wget -i pkglist`


## ビルド、インストール
* 以下のコマンドでビルド、インストールを行う(make のみは何もしない)
  * `make install`
* デフォルトでは`$HOME/pks_toolchain/`にインストールする
  * `make`時に`INSTDIR`で指定できる
* ソースコードを既に展開しているなら以下のコマンドでpackage/ がなくてもエラーにならない
  * `make install PAC_BIN= PAC_GCC=`

### 説明
binutils gcc が両方ともビルドだけでインストールしていない状態にはできない。
(gccをビルドするためにはターゲット用のbinutilsを使用する必要があるため)

そのため、`make`のみは何もしていない

`make install`時には内部では先にbinutilsの`make`, `make install`を実行している。
その後、インストールしたbinutilsを使用してgccの`make`, `make install`を実行する

### DESTDIR
DESTDIRを指定する場合でも先にbinutilsを適当な場所にインストールしてから
DESTDIRを指定したビルドを実行する必要がある

例:
1. `make build_bin install_bin`
2. `make build_bin build_gcc install_bin install_gcc INSTDIR=/usr/local DESTDIR=$HOME/ppp PKS_BINDIR=$HOME/pks_toolchain/bin`

### cygwin64ビット版でビルドする場合
binutilsの一部とgccのconfig.guessがcygwin64を認識できないので
gccの`--build` `--host`を指定する必要がある

例:
1. `make install CONF_FLAG_BIN=--build=i686-pc-cygwin CONF_FLAG_GCC=--host=i686-pc-cygwin`


## アンインストール
* `INSTDIR`以下にしかインストールしないので`INSTDIR`で指定したディレクトリを削除すればOK
  * デフォルトでは`$HOME/pks_toolchain/`

