
#pragma once

#include "stdafx.h"

class CAuLoopedPCMRenderer
{
private:
  DWORD m_dwUnitRender; // �u�^�́vUnitRender, 0 �͂���
  BYTE* m_pbyBuffer;    // ���傤�� dwUnitRender �����m�ۂ���
  BYTE* m_pbyUnitBuffer;// ���傤�� dwUnitRender �����m�ۂ���
  DWORD m_dwBytesFilled;// m_pbyBuffer �ɓ����Ă��� PCM
public:
  CAuLoopedPCMRenderer(DWORD dwUnitRender)
  {
    m_dwUnitRender = dwUnitRender;
    m_pbyBuffer = new BYTE[m_dwUnitRender];
    m_pbyUnitBuffer = new BYTE[m_dwUnitRender];
  }
  ~CAuLoopedPCMRenderer()
  {
    delete[] m_pbyBuffer;
    delete[] m_pbyUnitBuffer;
  }
  // ���̃|�C���^�Ƀf�R�[�_�� fill ����B
  BYTE* GetUnitBuffer()
  {
    return m_pbyUnitBuffer;
  }
  BOOL MakeupBuffer(DWORD dwBytesRendered, 

};
