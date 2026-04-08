ALIGN 2

# カーソル: cursor_1digit.png（8px）/ cursor_2digit.png（16px）/ cursor_3digit.png（24px）
# は同一パレット。数値テキスト用 PAL0（src/fm_display.c）と CRAM 1 本で併用可能な前提。
# 現状はスプライト用に PAL2 へロード（テキスト PAL0/PAL1 と分離）。

PALETTE pal_cursor_digit "cursor_1digit.png"
SPRITE spr_cursor_1d "cursor_1digit.png" 1 1 FAST 0
SPRITE spr_cursor_2d "cursor_2digit.png" 2 1 FAST 0
SPRITE spr_cursor_3d "cursor_3digit.png" 3 1 FAST 0

# リソース定義をここに追加する
# 例:
# TILESET bg_tiles   bg.png NONE NONE
# PALETTE bg_pal     bg.png
# SPRITE  spr_player player.png 2 4 FAST
