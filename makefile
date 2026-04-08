#
# SGDK FMOP Makefile
#
#  共通: SGDK 本体は環境変数 GDK で指定。SGDK 付属の makefile.gen を利用してビルド。
#
#  【Windows】 GDK を必ず設定（例: set GDK=C:\LIBRARY\SGDK）。make run 時は
#    EMU に Fusion 等のパスを指定（例: make run EMU="C:/path/to/Fusion.exe"）。
#
#  【macOS】 GDK 未設定時は /Users/junt74/SGDK を使用。
#    要: Java（ResComp 用）→ brew install openjdk。未導入だと "Unable to locate a Java Runtime" になる。
#    m68k-elf-gcc は別途必要（brew install m68k-elf-gcc または Docker/retrodevhelper）。
#    make run のデフォルトは open（.bin に紐づいたアプリで開く）。
#

# SGDK パス: 未設定時は Windows ならエラー、macOS ならデフォルト
ifndef GDK
ifeq ($(OS),Windows_NT)
$(error Please set GDK environment variable to your SGDK path (e.g. set GDK=C:\LIBRARY\SGDK))
else
GDK := /Users/junt74/SGDK
endif
endif

# PATH: Windows は common.mk が $(BIN)/gcc を直接使う。macOS 等では GDK/bin と M68K_BIN を追加
ifneq ($(OS),Windows_NT)
ifdef M68K_BIN
export PATH := $(M68K_BIN):$(GDK)/bin:$(PATH)
else
export PATH := $(GDK)/bin:$(PATH)
endif
endif

# プロジェクト名（出力 ROM 名）
PROJECT = sgdk_fmop

# ソース／ヘッダディレクトリ
SRC_DIRS = src
INC_DIRS = inc

# リソース定義
RESOURCES = res/resources.res

# rom_header.c の固定長フィールドによる -Wunterminated-string-initialization を抑制
# （SGDK の ROM ヘッダ仕様のため）
EXTRA_FLAGS := -Wno-unterminated-string-initialization

# SGDK 共通設定を include
include $(GDK)/makefile.gen

# MDSDRV: ResComp が生成する res/mdsdat.h より先に C をコンパイルしないよう依存を明示
$(OUT)/src/main.o: res/mdsdat.h
$(OUT)/src/mdsdrv.o: res/mdsdat.h

# macOS: /usr/bin/java はスタブのため Homebrew の java を明示指定（"Unable to locate a Java Runtime" 対策）
ifneq ($(OS),Windows_NT)
JAVA_HOMEBREW := $(shell brew --prefix openjdk 2>/dev/null)
ifneq ($(JAVA_HOMEBREW),)
JAVA := $(JAVA_HOMEBREW)/bin/java
RESCOMP := $(JAVA) -jar $(BIN)/rescomp.jar
endif
endif

# ----- プロジェクト独自: ROM ヘッダと LTO 対応（該当する場合のみ） -----
# 以下は makefile.gen のルールを意図的に上書きするため、
# "overriding commands for target ..." / "ignoring old commands ..." の警告が出ます（無視して問題ありません）。
#
# (1) カスタム src/boot/rom_head.c があるときだけ: 二重定義を防ぎ、これを rom_header.c の元にする
ifneq ($(wildcard src/boot/rom_head.c),)
src/rom_header.c: src/boot/rom_head.c
	@$(MKDIR) -p src
	$(CP) src/boot/rom_head.c src/rom_header.c
SRC_C := $(filter-out src/boot/rom_head.c,$(SRC_C))
OBJS := $(filter-out $(OUT)/src/boot/rom_head.o,$(OBJS))
# src/boot/sega.s が rom_head.bin 経由で rom_header を内包するため、コピーした rom_header.c はリンクしない
SRC_C := $(filter-out $(SRC)/rom_header.c,$(SRC_C))
OBJS := $(filter-out $(OUT)/src/rom_header.o,$(OBJS))
endif

# (2) Windows 以外（外部 m68k-elf-gcc 使用時）: SGDK の libmd.a は LTO 13 でビルド済みのため、
#     Homebrew 等の GCC 15 ではリンク時に LTO バージョン不一致になる。リンク時のみ -fno-lto にする。
ifneq ($(OS),Windows_NT)
$(OUT)/rom.out: $(OUT)/sega.o $(OUT)/cmd_ $(LIBMD)
	@$(MKDIR) -p $(dir $@)
	$(CC) -m68000 -B$(BIN) -n -T $(GDK)/md.ld -nostdlib $(OUT)/sega.o @$(OUT)/cmd_ $(LIBMD) $(LIBGCC) -o $(OUT)/rom.out -Wl,--gc-sections -fno-lto
	@$(RM) $(OUT)/cmd_
endif

# ============================================================
#  最終 ROM ファイル名（SGDK 既定の rom.bin に padROM 後コピーして配布用名にする）
# ============================================================
RELEASE_ROM = $(OUT)/FMOP.bin

# makefile.gen の padROM を上書き: sizebnd 後に配布用ファイル名へ複製（警告は無視してよい）。
padROM: $(OUT)/rom.bin
	$(SIZEBND) $(OUT)/rom.bin -sizealign 131072 -checksum
	$(CP) $(OUT)/rom.bin $(RELEASE_ROM)

# ============================================================
#  追加ターゲット: ビルド成功後にエミュレータで ROM を起動
# ============================================================
#  Windows: make run の前に EMU を指定（例: make run EMU="C:/path/to/Fusion.exe"）
#  macOS: デフォルトは OpenEMU で開く。別アプリにしたい場合は make run EMU="open -a アプリ名" 等で上書き可能。
# ============================================================

ROM_PATH = out/FMOP.bin
ifeq ($(OS),Windows_NT)
# EMU は未設定のまま。make run EMU="C:/path/to/Fusion.exe" のように指定する
else
EMU     ?= open -a OpenEMU
endif

.PHONY: run
run: release
	$(EMU) "$(ROM_PATH)"

# デフォルトで build + run にしたい場合はコメントアウトを外す:
# .DEFAULT_GOAL := run
