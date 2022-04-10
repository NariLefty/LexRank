# Continuous LexRank

## Overview
LexRank: Graph-based Lexical Centrality as Salience in Text Summarization[URL](https://arxiv.org/abs/1109.2128) のうち, Continuous LexRankを実装しました.

## Requirement
- macOS
- VisualStudio
- C++ 12.0.5

## Usage
`$./lexrank ファイル名 べき乗法の回数 テレポーテーションの確率`

## Description
- スパースな行列を疎行列として扱う

- 式変形を行い, 大量のデータでも計算できるようにした

- 隣接行列を計算する際にも式変形を行い，大量のデータでも計算できるようにした