
#pragma once

#include "stdafx.h"

#pragma pack(push, 1)

// NWA ファイルヘッダ
struct NWAHeader
{
  SHORT sChannels;             // チャンネル数 (1 / 2)
  SHORT sBitsPerSample;        // 1 サンプルあたりのビット数 (8 / 16)
  DWORD   nSamplesPerSec;      // サンプリング周波数
  INT   nCompressionLevel;     // 圧縮レベル：-1...2, 2 で最小のデータ、0 で最大の復元度、-1 なら生 PCM
  INT   bIsNWK;                // ボイスフラグ
  DWORD   nBlocks;             // ブロック数
  DWORD   nExpandedSize;       // 展開後のデータの大きさ（バイト単位）
  DWORD   nCompressedSize;     // 圧縮時のデータの大きさ（nwa ファイルの大きさ。バイト単位）
  DWORD nSamples;            // サンプル数：展開後のデータ数（16bit なら short 単位 ie. サンプル単位のデータの大きさ）
  DWORD   nSamplesPerBlock;    // データ 1 ブロックを展開した時のサンプル単位のデータ数
  DWORD   nSamplesInLastBlock; // 最終ブロックを展開した時のサンプル単位のデータ数
  INT   nReserved2;          // ?
};

#define RAW_PCM_BLOCK_SIZE  4096   // 生 PCM だったときの nSamplesPerBlock

#pragma pack(pop)
