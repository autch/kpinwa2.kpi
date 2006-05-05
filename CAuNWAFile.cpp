
#include "stdafx.h"
#include "CAuNWAFile.h"

BOOL CAuNWAFile::Open(LPSTR lpszFileName)
{
  m_hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if(m_hFile)
    return ReadHeader();
  return FALSE;
}

VOID CAuNWAFile::Close()
{
  delete[] m_pBlockOffsets; m_pBlockOffsets = NULL;
  delete[] m_pBlockBuffer; m_pBlockBuffer = NULL;
  delete[] m_pDecodedBuffer; m_pDecodedBuffer = NULL;
  if(m_hFile) { CloseHandle(m_hFile); m_hFile = NULL; }
  ZeroMemory(&m_Header, sizeof m_Header);
  ZeroMemory(&m_SoundInfo, sizeof m_SoundInfo);
  m_bRawPCM = FALSE;
  m_dwCurrentBlock = 0;
  m_dwDecodedBufferSize = 0;
}

BOOL CAuNWAFile::ReadHeader()
{
  DWORD dwDummy;
  if(!m_hFile) return FALSE;
  if(!ReadFile(m_hFile, &m_Header, sizeof m_Header, &dwDummy, NULL))
    return FALSE;
  if(m_bRawPCM = (m_Header.nCompressionLevel == -1))
  {
    // 適当にでっち上げる
    m_Header.nBlocks = 1;
    m_Header.nSamplesPerBlock = RAW_PCM_BLOCK_SIZE;
  }
  DWORD nFileSize = GetFileSize(m_hFile, NULL), nCurrentPos = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
  if(nCurrentPos + (m_Header.nBlocks << 2) >= nFileSize)
    // ブロックオフセットテーブルがファイルに収まっていない
    return FALSE;
  if(!m_bRawPCM)
  {
    // 中途半端なサイズでのデコードをするためのバッファ
    m_dwDecodedBufferCapacity = m_Header.nSamplesPerBlock * BytesPerChannel() * 2;
    m_pDecodedBuffer = new BYTE[m_dwDecodedBufferCapacity];
    // 生 PCM nwa にはオフセットテーブルがない
    m_pBlockOffsets = new DWORD[m_Header.nBlocks];
    if(!ReadFile(m_hFile, m_pBlockOffsets, m_Header.nBlocks << 2, &dwDummy, NULL))
      return FALSE;
  }
  m_SoundInfo.dwBitsPerSample = m_Header.sBitsPerSample;
  m_SoundInfo.dwChannels = m_Header.sChannels;
  m_SoundInfo.dwSamplesPerSec = m_Header.nSamplesPerSec;
  m_SoundInfo.dwUnitRender = m_Header.nSamplesPerBlock * BytesPerChannel();
  m_SoundInfo.dwSeekable = TRUE;
  m_SoundInfo.dwMultiSongFlag = FALSE;
  if(!m_bIsLooped)
  {
    m_SoundInfo.dwLength = SamplesToMSecs2(m_Header.nSamples);
    m_SoundInfo.dwLoopFlag = FALSE;
  }
  else
  {
    // 頭からエンドポイントまでの長さ
    if(m_dwEndPoint == 99999999) m_dwEndPoint = m_Header.nSamples / Channels();
    DWORD dwFullLength = SamplesToMSecs(m_dwEndPoint);
    DWORD dwToLoopPoint = SamplesToMSecs(m_dwLoopPoint);   // イントロの長さ
    m_SoundInfo.dwLength = dwToLoopPoint + (dwFullLength - dwToLoopPoint) * 2;
    m_SoundInfo.dwLoopFlag = TRUE;
  }
  return CheckHeader();
}

