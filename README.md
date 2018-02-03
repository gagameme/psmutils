# psmutils
PS1メモリーカード用**コマンドライン**ツール


## mcactl
メモリーカードアダプターを使用してPS1メモリーカードの読み書きを行う

### 依存
* pkg-config
* libusb-1.0

### 使い方
* メモリーカードの内容をreadmc.psに出力する
  * `./mcactl -r > readmc.ps`
* メモリーカードへwritemc.psの内容を書き込む
  * `cat writemc.ps | ./mcactl -w`


## pksccc
ポケットステーション用クロスコンパイラとしてgccをビルドするためのMakefile等<br>
(まだ役に立たないかも)<br>
(ビルドが楽なのでgcc3を使用)<br>
(リンカスクリプトを作成したけど微妙)<br>

### 依存
* binutils-2.27.tar.gz
* gcc-3.4.6.tar.gz


## psmedit
PS1メモリーカードのデータをブロック単位で編集する

### 依存
無し

### 使い方
* file.psの概要を表示
  * `./psmedit -I file.ps`
* file.psのブロック2へsave.mcbの内容を書き込む
  * `./psmedit -b2 -U save.mcb file.ps`
* file.psのブロック3の内容をblock3.mcbへ出力
  * `./psmedit -b3 -D -o block3.mcb file.ps`


---


## TODO
ドキュメントの作成
