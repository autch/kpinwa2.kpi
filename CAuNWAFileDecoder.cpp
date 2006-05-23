
#include "stdafx.h"
#include "CAuNWAFileDecoder.h"

BOOL CAuNWAFileDecoder::Open(LPSTR szFileName, SOUNDINFO* pInfo)
{
  m_pFile = new CAuNWAFile();

  // ���[�v�Ή�
  FILE* inifile;
  char szIniFileName[_MAX_PATH];

  strcpy(szIniFileName, szFileName);
  strcpy(strrchr(szIniFileName, '\\'), "\\GAMEEXE.INI");

  inifile = fopen(szIniFileName, "r");
  if(inifile)
  {
    char szBgmName[_MAX_FNAME];

    strcpy(szBgmName, strrchr(szFileName, '\\') + 1);
    *strrchr(szBgmName, '.') = '\0';

    m_pFile->SetLooped(FALSE);
    m_pFile->SetLoopPoint(0);
    m_pFile->SetEndPoint(0);

    ParseIni(inifile, szBgmName);

    fclose(inifile);
    if((m_pFile->GetLoopPoint() == 0 && m_pFile->GetEndPoint() == 99999999)
      || (m_pFile->GetLoopPoint() == 99999999 && m_pFile->GetEndPoint() == 99999999))
    {
      m_pFile->SetLooped(FALSE);
      m_pFile->SetLoopPoint(0);
      m_pFile->SetEndPoint(0);
    }
  }
  else
  {
    m_pFile->SetLooped(FALSE);
    m_pFile->SetLoopPoint(0);
    m_pFile->SetEndPoint(0);
  }

  if(m_pFile->Open(szFileName))
  {
    CopyMemory(pInfo, m_pFile->GetSoundInfo(), sizeof SOUNDINFO);

    m_dwBufferCapacity = pInfo->dwUnitRender << 1;
    m_pBuffer = new BYTE[m_dwBufferCapacity];
    m_dwBufferSize = 0;
    m_dwTotalSamples = 0;

    return TRUE;
  }
  return FALSE;
}

VOID CAuNWAFileDecoder::Close()
{
  delete m_pFile;
  delete[] m_pBuffer;
  m_pFile = NULL;
}

// Render() �̉�����
// dwUnitRender() ��菬���� dwSize ��n���Ă����R�Ɠ���
// dwSize ��菬���� dwBytesRendered ��Ԃ�����t�@�C���̏I���
// FALSE ��Ԃ�����G���[
BOOL CAuNWAFileDecoder::RenderSub(BYTE* pBuffer, DWORD dwSize, DWORD& dwBytesRendered)
{
  DWORD dwUnitRender = m_pFile->GetSoundInfo()->dwUnitRender;
  DWORD dwBytesToDecode = dwSize;
  BYTE* pBufferWrite = pBuffer;
  dwBytesRendered = 0;
  if(m_dwBufferSize)
  {
    // �O�̕��̐ςݎc��������
    DWORD dwBytesToCopy = (dwBytesToDecode < m_dwBufferSize) ? dwBytesToDecode : m_dwBufferSize;
    CopyMemory(pBufferWrite, m_pBuffer, dwBytesToCopy);    // m_pBuffer �̃T�C�Y�� dwBytesToDecode == dwSize == dwUnitRender �����傫��
    MoveMemory(m_pBuffer, m_pBuffer + dwBytesToCopy, m_dwBufferCapacity - dwBytesToCopy);
    pBufferWrite += dwBytesToCopy;
    dwBytesToDecode -= dwBytesToCopy;
    m_dwBufferSize -= dwBytesToCopy;
    dwBytesRendered += dwBytesToCopy;
  }
  if(dwBytesToDecode)
  {
    DWORD dwBytesDecoded = 0;
    if(m_pFile->Decode(m_pBuffer, dwUnitRender, dwBytesDecoded))
    {
      DWORD dwBytesToCopy = (dwBytesDecoded < dwBytesToDecode) ? dwBytesDecoded : dwBytesToDecode;
      CopyMemory(pBufferWrite, m_pBuffer, dwBytesToCopy);
      MoveMemory(m_pBuffer, m_pBuffer + dwBytesToCopy, m_dwBufferCapacity - dwBytesToCopy);
      m_dwBufferSize += dwBytesDecoded - dwBytesToCopy;
      dwBytesRendered += dwBytesToCopy;
    }
  }
  return TRUE;
}