BOOL CAuNWAFile::CheckHeader()
{
  if(!m_hFile) return FALSE;
  BOOL f = TRUE;
  f = f && (m_bRawPCM || m_pBlockOffsets);
  f = f && (m_Header.sChannels == 1 || m_Header.sChannels == 2);
  f = f && (m_Header.sBitsPerSample == 8 || m_Header.sBitsPerSample == 16);
  f = f && ((m_Header.nCompressionLevel >= -1 && m_Header.nCompressionLevel <= 2) || m_Header.nCompressionLevel == 5);
  if(m_bRawPCM) return f;     // 生 PCM のときはここまで
  f = f && (GetFileSize(m_hFile, NULL) == m_Header.nCompressedSize);
  f = f && (m_pBlockOffsets[m_Header.nBlocks - 1] < (DWORD)m_Header.nCompressedSize);
  f = f && (m_Header.nExpandedSize == m_Header.nSamples * BytesPerChannel());
  f = f && (m_Header.nSamples == (m_Header.nBlocks - 1) * m_Header.nSamplesPerBlock + m_Header.nSamplesInLastBlock);
  m_pBlockBuffer = new BYTE[m_Header.nSamplesPerBlock * (m_Header.sBitsPerSample >> 2)];
  return f;
}

DWORD CAuNWAFile::SeekBySamples(DWORD dwSamples)
{
  if(m_bIsLooped)
  {
    if(dwSamples > m_dwEndPoint)
    {
      dwSamples -= m_dwLoopPoint;
      dwSamples %= (m_dwEndPoint - m_dwLoopPoint);
      dwSamples += m_dwLoopPoint;
    }
  }
  if(!m_bRawPCM)
  {
    m_dwCurrentBlock = dwSamples * Channels() / m_Header.nSamplesPerBlock;
    SetFilePointer(m_hFile, m_pBlockOffsets[m_dwCurrentBlock], NULL, FILE_BEGIN);
    m_dwSamplesToSkip = (dwSamples * Channels()) % m_Header.nSamplesPerBlock;

    return dwSamples;
  }
  DWORD dwBytePos = SamplesToBytes2(dwSamples) + sizeof m_Header;
  DWORD r = SetFilePointer(m_hFile, dwBytePos, NULL, FILE_BEGIN);
  m_dwSamplesToSkip = 0;
  return BytesToSamples2(r - sizeof m_Header);
}

// Decode() の下請け
// ブロックサイズより小さい dwSize を渡しても平然と動く
// dwSize より小さい dwBytesRendered を返したらファイルの終わり
// FALSE を返したらエラー
BOOL CAuNWAFile::DecodeSub(BYTE* pBuffer, DWORD dwSize, DWORD& dwBytesRendered)
{
  DWORD dwBytesToDecode = dwSize;
  BYTE* pBufferWrite = pBuffer;
  dwBytesRendered = 0;
  if(m_dwDecodedBufferSize)
  {
    // 前の分の積み残しがある
    DWORD dwBytesToCopy = (dwBytesToDecode < m_dwDecodedBufferSize) ? dwBytesToDecode : m_dwDecodedBufferSize;
    CopyMemory(pBufferWrite, m_pDecodedBuffer, dwBytesToCopy);
    MoveMemory(m_pDecodedBuffer, m_pDecodedBuffer + dwBytesToCopy, m_dwDecodedBufferCapacity - dwBytesToCopy);
    pBufferWrite += dwBytesToCopy;
    dwBytesToDecode -= dwBytesToCopy;
    m_dwDecodedBufferSize -= dwBytesToCopy;
    dwBytesRendered += dwBytesToCopy;
  }
  if(dwBytesToDecode)
  {
    DWORD dwBytesDecoded = 0, dwBytesRead = 0;
    int nCurrentCompressedSize;

    if(IsInLastBlock())
      nCurrentCompressedSize = SamplesToBytes2(m_Header.nSamplesInLastBlock) << 1;
    else
      nCurrentCompressedSize = m_pBlockOffsets[m_dwCurrentBlock + 1] - m_pBlockOffsets[m_dwCurrentBlock];
    ReadFile(m_hFile, m_pBlockBuffer, nCurrentCompressedSize, &dwBytesRead, NULL);
    ZeroMemory(&m_pBlockBuffer[dwBytesRead], m_Header.nSamplesPerBlock * (m_Header.sBitsPerSample >> 2) - dwBytesRead);

    /* 展開 */
	  dwBytesDecoded = NWADecode(m_pDecodedBuffer);
	  m_dwCurrentBlock++;
    if(m_dwSamplesToSkip)
    {
      DWORD dwBytesToSkip = SamplesToBytes(m_dwSamplesToSkip);
      dwBytesDecoded -= dwBytesToSkip;
      MoveMemory(m_pDecodedBuffer, m_pDecodedBuffer + dwBytesToSkip, dwBytesDecoded);
      m_dwSamplesToSkip = 0;
    }

    DWORD dwBytesToCopy = (dwBytesDecoded < dwBytesToDecode) ? dwBytesDecoded : dwBytesToDecode;
    CopyMemory(pBufferWrite, m_pDecodedBuffer, dwBytesToCopy);
    MoveMemory(m_pDecodedBuffer, m_pDecodedBuffer + dwBytesToCopy, m_dwDecodedBufferCapacity - dwBytesToCopy);
    m_dwDecodedBufferSize += dwBytesDecoded - dwBytesToCopy;
    dwBytesRendered += dwBytesToCopy;
  }
  return TRUE;
}

