
#pragma once

#include "stdafx.h"

#pragma pack(push, 1)

// NWA �t�@�C���w�b�_
struct NWAHeader
{
  SHORT sChannels;             // �`�����l���� (1 / 2)
  SHORT sBitsPerSample;        // 1 �T���v��������̃r�b�g�� (8 / 16)
  DWORD   nSamplesPerSec;      // �T���v�����O���g��
  INT   nCompressionLevel;     // ���k���x���F-1...2, 2 �ōŏ��̃f�[�^�A0 �ōő�̕����x�A-1 �Ȃ琶 PCM
  INT   bIsNWK;                // �{�C�X�t���O
  DWORD   nBlocks;             // �u���b�N��
  DWORD   nExpandedSize;       // �W�J��̃f�[�^�̑傫���i�o�C�g�P�ʁj
  DWORD   nCompressedSize;     // ���k���̃f�[�^�̑傫���inwa �t�@�C���̑傫���B�o�C�g�P�ʁj
  DWORD nSamples;            // �T���v�����F�W�J��̃f�[�^���i16bit �Ȃ� short �P�� ie. �T���v���P�ʂ̃f�[�^�̑傫���j
  DWORD   nSamplesPerBlock;    // �f�[�^ 1 �u���b�N��W�J�������̃T���v���P�ʂ̃f�[�^��
  DWORD   nSamplesInLastBlock; // �ŏI�u���b�N��W�J�������̃T���v���P�ʂ̃f�[�^��
  INT   nReserved2;          // ?
};

#define RAW_PCM_BLOCK_SIZE  4096   // �� PCM �������Ƃ��� nSamplesPerBlock

#pragma pack(pop)
