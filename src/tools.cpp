//tools.cpp
#include "tools.h"
#include "commons.h"

SCharData CharSet;

void AddEngCharsToUnion(void)
{
  //добавляем английские буквы
  UCHAR ch = 'A';
  while (ch <= 'Z')
    CharSet.SetBit(ch++);
  ch = 'a';
  while (ch <= 'z')
    CharSet.SetBit(ch++);
} //AddEngCharsToUnion

void AddDigitsToUnion(void)
{
  UCHAR ch = '0';
  while (ch <= '9')
    CharSet.SetBit(ch++);
} //AddDigitsToUnion

void AddSubSetToUnion(UCHAR *set)
{
  UCHAR *ch = set;

  while (*ch)
  {
    UCHAR BeginChar = *ch++;
    if (*ch == '-' && ch[1])
    {
      UCHAR EndChar = *++ch;
      if (BeginChar <= EndChar)
        while (BeginChar <= EndChar && BeginChar)
         CharSet.SetBit(BeginChar++);
      ch++;
    }
    else
     CharSet.SetBit(BeginChar);
  }
} //AddSubSetToUnion

void InitUnion()
{
  ZeroMemory(&CharSet, 32);
  AddEngCharsToUnion();
  if (plugin_options.b_adddigits)
    AddDigitsToUnion();
  AddSubSetToUnion((UCHAR *)plugin_options.s_AdditionalLetters);
} //InitUnion

int is_char(UCHAR c)
{
  return CharSet.GetBit(c);
} //is_char

int get_word(char *word, const char *str, int pos, int sln, int &begin_word_pos)
{
  int i, j;
  for (i = pos; i > 0 && is_char(str[i]); i--);
  if (is_char((UCHAR)str[i])) i--;
  begin_word_pos = i+1;

  for (j = pos; j < sln && is_char((UCHAR)str[j]); j++);
  if (is_char((UCHAR)str[j])) j++;
  j -= i + 1;

  if (j <= 0)
    return false;
  //if (j > MAX_WORD_LENGTH - 1)
  if (j > MAX_PATH - 1)
    return false;
  lstrcpynA(word, str+i+1, j + 1);
  return (j != 0);
} //get_word

//static const int ASIZE = 256;
#define ASIZE 256
static UCHAR qs_bc[ASIZE];

void InitQuickSearch(bool SearchUp, const char *substr, int strlen_word, bool casesensitive)
{
  if (SearchUp)
  {
    FillMemory(qs_bc, strlen_word, ASIZE);
    for (int i = 0; i < strlen_word; i++)
      if (qs_bc[(UCHAR)substr[i]] == strlen_word)
        qs_bc[(UCHAR)substr[i]] = i ? (UCHAR)i : (UCHAR)strlen_word;

    if (!casesensitive)
    {
      char buffer[MAX_WORD_LENGTH];
      lstrcpy(buffer, substr);
      FSF.LStrupr(buffer);
      for (int i = 0; i < strlen_word; i++)
        if (qs_bc[(UCHAR)buffer[i]] == strlen_word)
          qs_bc[(UCHAR)buffer[i]] = i ? (UCHAR)i : (UCHAR)strlen_word;
    }
  }
  else //search down
  {
    FillMemory(qs_bc, (UCHAR)(strlen_word + 1), ASIZE);
    for (int i = 0; i < strlen_word; i++)
      qs_bc[(UCHAR)substr[i]] = (UCHAR)(strlen_word - i);

    if (!casesensitive)
    {
      char buffer[MAX_WORD_LENGTH];
      lstrcpy(buffer, substr);
      FSF.LStrupr(buffer);
      for (int i = 0; i < strlen_word; i++)
        qs_bc[(UCHAR)buffer[i]] = (UCHAR)(strlen_word - i);
    }
  }
} //InitQuickSearch

int QuickSearch_FW(const char* String, const char* substr, int n, int m, int begin_word_pos, bool SearchSelection, bool casesensitive)
{
  int i;

  i = begin_word_pos; bool res;
  while (i <= n - m)
  {
    if (casesensitive)
      res = crt::lmemcmp(&String[i], substr, m) != 0;
    else
      res = FSF.LStrnicmp(&String[i], substr, m) != 0;

    if (!res && (SearchSelection ||
                 (!is_char(String[i + m]) &&
                  !((i > 0) && is_char(String[i-1])))
                )
       )
      return (i);

    i += qs_bc[(UCHAR)String[i + m]];
  }
  return -1;
} //QuickSearch_FW