int CAuNWAFile::Decode(BYTE* pOutput, DWORD dwSize, DWORD& dwBytesRendered)
{
	if(!m_bRawPCM && (!m_pBlockOffsets || !m_pBlockBuffer)) return FALSE;
	if(m_dwCurrentBlock == m_Header.nBlocks) return FALSE;
	if(GetFileSize(m_hFile, NULL) == SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT)) return FALSE;

  if(m_bRawPCM)
  {
    DWORD dwBytesRead = 0;
    ReadFile(m_hFile, pOutput, dwSize, &dwBytesRead, NULL);
    dwBytesRendered = dwBytesRead;
    return TRUE;
  }

  dwBytesRendered = 0;
  DWORD dwBytesToDecode = dwSize, dwBytesDecoded = 0;
  BYTE* pOutputWrite = pOutput;
  while(dwBytesToDecode)
  {
    DecodeSub(pOutputWrite, dwBytesToDecode, dwBytesDecoded);
    dwBytesToDecode -= dwBytesDecoded;
    pOutputWrite += dwBytesDecoded;
    dwBytesRendered += dwBytesDecoded;
  }
  return TRUE;
}

DWORD CAuNWAFile::NWADecode(BYTE* pOutput)
{
	int nSample[2];
	int nShiftBits = 0;
  BYTE* pSrcBuffer = m_pBlockBuffer;
  BYTE* pOutputBackup = pOutput;
  SHORT sBitsPerSample = m_Header.sBitsPerSample, sChannels = m_Header.sChannels;
  int nCompressionLevel = m_Header.nCompressionLevel;
  BOOL bRunLength = m_bUseRunLength;
  int nDstSamples = IsInLastBlock() ? m_Header.nSamplesInLastBlock : m_Header.nSamplesPerBlock;

  /* 圧縮レベルが 5 の場合。 */
  if(nCompressionLevel == 5)
  {
    return NWA5Decode(pOutput);
  }

  /* 最初のデータを読み込む */
  WriteInitials(nSample, pSrcBuffer, sBitsPerSample);
  if(sChannels == 2)
    WriteInitials(nSample + 1, pSrcBuffer, sBitsPerSample);

  int nChannel = 0; /* stereo 用 */

  for(int i = 0; i < nDstSamples; i++)
  {
		int nSampleType = GetBits(pSrcBuffer, nShiftBits, 3);
		/* nSampleType により分岐：0, 1-6, 7 */
    if(nSampleType)
    {
			if(nSampleType == 7 && GetBits(pSrcBuffer, nShiftBits, 1))
				nSample[nChannel] = 0; // 未使用
      else
      {
        // patched 2005.10.30
        const int BITS = (nCompressionLevel == 5) ? 8 : ((nSampleType == 7) ? 8 : 5) - nCompressionLevel;
        const int SHIFT = (nCompressionLevel == 5) ? nSampleType + 1 + ((nSampleType == 7) ? 1 : 0): 2 + nSampleType + nCompressionLevel;
			  const int MASK1 = 1 << (BITS - 1);
			  const int MASK2 = MASK1 - 1;
			  int b = GetBits(pSrcBuffer, nShiftBits, BITS);
			  if(b & MASK1)
				  nSample[nChannel] -= (b & MASK2) << SHIFT;
			  else
				  nSample[nChannel] += (b & MASK2) << SHIFT;
      }
    }
    else
    {
      /* nSampleType == 0 */
			/* ランレングス圧縮なしの場合はなにもしない */
			if(m_bUseRunLength)
      {
				/* ランレングス圧縮ありの場合 */
				int nLength = GetBits(pSrcBuffer, nShiftBits, 1);
				if(nLength == 1)
        {
					nLength = GetBits(pSrcBuffer, nShiftBits, 2);
					if(nLength == 3)
						nLength = GetBits(pSrcBuffer, nShiftBits, 8);
				}
				/* 前のデータと同じデータを書いていく */
				for(int j = 0; j <= nLength; j++)
        {
            WriteDecoded(pOutput, nSample + nChannel, sBitsPerSample);
					if(sChannels == 2) nChannel ^= 1;
				}
				i += nLength;
        continue;
			}
		}
    WriteDecoded(pOutput, nSample + nChannel, sBitsPerSample);
		if(sChannels == 2) nChannel ^= 1; /* channel 切り替え */
	}
  return (DWORD)(pOutput - pOutputBackup);
};

