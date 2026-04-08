ALIGN 2

# カーソル: cursor_*digit.png は同一パレット。スプライトは PAL3 専用（src/fm_cursor.c）。

PALETTE pal_cursor_digit "cursor_1digit.png"
SPRITE spr_cursor_1d "cursor_1digit.png" 1 1 FAST 0
SPRITE spr_cursor_2d "cursor_2digit.png" 2 1 FAST 0
SPRITE spr_cursor_3d "cursor_3digit.png" 3 1 FAST 0

# FM アルゴリズム図（6x5 タイル × 8 フレーム、横連結 PNG）— パレットは PAL1 にロード
PALETTE pal_alg "arg.png"
SPRITE spr_alg "arg.png" 6 5 FAST 0

# リソース定義をここに追加する
# 例:
# TILESET bg_tiles   bg.png NONE NONE
# PALETTE bg_pal     bg.png
# SPRITE  spr_player player.png 2 4 FAST
