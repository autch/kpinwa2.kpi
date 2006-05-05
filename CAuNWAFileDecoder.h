
#pragma once

#include "stdafx.h"
#include "CAuNWAFile.h"

class CAuNWAFileDecoder
{
private:
  CAuNWAFile* m_pFile;
  BYTE* m_pBuffer;          // 内部バッファ
  DWORD m_dwBufferCapacity; // m_pBuffer の最大容量＝確保容量
  DWORD m_dwBufferSize;     // m_pBuffer に入っているサイズ
  DWORD m_dwTotalSamples;   // これまでに Render() してきたサンプル数
  VOID ParseIni(FILE* file, char* szBgmName);
  BOOL RenderSub(BYTE* pBuffer, DWORD dwSize, DWORD& dwBytesRendered);

public:
  CAuNWAFileDecoder()
  {
    m_pFile = NULL;
    m_pBuffer = NULL;
    m_dwBufferSize = 0;
    m_dwTotalSamples = 0;
  }
  virtual ~CAuNWAFileDecoder() { Close(); }
  BOOL Open(LPSTR szFileName, SOUNDINFO* pInfo);
  VOID Close();
  DWORD Render(BYTE* pBuffer, DWORD dwSize);
  DWORD SetPosition(DWORD dwPos);
};