DWORD CAuNWAFile::NWA5Decode(BYTE* pOutput)
{
	int nOffset = 0;
  short nSample, nTemp[2];
	int nShiftBits = 0;
  BYTE* pSrcBuffer = m_pBlockBuffer;
  BYTE* pOutputBackup = pOutput;
  SHORT sBitsPerSample = m_Header.sBitsPerSample, sChannels = m_Header.sChannels;
  int nDstSamples = IsInLastBlock() ? m_Header.nSamplesInLastBlock : m_Header.nSamplesPerBlock;
  int nBytesToRender = nDstSamples * (sBitsPerSample >> 3);
  int nSamplesDecoded = 0, nRun = 0, nCurrentChannel = 0;
  int bIsMono = (sChannels == 1); //m_Header.bIsNWK;

  nTemp[0] = *(signed short *)&pSrcBuffer[0];
  nTemp[1] = *(signed short *)&pSrcBuffer[2];

  pSrcBuffer += 2;
  *(short*)pOutput = nTemp[0];
  pOutput += 2;
  if(!bIsMono)
  {
    *(short*)pOutput = nTemp[1];
    pOutput += 2;
    pSrcBuffer += 2;
  }

  if(!nDstSamples) return 0;
  while(nSamplesDecoded < nDstSamples)
  {
    nSample = nTemp[nCurrentChannel];
    if(nRun)
    {
      nRun--;
      goto lWriteSample;
    }
    int nSampleType = GetBits(pSrcBuffer, nShiftBits, 3);
    switch(nSampleType)
    {
      case 0:
      {
        // ランレングス？
        if(!bIsMono) break;

        nRun = GetBits(pSrcBuffer, nShiftBits, 1);
        if(nRun != 0x01) break;

        nRun = GetBits(pSrcBuffer, nShiftBits, 2);
        if(nRun != 0x03) break;

        nRun = GetBits(pSrcBuffer, nShiftBits, 8);
        break;
      }
      case 1: case 2: case 3: case 4: case 5: case 6:
      {
        int i = GetBits(pSrcBuffer, nShiftBits, 8);
        if(i & 0x80)
        {
          i &= 0x7f;
          nSample -= (i << (nSampleType + 1));
          break;
        }
        nSample += (i << (nSampleType + 1));
        break;
      }
      case 7:
      {
        int b = GetBits(pSrcBuffer, nShiftBits, 1);
        if(!b)
        {
          int i = GetBits(pSrcBuffer, nShiftBits, 8);
          if(i & 0x80)
          {
            i &= 0x7f;
            nSample -= (i << 9);
            break;
          }
          nSample += i << 9;
          break;
        }
        nSample = 0;
      }
    }
lWriteSample:
    if(nSamplesDecoded >= nOffset)
    {
      if(nSamplesDecoded >= nDstSamples)
        break;
      *(short*)pOutput = nSample;
      pOutput += 2;
    }
    nTemp[nCurrentChannel] = nSample;
    nSamplesDecoded++;
    nCurrentChannel ^= 1;
    if(bIsMono) nCurrentChannel = 0;
  } // while

  return (DWORD)(pOutput - pOutputBackup); //nDstSamples * (sBitsPerSample >> 3);
}
