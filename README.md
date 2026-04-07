# sgdk_fmop

MDSDRVの音色（FM音源パラメータ）をメガドライブ実機上でリアルタイムに編集するためのツールプログラムです。

**本リポジトリのソフトウェアは現在も実装中であり、完成品・製品版ではありません。** 仕様・操作・表示は予告なく変わる可能性があります。

## 概要

画面に表示される数値はすべて **MDSDRV-mml の音色定義構文** に対応しています。
気に入った音色ができたら、画面の値をそのままPCのMMLファイルに転記するだけで、音色データとして利用できます。

## MDSDRV-mml 音色定義との対応

MDSDRV-mml の音色定義は以下の形式です。

```
;==== Instrument section =====

@0 fm 3 0 ;Instrument

;AR5 DR5 SR5 RR4 SL4 TL7  K2 ML4 DT2 SSG
  31   0  19   5   0  23   0   0   0   0
  31   6   0   4   3  19   0   0   0   0
  31  15   0   5   4  38   0   4   0   0
  31  27   0  11   1   0   0   1   0   0
```

### 各フィールドの説明

| フィールド | 意味 |
|-----------|------|
| `@0 fm 3 0` | 音色番号・種別・アルゴリズム(ALG)・フィードバック(FB)。**表示のみ・編集対象外** |
| `AR` | Attack Rate（アタックレート） |
| `DR` | Decay Rate（ディケイレート） |
| `SR` | Sustain Rate（サステインレート） |
| `RR` | Release Rate（リリースレート） |
| `SL` | Sustain Level（サステインレベル） |
| `TL` | Total Level（トータルレベル） |
| `K` | Key Scale（キースケール） |
| `ML` | Multiple（周波数倍率） |
| `DT` | Detune（デチューン） |
| `SSG` | SSG-EG |

行はオペレータ OP1〜OP4 に対応します（上から順）。

`@0 fm 3 0` の部分はMMLファイル上のヘッダであり、本プログラムでは参照用に表示しますが、編集対象のパラメータとしては扱いません。

## 操作方法

| 操作 | 動作 |
|------|------|
| D-pad（上下左右） | カーソル移動（編集するパラメータを選択） |
| C ＋ 左右 | 選択中のパラメータを **±1** 変更（最小値・最大値でクランプ） |
| C ＋ 上下 | 選択中のパラメータを **±10** 変更（最小値・最大値でクランプ） |
| A ボタン（押下中） | 現在の音色で発音 |
| A ボタン（離す） | 消音 |

## ビルド方法

### macOS

```sh
# GDK 環境変数が未設定の場合は /Users/junt74/SGDK を自動使用
make release

# ビルド後にエミュレータ（OpenEMU）で起動
make run
```

### Windows

```bat
set GDK=C:\LIBRARY\SGDK
make release

# エミュレータ指定例
make run EMU="C:/path/to/Fusion.exe"
```

### 前提条件

- [SGDK](https://github.com/Stephane-D/SGDK)（環境変数 `GDK` で指定）
- m68k-elf-gcc（macOS: `brew install m68k-elf-gcc`）
- Java（ResComp用、macOS: `brew install openjdk`）

## 関連

- [MDSDRV](https://github.com/superctr/MDSDRV) - 音色定義の構文仕様はMDSDRVのドキュメントを参照
