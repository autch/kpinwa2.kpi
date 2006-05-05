
#include "stdafx.h"
#include "kpiEntryPoints.h"
#include "CAuNWAFileDecoder.h"

// kmp_GetTestModule() ‚Í kpinwa2.cpp ‚É

void WINAPI kpiInit()
{
}

void WINAPI kpiDeinit()
{
}

HKMP WINAPI kpiOpen(const char* cszFileName, SOUNDINFO* pInfo)
{
  CAuNWAFileDecoder* d = new CAuNWAFileDecoder();
  if(d->Open((LPSTR)cszFileName, pInfo))
    return (HKMP)d;
	return NULL;
}

void WINAPI kpiClose(HKMP hKMP)
{
  if(hKMP)
    delete (CAuNWAFileDecoder*)hKMP;
}

DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize)
{
  CAuNWAFileDecoder* d = (CAuNWAFileDecoder*)hKMP;
  if(d)
  {
    return d->Render(Buffer, dwSize);
  }
	return 0;
}

DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos)
{
  CAuNWAFileDecoder* d = (CAuNWAFileDecoder*)hKMP;
  if(d)
  {
    return d->SetPosition(dwPos);
  }
	return 0;
}
