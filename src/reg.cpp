//reg.cpp
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "reg.h"
#include "TextNavigate.h"

SPluginOptions plugin_options;

/*******************************************************************************
  TRegistryStorage
*******************************************************************************/

TRegistryStorage::TRegistryStorage()
{
  FarSprintf(PluginRegRootKey, "%s\\%s", Info.RootKey, "Text Navigate");
  plugin_options.b_active = GetRegKey("", REG_KEY_ACTIVE, true);
  plugin_options.b_adddigits = GetRegKey("", REG_KEY_DIGIT_AS_CHAR, true);
  plugin_options.b_casesensitive = GetRegKey("", REG_KEY_CASE_SENSITIVE, true);
  plugin_options.b_cyclicsearch = GetRegKey("", REG_KEY_CYCLIC_SEARCH, true);
  plugin_options.b_searchselection = GetRegKey("", REG_KEY_SEARCH_SELECTION, false);
  plugin_options.b_savebookmarks = GetRegKey("", REG_KEY_SAVE_BOOKMARKS, true);
  GetRegKey("", REG_KEY_ADDITIONAL_LETTERS, plugin_options.s_AdditionalLetters, DEFAULT_ADDITIONAL_LETTERS, MAX_CHAR_SET_LENGTH);
}

void TRegistryStorage::SavePluginOptions()
{
  SetRegKey("", REG_KEY_ACTIVE, plugin_options.b_active);
  SetRegKey("", REG_KEY_DIGIT_AS_CHAR, plugin_options.b_adddigits);
  SetRegKey("", REG_KEY_CASE_SENSITIVE, plugin_options.b_casesensitive);
  SetRegKey("", REG_KEY_CYCLIC_SEARCH, plugin_options.b_cyclicsearch);
  SetRegKey("", REG_KEY_SEARCH_SELECTION, plugin_options.b_searchselection);
  SetRegKey("", REG_KEY_SAVE_BOOKMARKS, plugin_options.b_savebookmarks);
  SetRegKey("", REG_KEY_ADDITIONAL_LETTERS, plugin_options.s_AdditionalLetters);
}

void TRegistryStorage::SetRegKey(const char *Key, const char *ValueName, BYTE *ValueData, DWORD ValueSize)
{
  HKEY hKey = CreateRegKey(Key);
  RegSetValueEx(hKey, ValueName, 0, REG_BINARY, ValueData, ValueSize);
  RegCloseKey(hKey);
}

void TRegistryStorage::SetRegKey(const char *Key, const char *ValueName, DWORD ValueData)
{
  HKEY hKey = CreateRegKey(Key);
  RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (BYTE *)&ValueData, sizeof(ValueData));
  RegCloseKey(hKey);
}

void TRegistryStorage::SetRegKey(const char *Key, const char *ValueName, bool ValueData)
{
  SetRegKey(Key, ValueName, (DWORD)ValueData);
}

void TRegistryStorage::SetRegKey(const char *Key, const char *ValueName, const char *ValueData)
{
  HKEY hKey = CreateRegKey(Key);
  RegSetValueEx(hKey, ValueName, 0, REG_SZ, (CONST BYTE *)ValueData, (DWORD)lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}

int TRegistryStorage::GetRegKeyEx(const char *Key, const char *ValueName, LPBYTE ValueData, const LPBYTE Default, DWORD DataSize)
{
  HKEY hKey = OpenRegKey(Key);
  DWORD Type, Required = DataSize;
  int ExitCode = RegQueryValueEx(hKey, ValueName, 0, &Type, ValueData, &Required);
  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
  {
    if (Default != NULL)
      crt::lmemcpy(ValueData, Default, DataSize);
    else
      ZeroMemory(ValueData, (int)DataSize);
    return false;
  }
  return DataSize;
}

int TRegistryStorage::GetRegKey(const char *Key, const char *ValueName, char *ValueData, char *Default, DWORD DataSize)
{
  return GetRegKeyEx(Key, ValueName, (LPBYTE)ValueData, (const LPBYTE)Default, DataSize);
}

DWORD TRegistryStorage::GetRegKey(const char *Key, const char *ValueName, DWORD Default)
{
  DWORD ValueData;
  GetRegKeyEx(Key, ValueName, (LPBYTE)&ValueData, (const LPBYTE)&Default, sizeof(DWORD));
  return ValueData;
}

int TRegistryStorage::GetRegKey(const char *Key, const char *ValueName, int Default)
{
  return (int)GetRegKey(Key, ValueName, (DWORD)Default);
}

bool TRegistryStorage::GetRegKey(const char *Key, const char *ValueName, bool Default)
{
  return !!GetRegKey(Key, ValueName, (DWORD)Default);
}

HKEY TRegistryStorage::CreateRegKey(const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[KN];
  FarSprintf(FullKeyName, REG_KEY_SSS, PluginRegRootKey, *Key ? "\\":"", Key);
  RegCreateKeyEx(HROOT, FullKeyName, 0, NULL, 0, KEY_WRITE, NULL,
  &hKey, &Disposition);
  return hKey;
}

HKEY TRegistryStorage::OpenRegKey(const char *Key)
{
  HKEY hKey;
  char FullKeyName[KN];
  FarSprintf(FullKeyName, REG_KEY_SSS, PluginRegRootKey, *Key ? "\\" : "", Key);
  if (RegOpenKeyEx(HROOT, FullKeyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
    return NULL;
  return hKey;
}

void TRegistryStorage::DeleteRegKey(const char *Key)
{
  char FullKeyName[KN];
  FarSprintf(FullKeyName, REG_KEY_SSS, PluginRegRootKey, *Key ? "\\" : "", Key);
  RegDeleteKey(HROOT, FullKeyName);
}

void TRegistryStorage::DeleteRegValue(const char *Key, const char *Value)
{
  HKEY key = OpenRegKey(Key);
  RegDeleteValue(key, Value);
  RegCloseKey(key);
}

TRegistryStorage* RegistryStorage;
