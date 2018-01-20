/*
 * linker script for pocketstation
 * ポケットステーション用 リンカスクリプト
 *
 * .section scheader
 *   先頭ブロック用のヘッダがあるセクションとみなす
 *
 * __block_num
 *   使用ブロック数が格納される ソースコードから使用可能
 *   使用ブロック数が15を超えるとリンカでエラーとなる
 */

OUTPUT_ARCH(arm)


MEMORY
{
  /* 0x200 から始めるとexitのメニューがでてくる */
  ram : o = 0x00000204, l = 0x5fc
  rom : o = 0x02000000, l = 0x2000
}


SECTIONS
{
  /*
   * 使用するブロック数を計算する
   * 15を超えるとエラー
   */
  __block_size = 8192;
  __rom_size = (__rom_end - __rom_start);
  __block_num = (__rom_size - 1) / __block_size + 1;
  ASSERT((__block_num <= 15), "ERROR: over 15 blocks")

  .text :
  {
    __rom_start = .;

    *(scheader)

    . = ALIGN(0x200);  /* ? 何のサイズか忘れた */
    *(.text)
    *(.strings)
    *(.rodata*)
    *(.rdata)
    *(.glue_7)
    *(.glue_7t)
    _etext = .;
  } > rom

  .data :
  {
    *(.data)
    __rom_end = .;
    _edata = .;
  } > rom

  .bss :
  {
    _bss_start = .;
    *(.bss)
    *(COMMON)
    _end = .;
  } > ram
}

