//tools.h
#ifndef __TOOLS_H
#define __TOOLS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "crtdll.h"
#include "TextNavigate.h"
#include "sgml.h"
#include "CRegExp.h"

void InitUnion();
void AddEngCharsToUnion(void);
void AddDigitsToUnion(void);
void AddSubSetToUnion(UCHAR *set);
int is_char(UCHAR c);
int get_word(char* word, const char* str, int pos, int sln, int& begin_word_pos);

void InitQuickSearch(bool SearchUp, const char* substr, int strlen_word, bool casesensitive);
int QuickSearch_FW(const char* String, const char* substr, int n, int m, int begin_word_pos, bool SearchSelection, bool casesensitive);
int QuickSearch_BW(const char* String, const char* substr, int n, int m, bool SearchSelection, bool casesensitive);

bool GetFile(char* FileName, char*& buffer, UINT& sz);
int file_exists(const char* fname);

void set_cursor_pos(int x, int y, struct EditorInfo *pei);
int get_cursor_pos(int& x, int& String);

int CheckForEsc(void);
PSgmlEl GetChild(PSgmlEl parent, const char* Name);
bool InitParam(char*& field, const char* param);

extern SCharData CharSet;

#ifdef _DEBUG

static char tmp_str[1500];

#endif //_DEBUG

bool SplitRegExpr(const char* RegExpr, const char* InputStr, SMatches& m);

#endif /* __TOOLS_H */
