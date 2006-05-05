// kpinwa2.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "kpiEntryPoints.h"

#define KPINWA2_VERSION   ((2 << 8) | 20)      // 2.20
#ifdef _DEBUG
#define KPINWA2_DESC      "VisualArts AVG32 nwa decoder for KbMedia Player [DEBUG]"
#else
#define KPINWA2_DESC      "VisualArts AVG32 nwa decoder for KbMedia Player"
#endif
#define KPINWA2_COPYRIGHT "Copyright (c) 2001-2002, jagarl / 2004-2006, S. Kino. / 2004-2006, Yui N."

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
  if(dwReasonForCall == DLL_PROCESS_ATTACH)
    DisableThreadLibraryCalls((HMODULE)hModule);
  return TRUE;
}

extern "C" KMPMODULE* WINAPI kmp_GetTestModule()
{
  static const char* pszExts[] = { ".nwa", NULL };
  static KMPMODULE kpiModule =
  {
		KMPMODULE_VERSION,				// DWORD dwVersion;
		KPINWA2_VERSION,    			// DWORD dwPluginVersion;
		KPINWA2_COPYRIGHT, 		    // const char	*pszCopyright;
		KPINWA2_DESC,							// const char	*pszDescription;
		pszExts,									// const char	**ppszSupportExts;
		1,												// DWORD dwReentrant;
		kpiInit,									// void (WINAPI *Init)(void);
		kpiDeinit,								// void (WINAPI *Deinit)(void);
		kpiOpen,									// HKMP (WINAPI *Open)(const char *cszFileName, SOUNDINFO *pInfo);
		NULL,	            				// HKMP (WINAPI *OpenFromBuffer)(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo);
		kpiClose,									// void (WINAPI *Close)(HKMP hKMP);
		kpiRender,								// DWORD (WINAPI *Render)(HKMP hKMP, BYTE* Buffer, DWORD dwSize);
		kpiSetPosition						// DWORD (WINAPI *SetPosition)(HKMP hKMP, DWORD dwPos);
  };
  return &kpiModule;
}
