//reg.h
#ifndef __REG_H
#define __REG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DEFAULT_ADDITIONAL_LETTERS "€-Ÿ -¯à-ñ_"
#define MAX_CHAR_SET_LENGTH 30
#define KN 0x200

#define HROOT HKEY_CURRENT_USER

#define REG_KEY_SSS "%s%s%s"
#define REG_KEY_ACTIVE "Active"
#define REG_KEY_DIGIT_AS_CHAR "ConsiderDigitAsChar"
#define REG_KEY_CASE_SENSITIVE "CaseSensitive"
#define REG_KEY_CYCLIC_SEARCH "CyclicSearch"
#define REG_KEY_SEARCH_SELECTION "SearchSelection"
#define REG_KEY_SAVE_BOOKMARKS "SaveBookmarks"
#define REG_KEY_ADDITIONAL_LETTERS "AdditionalLetters"
#define REG_KEY_BOOKMARKS "Bookmarks"

typedef struct
{
  char s_AdditionalLetters[MAX_CHAR_SET_LENGTH];
  bool b_active;
  bool b_adddigits;
  bool b_casesensitive;
  bool b_cyclicsearch;
  bool b_searchselection;
  bool b_savebookmarks;
} SPluginOptions;

/*******************************************************************************
              class TRegistryStorage
*******************************************************************************/

class TRegistryStorage
{
  private:
    char PluginRegRootKey[KN];

    HKEY CreateRegKey(const char *Key);
    HKEY OpenRegKey(const char *Key);
    void DeleteRegKey(const char *Key);

    void SetRegKey(const char *Key, const char *ValueName, BYTE *ValueData, DWORD ValueSize);
    void SetRegKey(const char *Key, const char *ValueName, DWORD ValueData);
    void SetRegKey(const char *Key, const char *ValueName, bool ValueData);

    int GetRegKeyEx(const char *Key, const char *ValueName, LPBYTE ValueData, const LPBYTE Default, DWORD DataSize);
    DWORD GetRegKey(const char *Key, const char *ValueName, DWORD Default);
    int GetRegKey(const char *Key, const char *ValueName, int Default);
    bool GetRegKey(const char *Key, const char *ValueName, bool Default);

  public:
    TRegistryStorage();
    void SavePluginOptions();

    int GetRegKey(const char *Key, const char *ValueName, char *ValueData, char *Default, DWORD DataSize);
    void SetRegKey(const char *Key, const char *ValueName, const char *ValueData);
    void DeleteRegValue(const char *Key, const char *Value);
};

extern TRegistryStorage* RegistryStorage;
extern SPluginOptions plugin_options;

#endif /* __REG_H */
