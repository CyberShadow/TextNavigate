//TextNavigate.cpp

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "plugin.h"
#include "TextNavigate.h"

#include "crtdll.h"
#include "commons.h"
#include "reg.h"
#include "CRegExp.h"
#include "Dialog.h"
#include "sgml.h"
#include "pclasses.h"
#include "TextNavigate.h"

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

CTextNavigate* TextNavigate = NULL;

inline const char *get_msg(int id)
{
  return Info.GetMsg(Info.ModuleNumber, id);
}

#ifdef __cplusplus
extern "C"
#endif

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *PSInfo)
{
  ::Info = *PSInfo;
  FSF = *PSInfo->FSF;
  Info.FSF = &FSF;
  DebugString("SetStartupInfo:");

  RegistryStorage = new TRegistryStorage();
  TextNavigate = new CTextNavigate();
} //SetStartupInfo

void WINAPI _export GetPluginInfo(struct PluginInfo *PInfo)
{
  DebugString("GetPluginInfo");
  PInfo->StructSize = sizeof(struct PluginInfo);
  PInfo->Flags = PF_DISABLEPANELS|PF_EDITOR;

  static const char *PMStrings[] = { get_msg(STitle) };

  PInfo->PluginMenuStrings = PMStrings;
  PInfo->PluginMenuStringsNumber = 1;
  PInfo->PluginConfigStrings = PMStrings;
  PInfo->PluginConfigStringsNumber = 1;
} //GetPluginInfo

int WINAPI _export GetMinFarVersion(void) { return (MAKEFARVERSION(1, 70, 0)); };

HANDLE WINAPI _export OpenPlugin(int, int)
{
  if (TextNavigate)
    TextNavigate->ShowPluginMenu();

  return INVALID_HANDLE_VALUE;
} //OpenPlugin

void  WINAPI _export ExitFAR()
{
  delete RegistryStorage;
  delete TextNavigate;
  TextNavigate = NULL;
} //ExitFAR

int WINAPI _export Configure(int)
{
  if (TextNavigate)
    TextNavigate->config_plugin();
  return false;
} //Configure

int WINAPI _export ProcessEditorInput(const INPUT_RECORD *Rec)
{
  if (!plugin_options.b_active || !TextNavigate) return 0;
  return TextNavigate->ProcessEditorInput(Rec);
} //ProcessEditorInput

int WINAPI _export ProcessEditorEvent(int Event, void *Param)
{
  //if (!b_active) return 0;
  if (!TextNavigate) return 0;
  return TextNavigate->ProcessEditorEvent(Event, Param);
} //ProcessEditorEvent

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  return true;
}