DWORD CAuNWAFileDecoder::Render(BYTE* pBuffer, DWORD dwSize)
{
  PBYTE pCurrentBuffer = pBuffer, pBufferEnd = (pBuffer + dwSize);
  DWORD dwBytesDecoded = 0;

  if(m_pFile->GetLooped())
  {
    // ���[�v�ʒu�̎w�肪����ꍇ
    DWORD dwUnitRender = m_pFile->GetSoundInfo()->dwUnitRender;                       // SOUNDINFO::dwUnitRender
    DWORD dwLoopStart = m_pFile->GetLoopPoint(), dwLoopEnd = m_pFile->GetEndPoint();  // ���[�v�X�^�[�g�E�G���h�i�T���v���ʒu�j
    DWORD dwSamplesDecoded = m_pFile->BytesToSamples2(dwUnitRender);                  // ����f�R�[�h����T���v����
    DWORD dwDecodedEnd = m_dwTotalSamples + dwSamplesDecoded;                         // ����܂łɃf�R�[�h���鑍�T���v����
    if(dwLoopEnd <= dwDecodedEnd)                                                     // ����܂łɃf�R�[�h����T���v���������[�v�G���h���z����
    {
      DWORD dwBytesToRender = m_pFile->SamplesToBytes2(dwLoopEnd - m_dwTotalSamples);
      DWORD dwTotalBytesRendered = 0;
      if(RenderSub(pBuffer, dwBytesToRender, dwBytesDecoded))
        dwTotalBytesRendered += dwBytesDecoded;
      m_dwTotalSamples = m_pFile->SeekBySamples(dwLoopStart);

      while(dwTotalBytesRendered < dwSize)
      {
        // ���� RenderSub() �� EOF �������肵�Ď��s�����Ƃ��́A�o�b�t�@�̎c���S�����߂�
        dwBytesToRender = (dwUnitRender < (dwSize - dwTotalBytesRendered)) ? dwBytesToRender : (dwSize - dwTotalBytesRendered);
        if(RenderSub(pBuffer + dwBytesDecoded, dwBytesToRender, dwBytesDecoded))
          dwTotalBytesRendered += dwBytesDecoded;
      }
      m_dwTotalSamples += m_pFile->BytesToSamples2(dwTotalBytesRendered);
      return dwTotalBytesRendered;
    }
    else
    {
      // �܂����[�v�G���h�ɒB���Ȃ�
      if(!RenderSub(pBuffer, dwSize, dwBytesDecoded))
        return 0;
      m_dwTotalSamples += m_pFile->BytesToSamples2(dwBytesDecoded);
      return dwBytesDecoded;
    }
  }
  else
  {
    // ���[�v���Ȃ��Ƃ��́ApBuffer �ɒ��ڏ����Ă��܂�
    if(!m_pFile->Decode(pBuffer, dwSize, dwBytesDecoded))
      return 0;
    return dwBytesDecoded;
  }

  return dwBytesDecoded;
}

DWORD CAuNWAFileDecoder::SetPosition(DWORD dwPos)
{
  m_dwBufferSize = 0;
  DWORD dwSamplesToSeek = m_pFile->MSecsToSamples(dwPos);
  m_dwTotalSamples = m_pFile->SeekBySamples(dwSamplesToSeek);
  return dwPos;
}

VOID CAuNWAFileDecoder::ParseIni(FILE* file, char* szBgmName)
{
  char buf[128];
  char param[11][128];

  while(fgets(buf, sizeof buf, file))
  {
    sscanf(buf,
      "%s %s %s %s %s %s %s %s %s %s %s",
      &param[0], &param[1], &param[2], &param[3], &param[4],
      &param[5], &param[6], &param[7], &param[8], &param[9],
      &param[10]);

    if(!strcmp(param[0], "#DSTRACK"))
    {
      param[8][strlen(param[8]) - 1] = '\0';
      if(!stricmp(param[8] + 1, szBgmName))
      {
        m_pFile->SetLooped(TRUE);
        m_pFile->SetEndPoint(atoi(param[4]));
        m_pFile->SetLoopPoint(atoi(param[6]));
        return;
      }
    }
  }
  m_pFile->SetLooped(FALSE);
  m_pFile->SetLoopPoint(0);
  m_pFile->SetEndPoint(0);
}
