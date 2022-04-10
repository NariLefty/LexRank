# Continuous LexRank

## Overview
[LexRank: Graph-based Lexical Centrality as Salience in Text Summarization](https://arxiv.org/abs/1109.2128) のうち, Continuous LexRankを実装しました.

## Requirement
- macOS
- VisualStudio
- C++ 12.0.5

## Usage
`$./lexrank ファイル名 べき乗法の回数 テレポーテーションの確率`

## Description
- スパースな行列を疎行列として扱う

    CSR形式で疎行列を扱うようにし，少ないメモリで記述できるようにした．

- 式変形を行い, 大量のデータでも計算できるようにした
    計算結果を陽に持たないように式変形を行うことで大量のデータでも計算できるようにした．
    また，隣接行列を計算する際には全ての要素が1の縦ベクトルを使用することで，大量のデータでも計算できるようにした．

## Reference
[LexRank: Graph-based Lexical Centrality as Salience in Text Summarization Gunes Erkan, Dragomir R. Radev](https://arxiv.org/abs/1109.2128)

[CRS形式 - JICFuS Wiki](http://www.jicfus.jp/wiki/index.php?CRS%E5%BD%A2%E5%BC%8F)

## Author
Twitter    : [@kazun6ri](https://twitter.com/kazun6ri)
slideshape :  [KazunariHemmi](https://www.slideshare.net/KazunariHemmi)
