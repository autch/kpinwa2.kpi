
#pragma once

#include "stdafx.h"

class CAuLoopedPCMRenderer
{
private:
  DWORD m_dwUnitRender; // 「真の」UnitRender, 0 はだめ
  BYTE* m_pbyBuffer;    // ちょうど dwUnitRender だけ確保する
  BYTE* m_pbyUnitBuffer;// ちょうど dwUnitRender だけ確保する
  DWORD m_dwBytesFilled;// m_pbyBuffer に入っている PCM
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
  // このポインタにデコーダが fill する。
  BYTE* GetUnitBuffer()
  {
    return m_pbyUnitBuffer;
  }
  BOOL MakeupBuffer(DWORD dwBytesRendered, 

};