int QuickSearch_BW(const char* String, const char* substr, int n, int m, bool SearchSelection, bool casesensitive)
{
  int i;

  i = n - m; bool res;
  while (i >= 0)
  {
    if (casesensitive)
      res = crt::lmemcmp(&String[i], substr, m) != 0;
    else
      res = FSF.LStrnicmp(&String[i], substr, m) != 0;

    if (!res && (SearchSelection ||
                 (!is_char(String[i + m]) &&
                  !((i > 0) && is_char(String[i-1]))
                 )
                )
       )
      return (i);
    i -= qs_bc[(UCHAR)String[i]];
  }
  return -1;
} //QuickSearch_BW

bool GetFile(char* FileName, char*& buffer, UINT& sz)
{
  buffer = NULL;
  HANDLE s = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (s != INVALID_HANDLE_VALUE)
  {
    sz = GetFileSize(s, NULL);
    AutoPtr<char> _buffer(new char[sz+1]);
    if (sz && _buffer != NULL)
    {
      DWORD rb = 0;
      if (ReadFile(s, _buffer, sz, &rb, NULL) && rb)
      {
        buffer = _buffer.Detach();
        buffer[rb] = 0;
      }
      else
        return false;
    }
  }
  CloseHandle(s);
  return true;
} //GetFile

int file_exists(const char* fname)
{
  return (GetFileAttributes(fname) != (DWORD)(-1));
} //file_exists

int get_cursor_pos(int &x, int &String)
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return false;
  x = EInfo.CurPos;
  String = EInfo.CurLine;
  return true;
} //get_cursor_pos

void set_cursor_pos(int x, int y, struct EditorInfo* pei)
{
  struct EditorSetPosition esp;

  esp.CurLine = pei->CurLine;
  esp.CurPos = pei->CurPos;
  esp.TopScreenLine = pei->TopScreenLine;
  esp.LeftPos = pei->LeftPos;
  esp.CurTabPos = -1;
  esp.Overtype = -1;
  Info.EditorControl(ECTL_SETPOSITION, &esp);

  if (y != -1)
  {
    esp.CurLine = y;
    esp.CurPos = x;

    esp.TopScreenLine = -1;
    esp.LeftPos = -1;
    esp.CurTabPos = -1;

    Info.EditorControl(ECTL_SETPOSITION, &esp);
  }
  Info.EditorControl(ECTL_REDRAW, NULL);
} //set_cursor_pos

int CheckForEsc(void)
{
  INPUT_RECORD rec;
  static HANDLE hConInp = GetStdHandle(STD_INPUT_HANDLE);
  DWORD ReadCount;

  PeekConsoleInput(hConInp, &rec, 1, &ReadCount);
  if (0 == ReadCount) return false;
  ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
  if (rec.EventType == KEY_EVENT)
   if (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
     rec.Event.KeyEvent.bKeyDown) return true;
  return false;
} //CheckForEsc

PSgmlEl GetChild(PSgmlEl parent, const char* Name)
{
  if (!parent || !Name)
   return NULL;
  PSgmlEl child = parent->child();

  if (!child) return NULL;

  if (child->getname() && !lstrcmp(child->getname(), Name))
    return (child);
  else
    return (child->search(Name));
} //GetChild

bool InitParam(char*& field, const char* param)
{
  if (param)
  {
    crt::size_t len = lstrlen(param) + 1;
    field = new char[len];
    lstrcpyA(field, param);
    return true;
  }
  return false;
} //InitParam

bool SplitRegExpr(const char* RegExpr, const char* InputStr, SMatches& m)
{
  CRegExp reg;
  if (!reg.SetExpr((char*)RegExpr))
    return false;

  int cm(0);
  bool exec;
  SMatches lm;
  char *s = (char*)InputStr; const char *e = InputStr + lstrlen(InputStr);
  do
  {
    exec = s < e;
    if (reg.Parse(s, &lm))
    {
      m.s[cm] = (short)(s - InputStr);
      m.e[cm] = m.s[cm] + lm.s[0];
      cm++;
      s += lm.e[0];
    }
    else if (exec)
    {
      m.s[cm] = (short)(s - InputStr);
      m.e[cm] = m.s[cm] + (short)(e - s);
      cm++;
      exec = false;
    }
  } while (exec);
  m.CurMatch = cm;
  return true;
}
