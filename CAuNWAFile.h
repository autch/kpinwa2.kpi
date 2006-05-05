
#pragma once

#include "stdafx.h"
#include "NWA.h"

/**
 * CAuNWAFile - NWA ファイルのコンテナクラス
 */

class CAuNWAFile
{
private:
  HANDLE m_hFile;
  NWAHeader m_Header;
  BYTE* m_pBlockBuffer;
  DWORD* m_pBlockOffsets;

  // 中途半端なサイズのデコードに使うバッファ
  BYTE* m_pDecodedBuffer;
  DWORD m_dwDecodedBufferSize;
  DWORD m_dwDecodedBufferCapacity;

  SOUNDINFO m_SoundInfo;
  DWORD m_dwCurrentBlock;
  BOOL m_bRawPCM;
  DWORD m_dwLoopPoint, m_dwEndPoint;
  DWORD m_dwSamplesToSkip;
  BOOL m_bIsLooped;

  BOOL ReadHeader();
  BOOL CheckHeader();

  DWORD NWADecode(BYTE* pOutput);

  BOOL DecodeSub(BYTE* pBuffer, DWORD dwSize, DWORD& dwBytesRendered);

  inline int GetBits(BYTE*& pData, int& nShiftBits, int nBits) const
  {
	  if(nShiftBits >= 8) { pData++; nShiftBits &= 0x07; }
	  int r = *(SHORT*)pData >> nShiftBits;
	  nShiftBits += nBits;
	  return r & ((1 << nBits) - 1); /* mask */
  }

	inline VOID WriteInitials(int* pDst, BYTE*& pSrc, SHORT sBitsPerSample)
	{
		if(!pDst) return;
		if(sBitsPerSample == 8) { *pDst = *pSrc++; }
		else { *pDst = *(SHORT*)pSrc; pSrc += 2; }
	}

	inline VOID WriteDecoded(BYTE*& pDst, int* pSrc, SHORT sBitsPerSample)
	{
		if(!pDst) return;
		if(sBitsPerSample == 8)
			*pDst++ = *pSrc;
		else
		{
      *(SHORT*)pDst = *pSrc;
			pDst += 2;
		}
	}

public:
  virtual ~CAuNWAFile() { Close(); }
  CAuNWAFile()
  {
    m_hFile = NULL;
    m_pBlockBuffer = NULL;
    m_pBlockOffsets = NULL;
    m_pDecodedBuffer = NULL;
    m_dwDecodedBufferSize = 0;
    ZeroMemory(&m_Header, sizeof m_Header);
    ZeroMemory(&m_SoundInfo, sizeof m_SoundInfo);
    m_bRawPCM = FALSE;
    m_bIsLooped = FALSE;
    m_dwCurrentBlock = 0;
    m_dwLoopPoint = m_dwEndPoint = 0;
    m_dwSamplesToSkip = 0;
  }
  BOOL Open(LPSTR lpszFileName);
  VOID Close();
  DWORD SeekBySamples(DWORD dwSamples);
  int Decode(BYTE* pOutput, DWORD dwSize, DWORD& dwBytesDecoded);

  const NWAHeader& GetHeader() const { return m_Header; }
  const SOUNDINFO* GetSoundInfo() const { return &m_SoundInfo; }
  BOOL IsRawPCM() const { return m_bRawPCM; }
  VOID SetLoopPoint(DWORD dwLoopPoint) { m_dwLoopPoint = dwLoopPoint; }
  DWORD GetLoopPoint() const { return m_dwLoopPoint; }
  VOID SetEndPoint(DWORD dwEndPoint) { m_dwEndPoint = dwEndPoint; }
  DWORD GetEndPoint() const { return m_dwEndPoint; }
  BOOL IsInLastBlock() const { return m_dwCurrentBlock == (m_Header.nBlocks - 1); }
  VOID SetLooped(BOOL b) { m_bIsLooped = b; }
  BOOL GetLooped() const { return m_bIsLooped; }

  // NokomoSystem より引用
  DWORD BytesPerChannel() const { return m_Header.sBitsPerSample >> 3; }
  DWORD Channels() const { return m_Header.sChannels; }
  DWORD SamplesPerSecond() const { return m_Header.nSamplesPerSec; }
  DWORD SamplesToBytes(DWORD dwSamples) const { return BytesPerChannel() * dwSamples; }
  DWORD BytesToSamples(DWORD dwBytes) const { return dwBytes / BytesPerChannel(); }
  DWORD BytesPerSecond() const { return SamplesToBytes(SamplesPerSecond()); }

  DWORD SamplesToMSecs(DWORD dwSamples) const { return MulDiv(dwSamples, 1000, SamplesPerSecond()); }
  DWORD MSecsToSamples(DWORD dwMSecs) const { return MulDiv(dwMSecs, SamplesPerSecond(), 1000); }
  DWORD BytesToMSecs(DWORD dwBytes) const { return SamplesToMSecs(BytesToSamples(dwBytes)); }
  DWORD MSecsToBytes(DWORD dwMSecs) const { return SamplesToBytes(MSecsToSamples(dwMSecs)); }

  DWORD SamplesToMSecs2(DWORD dwSamples) const { return MulDiv(dwSamples / Channels(), 1000, SamplesPerSecond()); }
  DWORD SamplesToBytes2(DWORD dwSamples) const { return BytesPerChannel() * dwSamples * Channels(); }
  DWORD BytesToSamples2(DWORD dwBytes) const { return dwBytes / Channels() / BytesPerChannel(); }
  DWORD MSecsToSamples2(DWORD dwMSecs) const { return MulDiv(dwMSecs, SamplesPerSecond() * Channels(), 1000); }
};
