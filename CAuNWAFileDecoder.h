
#pragma once

#include "stdafx.h"
#include "CAuNWAFile.h"

class CAuNWAFileDecoder
{
private:
  CAuNWAFile* m_pFile;
  BYTE* m_pBuffer;          // �����o�b�t�@
  DWORD m_dwBufferCapacity; // m_pBuffer �̍ő�e�ʁ��m�ۗe��
  DWORD m_dwBufferSize;     // m_pBuffer �ɓ����Ă���T�C�Y
  DWORD m_dwTotalSamples;   // ����܂ł� Render() ���Ă����T���v����
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
