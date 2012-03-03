//pclasses.cpp
#include "pclasses.h"
#include "farkeys.hpp"

const static char s_MethodType[] = "%MethodType%";
const static char s_MethodName[] = "%MethodName%";
const static char s_ClassName[] = "%ClassName%";
const static char s_RegWord[] = "(\\w+)";
const static char s_Definition[] = "Definition";
const static char s_End[] = "End";
const static char s_Implementation[] = "Implementation";
const static char s_Method[] = "Method";
const static char s_GlobalProc[] = "GlobalProc";
const static char s_Type[] = "Type";
const static char s_Pos[] = "Pos";
const static char s_MethodTypePos[] = "MethodTypePos";
const static char s_MethodNamePos[] = "MethodNamePos";

const static char path_root[] = "\\";
const static char s_FileExts[] = "FileExts";
const static char s_ExcludedFileExts[] = "ExcludedFileExts";
const static char s_Include[] = "include";
const static char s_Language[] = "Language";
const static char s_File[] = "file";
const static char s_Name[] = "name";
const static char s_SpecVars[] = "SpecVars";
const static char s_Var[] = "Var";
const static char s_Value[] = "value";
const static char s_Class[] = "Class";
const static char s_SearchPaths[] = "SearchPaths";
const static char s_SourceFiles[] = "SourceFiles";
const static char s_Headers[] = "Headers";
const static char s_version[] = "version";
const static char s_versionID[] = "0.3";
const static char s_TextNavigate[] = "TextNavigate";

/*******************************************************************************
              class CTextNavigate
*******************************************************************************/

CTextNavigate::CTextNavigate()
{
  //инициализация битового массива CharSet
  InitUnion();

  XMLFilesColl = new CXMLFilesColl();
  windows = new CWindowsColl();
  FPrevKeyCode = 0;
  ZeroMemory(FIncrementalSearchBuffer, MAX_INCREMENTAL_SEARCH_LENGTH);
}

CTextNavigate::~CTextNavigate()
{
}

int CTextNavigate::ProcessEditorInput(const INPUT_RECORD* Rec)
{
  if (Rec->EventType != KEY_EVENT || !Rec->Event.KeyEvent.bKeyDown)
    return 0;

  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return 0;

  int res(0);
  bool CtrlPressed  = (Rec->Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)) != 0;
  bool AltPressed   = (Rec->Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED  | LEFT_ALT_PRESSED )) != 0;
  bool ShiftPressed = (Rec->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED                           ) != 0;

  if (CtrlPressed)
  {
    FKeyCode = Rec->Event.KeyEvent.wVirtualKeyCode;
    if (((FKeyCode == VK_UP) || (FKeyCode == VK_DOWN)) && AltPressed)
    { //­ ¦ в  Ctrl-Alt-Up/Down
      res = processCtrlAltUpDown(FKeyCode, FPrevKeyCode);
      FPrevKeyCode = FKeyCode;
      FEditorState = esStateNormal;
      return res;
    } //if (Ctrl-Alt-Up/Down)
    else if (FKeyCode == VK_RETURN)
    {
      DebugString("ProcessEditorInput: Ctrl-Enter Pressed");
      FEditorState = esStateNormal;
      return processCtrlEnter();
    }
    else if (((FKeyCode == VK_UP) || (FKeyCode == VK_DOWN)) && ShiftPressed)
    {
      FEditorState = esStateNormal;
      return processCtrlShiftUpDown(FKeyCode);
    }
    else if (FKeyCode == 'K' && CtrlPressed && !AltPressed && !ShiftPressed)
    {
      DebugString("Ctrl-K pressed");
      switch (FEditorState)
      {
        case esStateNormal:
        case esCtrlIPressed:
          FEditorState = esCtrlKPressed;
          break;
        case esCtrlKPressed:
          //got Ctrl-K Ctrl-K sequence
          //save current editor position
          AddBookmark();
          FEditorState = esStateNormal;
          break;
      }
      DrawTitle();
      return 1;
    }
    else if (FKeyCode == 'N' && CtrlPressed)
    {
      if (FEditorState == esCtrlKPressed)
      {
        //got Ctrl-K Ctrl-N sequence
        //go to next saved position
        MoveToNextBookmark(true);
        FEditorState = esStateNormal;
        DrawTitle();
        return 1;
      }
    }
    else if (FKeyCode == 'P' && CtrlPressed)
    {
      if (FEditorState == esCtrlKPressed)
      {
        //got Ctrl-K Ctrl-P sequence
        //go to previous saved position
        MoveToNextBookmark(false);
        FEditorState = esStateNormal;
        DrawTitle();
        return 1;
      }
    }
    else if (FKeyCode == 'L' && CtrlPressed)
    {
      if (FEditorState == esCtrlKPressed)
      {
        //got Ctrl-K Ctrl-L sequence
        //Clear bookmarks
        ClearBookmarks(EInfo.EditorID, true);
        FEditorState = esStateNormal;
        DrawTitle();
        return 1;
      }
    }
    else if (FKeyCode == 'U' && CtrlPressed)
    {
      if (FEditorState == esCtrlKPressed)
      {
        //got Ctrl-K Ctrl-U sequence
        Push();
        FEditorState = esStateNormal;
        DrawTitle();
        return 1;
      }
    }
    else if (FKeyCode == 'O' && CtrlPressed)
    {
      if (FEditorState == esCtrlKPressed)
      {
        //got Ctrl-K Ctrl-O sequence
        Pop();
        FEditorState = esStateNormal;
        DrawTitle();
        return 1;
      }
    }
    else if (Rec->Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED &&
             !AltPressed && !ShiftPressed &&
             FKeyCode >= '0' && FKeyCode <= '9'
            )
    {
#ifdef _DEBUG
      FarSprintf(tmp_str, "RCtrl+%c Pressed", FKeyCode);
      DebugString(tmp_str);
#endif
      AddBookmark();
    }
    else if (Rec->Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED &&
             !AltPressed && !ShiftPressed &&
             FKeyCode >= '0' && FKeyCode <= '9'
            )
    {
#ifdef _DEBUG
      FarSprintf(tmp_str, "LCtrl+%c Pressed", FKeyCode);
      DebugString(tmp_str);
#endif // _DEBUG
    }
    else if (FKeyCode == 'I' && CtrlPressed && !AltPressed && !ShiftPressed)
    {
      DebugString("Ctrl-I pressed");
      if (FEditorState == esStateNormal)
      {
        StartIncrementalSearch();
        return 1;
      }
    }
    FEditorState = esStateNormal;
    DrawTitle();
    FPrevKeyCode = 0;
  }
  else if (Rec->Event.KeyEvent.bKeyDown)
  {
    if (FEditorState == esCtrlIPressed)
    {
      DWORD KeyCode = Rec->Event.KeyEvent.wVirtualKeyCode | 0x100;
      bool ArrowKeysPressed = KeyCode == KEY_RIGHT ||
                              KeyCode == KEY_DOWN ||
                              KeyCode == KEY_UP ||
                              KeyCode == KEY_LEFT;

      unsigned char c = Rec->Event.KeyEvent.uChar.AsciiChar;
      if (!CtrlPressed && (FSF.LIsAlphanum(c) || (c == KEY_SPACE) || (c == '_') || 
                           (KEY_BS == c) ||
                           ArrowKeysPressed))
      {
        if (lstrlen(FIncrementalSearchBuffer) < MAX_INCREMENTAL_SEARCH_LENGTH)
        {
          if (!ArrowKeysPressed)
          {
            if (KEY_BS == c)
            {
              if (FIncrementalSearchBufferEnd > FIncrementalSearchBuffer)
                *--FIncrementalSearchBufferEnd = '\0';
            }
            else
            {
              *FIncrementalSearchBufferEnd = c;
              FIncrementalSearchBufferEnd++;
            }
          }
          DrawTitle();

          struct EditorInfo EInfo;
          if (Info.EditorControl(ECTL_GETINFO, &EInfo))
          {
            int CurLine = EInfo.CurLine;

            struct EditorGetString EStr;
            EStr.StringNumber = CurLine;
            if (!Info.EditorControl(ECTL_GETSTRING, &EStr))
              return 0;

            FPrevKeyCode = 0;
            bool Upward = ArrowKeysPressed ? KeyCode == KEY_UP || KeyCode == KEY_LEFT : false;
            int CurPos = EInfo.CurPos && !Upward ? EInfo.CurPos - 1 : EInfo.CurPos - 1;
            int pos_found;
            if ((pos_found = do_search(EStr.StringText, FIncrementalSearchBuffer,
                                       Upward, CurPos, CurLine, true, false)) != -1)
            {
              //обнулим код клавиши, для того чтобы запретить плагину ECompl
              //проводить свою обработку
              ((INPUT_RECORD*)Rec)->Event.KeyEvent.bKeyDown = 0;
              ((INPUT_RECORD*)Rec)->Event.KeyEvent.wVirtualKeyCode = 0;
              ((INPUT_RECORD*)Rec)->Event.KeyEvent.wVirtualScanCode = 0;
              //((INPUT_RECORD*)Rec)->Event.KeyEvent.uChar.AsciiChar = '\0';
              //((INPUT_RECORD*)Rec)->Event.KeyEvent.wRepeatCount = 0;
              int Len = lstrlen(FIncrementalSearchBuffer);
              SelectFound(CurLine, pos_found, Len);
              set_cursor_pos(pos_found + Len, CurLine, &EInfo);
            }
            return 1;
          }
        }
      }
      else if (c)
      {
        Info.EditorControl(ECTL_SETTITLE, NULL);
        Info.EditorControl(ECTL_REDRAW, NULL);
        if (FEditorState == esCtrlIPressed)
          res = 1;
        FEditorState = esStateNormal;
      }
    }
    else if (FEditorState == esCtrlKPressed)
    {
      FEditorState = esStateNormal;
      DrawTitle();
    }
  }
  return res;
} //ProcessEditorInput

int CTextNavigate::ProcessEditorEvent(int Event, void* Param)
{
  struct EditorInfo EInfo;
  int id;
  switch (Event)
  {
    case EE_READ:
      if (Info.EditorControl(ECTL_GETINFO, &EInfo))
      {
        id = EInfo.EditorID;
        XMLFile = new CXMLFile();
        XMLFilesColl->Add(id, XMLFile);

        char* FilePart;
        char Buffer[MAX_PATH];
        if(!crt::lstrchr(EInfo.FileName, ':'))
        {
          //DWORD retlen;
          /*retlen = */GetFullPathName(EInfo.FileName, MAX_PATH, Buffer, &FilePart);
        }
        else
          lstrcpy(Buffer, EInfo.FileName);

        TWindowData* window = new TWindowData(id, Buffer);
        window = windows->Add(id, window);

        //read saved bookmarks info
        if (plugin_options.b_active)
          LoadBookmarks(id);
      }
      break;

    case EE_CLOSE:
      id = *static_cast<int *>(Param);
      XMLFilesColl->Delete(id);
      if (plugin_options.b_active && plugin_options.b_savebookmarks)
        SaveBookmarks(id);
      ClearBookmarks(id, false);
      windows->Delete(id);
      break;

 } //switch
 return 0;
} //ProcessEditorEvent

int CTextNavigate::processCtrlAltUpDown(int FKeyCode, int FPrevKeyCode)
{
#ifdef _DEBUG
  if (FKeyCode == VK_UP)
  {
    DebugString("Ctrl-Alt-Up pressed");
  }
  else
  {
    DebugString("Ctrl-Alt-Down pressed");
  }
#endif
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return 0;

  XMLFile = XMLFilesColl->Get(EInfo.EditorID);
  if (!XMLFile)
    return 0;

  InitUnion();
  //int BeginLine = (EInfo.BlockType == BTYPE_STREAM) ? EInfo.BlockStartLine:EInfo.CurLine;
  int BeginLine = EInfo.CurLine, CurLine = BeginLine;

  int mx, my;
  if (!get_cursor_pos(mx, my))
    return false;

  struct EditorGetString EStr;
  EStr.StringNumber = CurLine;
  if (!Info.EditorControl(ECTL_GETSTRING, &EStr))
    return 0;

  char *String = (char *)EStr.StringText;

  bool SearchSelection = plugin_options.b_searchselection && (EInfo.BlockType == BTYPE_STREAM);
  //если курсор стоит сразу после слова, ищем его
  if (!SearchSelection && (mx <= EStr.StringLength))
  {
    //if (mx && ((String[mx] == ' ') || (mx == EStr.StringLength)) && is_char(String[mx-1]))
    if (mx &&
        (!is_char(String[mx]) || (mx == EStr.StringLength)) &&
        is_char(String[mx-1]))
     mx--;
  }
  int begin_word_pos = mx;

  if (!FPrevKeyCode) //раньше было нажато не Ctrl-Alt-Up/Down
  {
    //если есть выделение
    if (SearchSelection)
    {
      FillMemory(&CharSet, 0xFF, 32); //ищем любые символы
      struct EditorGetString egs;
      egs.StringNumber = EInfo.BlockStartLine;
      if (!Info.EditorControl(ECTL_GETSTRING, &egs))
       return false;
      String = (char *)egs.StringText;
      int SelLength = egs.SelEnd == -1 ? egs.StringLength : egs.SelEnd - egs.SelStart;
      lstrcpynA(word, &String[egs.SelStart], SelLength + 1);
    }
    else //выделим слово под курсором в массив word
      if (begin_word_pos > EStr.StringLength || !get_word(word, String, begin_word_pos, EStr.StringLength, begin_word_pos))
        *word = 0;
  }

  if (!*word)
    return 0;
  if (!plugin_options.b_casesensitive)
    FSF.LStrlwr(word);

  int pos_found;
  if ((pos_found = do_search(String, word, FKeyCode == VK_UP, begin_word_pos, CurLine, SearchSelection, plugin_options.b_casesensitive)) != -1)
  {
    set_cursor_pos(pos_found, CurLine, &EInfo);
  }
  //Colorize(strlen_word);
  return (pos_found != -1 ? 1 : 0);
} //processCtrlAltUpDown

int CTextNavigate::do_search(const char* String, const char* substr, bool SearchUp, int begin_word_pos, int &CurLine, bool SearchSelection, bool casesensitive)
{
  if (!substr) return -1;
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return -1;

  int StringLength = lstrlen(String);
  int strlen_word = lstrlen(substr);
  int search_pos = SearchUp ? begin_word_pos : begin_word_pos+1;
  bool borders_crossed = false; //признак зацикливания поиска
  int BeginLine = CurLine;
  struct EditorGetString EStr;
  EStr.StringNumber = -1; //получаем текущую строку
  struct EditorSetPosition esp;
  esp.CurPos = esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = esp.Overtype = -1;

  if (!FPrevKeyCode || FKeyCode != FPrevKeyCode) //раньше было нажато не Ctrl-Alt-Up/Down
    InitQuickSearch(SearchUp, substr, strlen_word, casesensitive);

  int pos_found(-1);
  for (; ; esp.CurLine = CurLine)
  {
    if (CurLine != BeginLine || borders_crossed)
    {
      Info.EditorControl(ECTL_SETPOSITION, &esp);
      Info.EditorControl(ECTL_GETSTRING, &EStr);
      StringLength = EStr.StringLength;
      String = EStr.StringText;
      search_pos = SearchUp ? EStr.StringLength : 0;
    }

    if ((pos_found = SearchUp ? QuickSearch_BW(String, substr, search_pos, strlen_word, SearchSelection, casesensitive != 0) :
                                QuickSearch_FW(String, substr, StringLength, strlen_word, search_pos, SearchSelection, casesensitive != 0)) != -1)
      break;

    if (BeginLine == CurLine && borders_crossed)
      break;
    SearchUp ? CurLine-- : CurLine++;

    if ((CurLine == -1) && SearchUp) //переход на последнюю строку при достижении первой
      if (plugin_options.b_cyclicsearch)
      {
        CurLine = EInfo.TotalLines - 1;
        borders_crossed = true;
      } else break;
    else if ((CurLine == EInfo.TotalLines) && !SearchUp) //переход на первую строку
      if (plugin_options.b_cyclicsearch)
      {
        CurLine = 0;
        borders_crossed = true;
      } else break;
    if (CheckForEsc())
      break;
  }
  return pos_found;
}

int CTextNavigate::processCtrlEnter(void)
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
   return 0;

  XMLFile = XMLFilesColl->Get(EInfo.EditorID);
  if (!XMLFile || !XMLFile->SearchPaths)
    return 0;
  //PSearchPaths SearchPaths = XMLFile->SearchPaths;
  return (XMLFile->SearchPaths->ProcessCtrlEnter(XMLFile->Language));
} //processCtrlEnter

int CTextNavigate::processCtrlShiftUpDown(int FKeyCode)
{
#ifdef _DEBUG
  if (FKeyCode == VK_UP)
    DebugString("Ctrl-Shift-Up pressed");
  else
    DebugString("Ctrl-Shift-Down pressed");
#endif
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return 0;

  XMLFile = XMLFilesColl->Get(EInfo.EditorID);
  if (!XMLFile)
    return 0;

  int LineFound = EInfo.CurLine, x_pos = 0;

  if (FKeyCode == VK_UP) //ищем определение метода
  {
    FindMethodDefinition(LineFound, x_pos);
  }
  else //ищем реализацию метода
  {
    FindMethodImplementation(LineFound, x_pos);
  }

  set_cursor_pos(x_pos, LineFound, &EInfo);
  return (LineFound != -1 ? 1 : 0);
} //processCtrlShiftUpDown

int CTextNavigate::strreplace(char *str, const char *pattern, const char *value)
{
  if (!str || !pattern || !value)
    return -1;

  char *s = crt::lstrstr(str, pattern);
  if (!s)
    return -1;

  int n_roundbrackets = 0;
  char *ch = str;
  while (ch != s)
    if (*ch++ == '(') n_roundbrackets++;
  if (*value == '(')
    n_roundbrackets++;
  else if (*pattern == '(')
    n_roundbrackets++;

  int pattern_len = lstrlen(pattern); int value_len = lstrlen(value);

  if (pattern_len > value_len)
  {
    lstrcpyn(s, value, (int)value_len);
    lstrcpy(s + value_len, s + pattern_len);
  }
  else if (pattern_len < value_len)
  {
    int s_len = lstrlen(s);
    crt::lmemmove(s + value_len, s + pattern_len, s_len - pattern_len + 1);
    lstrcpyn(s, value, (int)value_len);
    s[s_len + value_len - pattern_len+1] = 0;
  }
  else
   lstrcpyn(s, value, (int)value_len);

  return n_roundbrackets;
} //strreplace

void CTextNavigate::ReplaceSpecRegSymbols(char *str)
{
  const static char specSymbols[] = "()[]{}|";

  for (int i = 0; i < lstrlen(specSymbols); i++)
  {
    char sS = specSymbols[i];
    for (int j = lstrlen(str); j >= 0; j--)
    {
      char *s = &str[j];
      if (sS != *s)
       continue;
      int s_len = lstrlen(s);
      crt::lmemmove(s + 1, s, s_len);
      *s = '\\';
      s[s_len+1] = 0;
    }
  }
} //ReplaceSpecRegSymbols

int CTextNavigate::SearchForMethodImplementation(int &CurLine, int GlobalProc)
{
  DebugString(" ========== SearchForMethodImplementation");

  SLanguage* Language = XMLFile->Language;

  if (!Language)
    return false;

  if (GlobalProc)
    Method = Language->Method;
  else
  {
    Class = Language->Class;
    if (!Class) return false;
    Method = Class->Method;
  }

  if (!Method)
    return false;

  if (!Method->Name || !Method->Implementation)
   return false;

  char MethodImplementation[200];
  ZeroMemory(MethodImplementation, 200);
  lstrcpyA(MethodImplementation, Method->Implementation);
  int MethodTypePos = strreplace(MethodImplementation, s_MethodType, Method->Type);

  int ClassNamePos = -1;
  if (!GlobalProc)
    ClassNamePos = strreplace(MethodImplementation, s_ClassName,  Class->Name);
  MethodNamePos = strreplace(MethodImplementation, s_MethodName, Method->Name);

  SMatches m; char *StringText;
  if (!SearchBackward(MethodImplementation, CurLine, StringText, m))
    return false;

  GetMatch(MethodType, m, StringText, MethodTypePos);
  if (!GlobalProc)
    GetMatch(ClassName, m, StringText, ClassNamePos);
  GetMatch(MethodName, m, StringText, MethodNamePos);
#ifdef _DEBUG
  FarSprintf(tmp_str, "MethodName = %s", MethodName);
  DebugString(tmp_str);
#endif


  if (!GlobalProc)
  {
    lstrcpyA(ClassDefinition, Class->Definition);
    strreplace(ClassDefinition, s_ClassName, ClassName);
  }
  return true;
} //SearchForMethodImplementation

bool CTextNavigate::SearchForMethodDefinition2(int &CurLine, bool GlobalProc)
{
  DebugString(" ========== SearchForMethodDefinition2");

  SLanguage* Language = XMLFile->Language;

  if (!Language)
    return false;

  if (GlobalProc)
    Method = Language->Method;
  else
  {
    Class = Language->Class;
    if (!Class) return false;
    Method = Class->Method;
  }

  if (!Method)
    return false;

  lstrcpyA(MethodDefinition, Method->Definition);

  int MethodTypePos = strreplace(MethodDefinition, s_MethodType, Method->Type);
  MethodNamePos = strreplace(MethodDefinition, s_MethodName, Method->Name);


  if (!MethodDefinition)
    return false;

  SMatches m; char *StringText;
  if (!SearchBackward(MethodDefinition, CurLine, StringText, m))
    return false;

  GetMatch(MethodType, m, StringText, MethodTypePos);
  GetMatch(MethodName, m, StringText, MethodNamePos);

  if (!GlobalProc)
  {
    //ищем имя класса
    lstrcpyA(ClassDefinition, Class->Definition);
    int ClassNamePos = strreplace(ClassDefinition, s_ClassName, Class->Name);

    if (!SearchBackward(ClassDefinition, CurLine, StringText, m))
      return false;

    GetMatch(ClassName, m, StringText, ClassNamePos);

    if (!*ClassName || !Method->Implementation)
     return false;
  }

  ZeroMemory(MethodImplementation, 200);
  lstrcpyA(MethodImplementation, Method->Implementation);
  //ищем строку MethodType ClassName.MethodName
  ReplaceSpecRegSymbols(MethodType);

  strreplace(MethodImplementation, s_MethodType, MethodType);
  if (!GlobalProc)
  {
    ReplaceSpecRegSymbols(ClassName);
    strreplace(MethodImplementation, s_ClassName,  ClassName);
  }
  ReplaceSpecRegSymbols(MethodName);
  MethodNamePos = strreplace(MethodImplementation, s_MethodName, MethodName);
  return true;
} //SearchForMethodDefinition2

bool CTextNavigate::SearchForMethodImplementation2(int &CurLine, int &x_pos)
{
  DebugString(" ========= SearchForMethodImplementation2");

  SMatches m; char *StringText;
  if (SearchForward(MethodImplementation, CurLine, StringText, m))
  {
    x_pos = m.s[MethodNamePos];
    return true;
  }
  return false;
} //SearchForMethodImplementation2

bool CTextNavigate::SearchBackward(char *RegExpr, int &CurLine, char *&StringText, SMatches &m)
{
  if (!reg.SetExpr(RegExpr))
    return false;

  struct EditorGetString egs;
  egs.StringNumber = -1;

  struct EditorSetPosition esp;
  esp.CurPos = esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = esp.Overtype = -1;
  esp.CurLine = CurLine;

  while (esp.CurLine >= 0)
  {
    Info.EditorControl(ECTL_SETPOSITION, &esp);
    Info.EditorControl(ECTL_GETSTRING, &egs);

    if (reg.Parse((char *)egs.StringText, &m))
    {
      CurLine = esp.CurLine;
      StringText = (char*)egs.StringText;
      break;
    }
    esp.CurLine--;
    if (CheckForEsc()) return false;
  }
  return (esp.CurLine != -1);
}

bool CTextNavigate::SearchForward(char *RegExpr, int &CurLine, char *&StringText, SMatches &m)
{
  if (!reg.SetExpr(RegExpr))
    return false;

  struct EditorGetString egs;
  egs.StringNumber = -1;

  struct EditorSetPosition esp;
  esp.CurPos = esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = esp.Overtype = -1;

  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return false;

  for (esp.CurLine = CurLine; esp.CurLine < EInfo.TotalLines; esp.CurLine++)
  {
    Info.EditorControl(ECTL_SETPOSITION, &esp);
    Info.EditorControl(ECTL_GETSTRING, &egs);

    if (reg.Parse((char *)egs.StringText, &m))
    {
      CurLine = esp.CurLine;
      StringText = (char*)egs.StringText;
      return true;
    }
    if (CheckForEsc()) return false;
  }
  return false;
} //SearchForward

bool CTextNavigate::IsInClassDefinition(int CurLine)
{
  SLanguage* Language = XMLFile->Language;

  if (!Language)
    return false;

  Class = Language->Class;
  if (!Class) return false;

  int Beg = CurLine;
  //ищем имя класса
  lstrcpyA(ClassDefinition, Class->Definition);
  int ClassNamePos = strreplace(ClassDefinition, s_ClassName, Class->Name);

  SMatches m; char *StringText;
  if (!SearchBackward(ClassDefinition, CurLine, StringText, m))
    return false;

  int ClassBegin = CurLine;
  GetMatch(ClassName, m, StringText, ClassNamePos);

  if (!ClassName)
    return false;

  if (!SearchForward(Class->End, CurLine, StringText, m))
    return false;

  return (CurLine >= Beg && ClassBegin <= Beg);
} //IsInClassDefinition

bool CTextNavigate::FindMethodImplementation(int &CurLine, int &x_pos)
{
  DebugString(" ========== FindMethodImplementation");
  int Beg = CurLine + 1;
  GlobalProc = !IsInClassDefinition(CurLine);
  if (!SearchForMethodDefinition2(CurLine, GlobalProc))
    return false;
  CurLine = Beg;
  return (SearchForMethodImplementation2(CurLine, x_pos));
} //FindMethodImplementation

bool CTextNavigate::SearchForMethodDefinition(int &CurLine, int &x_pos, bool GlobalProc)
{
#ifdef _DEBUG
  FarSprintf(tmp_str, "ClassDefinition = %s", ClassDefinition);
  DebugString(tmp_str);
#endif
  SMatches m; char *StringText;

  if (!GlobalProc)
    if (!SearchBackward(ClassDefinition, CurLine, StringText, m))
      return false;

  lstrcpyA(MethodDefinition, Method->Definition);
  ReplaceSpecRegSymbols(MethodType);
  strreplace(MethodDefinition, s_MethodType, MethodType);

  ReplaceSpecRegSymbols(MethodName);
  int Pos = strreplace(MethodDefinition, s_MethodName, MethodName);

  int Res;
  GlobalProc ? CurLine-- : CurLine++;
  if (GlobalProc)
    Res = SearchBackward(MethodDefinition, CurLine, StringText, m);
  else
    Res = SearchForward(MethodDefinition, CurLine, StringText, m);
  if (Res)
  {
    x_pos = m.s[Pos];
    return true;
  }
  return false;
} //SearchForMethodDefinition

bool CTextNavigate::IsGlobalMethodImplementation(int CurLine)
{
  int GlobalMethodImplementationPos = -1, ClassMethodImplementationPos = -1;
  int Beg = CurLine;

  if (SearchForMethodImplementation(Beg, false))
    ClassMethodImplementationPos = Beg;

  if (SearchForMethodImplementation(CurLine, true))
    GlobalMethodImplementationPos = CurLine;

  if (GlobalMethodImplementationPos == ClassMethodImplementationPos)
    return false;

  return (GlobalMethodImplementationPos > ClassMethodImplementationPos);
} //IsGlobalMethodImplementation

bool CTextNavigate::FindMethodDefinition(int &CurLine, int &x_pos)
{
  DebugString(" ========== FindMethodDefinition");
  GlobalProc = IsGlobalMethodImplementation(CurLine);
  if (!SearchForMethodImplementation(CurLine, GlobalProc))
    return false;

  return (SearchForMethodDefinition(CurLine, x_pos, GlobalProc));
} //FindMethodDefinition

int CTextNavigate::ShowPluginMenu()
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return 0;

  bool IsSimpleMenu = !XMLFile->Language;
  static const int MItemsSimple[] = {
    MFindNext, MFindPrev, MOpenFile, 
    MAddBookmark, MMoveToTheNextBookmark, MMoveToThePrevBookmark, MCleanAllBookmarks, 
    MIncrementalSearch, 
    MPush, MPop,
    MConfig, MConfig
  };
  static const int n_ItemsSimple = sizeof(MItemsSimple) / sizeof(*MItemsSimple);

  static const int MItemsLang[] = {
    MFindNext, MFindPrev, MOpenFile, 
    MFindMethod, MFindPrototype,
    MAddBookmark, MMoveToTheNextBookmark, MMoveToThePrevBookmark, MCleanAllBookmarks, 
    MIncrementalSearch,
    MPush, MPop,
    MConfig, MConfig };
  static const int n_ItemsLang = sizeof(MItemsLang) / sizeof(*MItemsLang);

  const int *Msgs = IsSimpleMenu ? MItemsSimple : MItemsLang;
  const int n_Items = IsSimpleMenu ? n_ItemsSimple : n_ItemsLang;

  AutoPtr<FFarMenuItem, VectorPtr> fmi (new FFarMenuItem[n_Items]);
  for (int i = 0; i < n_Items; i++)
  {
    if (i == n_Items - 2)
      fmi[i].Separator = true;
    else
      fmi[i].Text = (char *)get_msg(Msgs[i]);
  }
  AutoPtr<CPluginMenu> PluginMenu (new CPluginMenu(n_Items, fmi));

  int selected;

  if (!XMLFile->Language)
  {
    do
    {
      selected = PluginMenu->Execute(get_msg(STitle), NULL);
      switch (selected)
      {
        case 0: //MFindNext
          TextNavigate->processCtrlAltUpDown(VK_DOWN, 0);
          break;
        case 1: //MFindPrev
          TextNavigate->processCtrlAltUpDown(VK_UP, 0);
          break;
        case 2: //MOpenFile
          TextNavigate->processCtrlEnter();
          break;
        case 3: //MAddBookmark
          TextNavigate->AddBookmark();
          break;
        case 4: //MMoveToTheNextBookmark
          TextNavigate->MoveToNextBookmark(true);
          break;
        case 5: //MMoveToThePrevBookmark
          TextNavigate->MoveToNextBookmark(false);
          break;
        case 6: //MCleanAllBookmarks
          TextNavigate->ClearBookmarks(EInfo.EditorID, true);
          break;
        case 7: //MIncrementalSearch
          TextNavigate->StartIncrementalSearch();
          break;
        case 8: //MPush
          TextNavigate->Push();
          break;
        case 9: //MPop
          TextNavigate->Pop();
          break;
        case 11: //MConfig
          config_plugin();
          break;
      }
    } while (selected == 11);

  }
  else
  {
    do
    {
      selected = PluginMenu->Execute(get_msg(STitle), NULL);

      switch (selected)
      {
        case 0: //MFindNext
          TextNavigate->processCtrlAltUpDown(VK_DOWN, 0);
          break;
        case 1: //MFindPrev
          TextNavigate->processCtrlAltUpDown(VK_UP, 0);
          break;
        case 2: //MOpenFile
          TextNavigate->processCtrlEnter();
          break;
        case 3: //MFindPrototype
          TextNavigate->processCtrlShiftUpDown(VK_UP);
          break;
        case 4: //MFindMethod
          TextNavigate->processCtrlShiftUpDown(VK_DOWN);
          break;
        case 5: //MAddBookmark
          TextNavigate->AddBookmark();
          break;
        case 6: //MMoveToTheNextBookmark
          TextNavigate->MoveToNextBookmark(true);
          break;
        case 7: //MMoveToThePrevBookmark
          TextNavigate->MoveToNextBookmark(false);
          break;
        case 8: //MCleanAllBookmarks
          TextNavigate->ClearBookmarks(EInfo.EditorID, true);
          break;
        case 9: //MIncrementalSearch
          TextNavigate->StartIncrementalSearch();
          break;
        case 10: //MPush
          TextNavigate->Push();
          break;
        case 11: //MPop
          TextNavigate->Pop();
          break;
        case 13: //MConfig
          config_plugin();
          break;
      }
    } while (selected == 13);

  }
  return true;
} //ShowPluginMenu

void CTextNavigate::config_plugin()
{
  int height = 67, weight = 14;

  FDialogItem itm[] =
  {
   //FDI_CONTROL(DI_DOUBLEBOX, 3, 1, 63, 11, 0, get_msg(SCfgTitle)),
   FDI_DOUBLEBOX(height, weight, get_msg(SCfgTitle)),

   FDI_CHECK(4, 2, get_msg(SActive)),
   FDI_CHECK(4, 3, get_msg(SAddDigits)),
   FDI_CHECK(4, 4, get_msg(SCaseSensitive)),
   FDI_CHECK(4, 5, get_msg(SCyclicSearch)),
   FDI_CHECK(4, 6, get_msg(SSearchSelection)),
   FDI_CHECK(4, 7, get_msg(SSaveBookmarks)),
   FDI_LABEL(4, 8, get_msg(SAddSymbols)),
   FDI_EDIT(4, 9, 62),
   FDI_SEPARATOR(10),

   FDI_DEFBUTTON(12, 11, get_msg(SSave)),
   FDI_BUTTON(42, 11, get_msg(SCancel))
  };

  static const int n_itm = sizeof(itm)/sizeof(*itm);
  AutoPtr<CConfigDialog> dlg (new CConfigDialog(n_itm, itm));

  dlg->Item(1)->Selected = plugin_options.b_active;
  dlg->Item(2)->Selected = plugin_options.b_adddigits;
  dlg->Item(3)->Selected = plugin_options.b_casesensitive;
  dlg->Item(4)->Selected = plugin_options.b_cyclicsearch;
  dlg->Item(5)->Selected = plugin_options.b_searchselection;
  dlg->Item(6)->Selected = plugin_options.b_savebookmarks;
  lstrcpyA(dlg->Item(8)->Data, plugin_options.s_AdditionalLetters);
  dlg->Item(8)->Focus = true;
  //...
  dlg->Item(n_itm-2)->Flags = DIF_CENTERGROUP;
  dlg->Item(n_itm-1)->Flags = DIF_CENTERGROUP;

  int d_code = dlg->Execute(height, weight);
  if (d_code == n_itm - 2)
  {
    plugin_options.b_active = !!dlg->Item(1)->Selected;
    plugin_options.b_adddigits = !!dlg->Item(2)->Selected;
    plugin_options.b_casesensitive = !!dlg->Item(3)->Selected;
    plugin_options.b_cyclicsearch = !!dlg->Item(4)->Selected;
    plugin_options.b_searchselection = !!dlg->Item(5)->Selected;
    plugin_options.b_savebookmarks = !!dlg->Item(6)->Selected;
    lstrcpyA(plugin_options.s_AdditionalLetters, dlg->Item(8)->Data);
    RegistryStorage->SavePluginOptions();
  }
  InitUnion();
} //config_plugin

int CTextNavigate::GetMatch(char *Match, const SMatches &m, const char *str, int n)
{
  if (n == -1) return (-1);
  int Len = m.e[n] - m.s[n];
  if (Len)
    lstrcpynA(Match, str + m.s[n], Len + 1);
  return Len;
} //GetMatch

void CTextNavigate::AddBookmark()
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return;
  TWindowData *window = windows->Get(EInfo.EditorID);
  if (window)
    window->AddBookmark(EInfo.CurLine, EInfo.CurPos, EInfo.TopScreenLine, EInfo.LeftPos);
} //AddBookmark

void CTextNavigate::MoveToNextBookmark(bool Next)
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return;

  TWindowData *window = windows->Get(EInfo.EditorID);
  if (window)
    window->MoveToNextBookmark(Next);
} //MoveToNextBookmark

void CTextNavigate::SaveBookmarks(int id)
{
  TWindowData *window = windows->Get(id);
  if (window)
    window->SaveBookmarks();
} //SaveBookmarks

void CTextNavigate::LoadBookmarks(int id)
{
  TWindowData *window = windows->Get(id);
  if (window)
    window->LoadBookmarks();
} //LoadBookmarks

void CTextNavigate::ClearBookmarks(int id, bool RegistryAlso)
{
  TWindowData *window = windows->Get(id);
  if (window)
    window->clear(RegistryAlso);
} //ClearBookmarks

void CTextNavigate::StartIncrementalSearch()
{
  ZeroMemory(FIncrementalSearchBuffer, MAX_INCREMENTAL_SEARCH_LENGTH);
  FIncrementalSearchBufferEnd = FIncrementalSearchBuffer;
  FEditorState = esCtrlIPressed;
  DrawTitle();
  //Info.EditorControl(ECTL_REDRAW, NULL);
} //StartIncrementalSearch

void CTextNavigate::Push()
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return;
  TWindowData *window = windows->Get(EInfo.EditorID);
  if (window)
    window->PushBookmark(EInfo.CurLine, EInfo.CurPos, EInfo.TopScreenLine, EInfo.LeftPos);
} //Push

void CTextNavigate::Pop()
{
  struct EditorInfo EInfo;
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return;
  TWindowData *window = windows->Get(EInfo.EditorID);
  if (window)
    window->PopBookmark();
} //Pop

void CTextNavigate::DrawTitle()
{
  if (FEditorState == esCtrlKPressed)
  {
    string Title((const UCHAR*)get_msg(SCtrlKPressed));
    Info.EditorControl(ECTL_SETTITLE, (char*)Title.get());
  }
  else if (FEditorState == esCtrlIPressed)
  {
    string Title((const UCHAR*)get_msg(SIncrementalSearchTitle));
    Title += FIncrementalSearchBuffer;
    Info.EditorControl(ECTL_SETTITLE, (char*)Title.get());
  }
  else
  {
    Info.EditorControl(ECTL_SETTITLE, NULL);
    Info.EditorControl(ECTL_REDRAW, NULL);
  }
}

void CTextNavigate::SelectFound(int StringNumber, int StartPos, int Len)
{
  EditorSelect es;
  es.BlockType = BTYPE_STREAM;
  es.BlockStartLine = StringNumber;
  es.BlockStartPos = StartPos;
  es.BlockWidth = Len;
  es.BlockHeight = 1;
  Info.EditorControl(ECTL_SELECT, (void*)&es);
  Info.EditorControl(ECTL_REDRAW, 0);
}

/*******************************************************************************
              class CPositionsRefColl
*******************************************************************************/

CRefColl<EditorInfo>* CPositionsRefColl::GetNextTo(int row, int pos)
{
  CRefColl<EditorInfo>* fl = prev; //begin search from the end

  while (!fl->main() && fl->id() > row)
    fl = fl->prev;

  if (fl->id() == row)
  {
    while (!fl->main() && fl->id() == row && fl->Ref->CurPos > pos)
      fl = fl->prev;
  }
  return fl->next;
} //GetNextTo

CRefColl<EditorInfo>* CPositionsRefColl::GetPrevTo(int row, int pos)
{
  CRefColl<EditorInfo>* fl = next; //begin search from the beginning
  while (!fl->main() && fl->id() < row)
    fl = fl->next;

  if (fl->id() == row)
  {
    while (!fl->main() && fl->id() == row && fl->Ref->CurPos < pos)
      fl = fl->next;
  }
  return fl->prev;
} //GetPrevTo

char* CPositionsRefColl::get_str_repr(const EditorInfo* pEditorInfo)
{
  static char buffer[40];
  FarSprintf(buffer, "(%d:%d:%d:%d)", pEditorInfo->CurLine, pEditorInfo->CurPos, pEditorInfo->TopScreenLine, pEditorInfo->LeftPos);
  return buffer;
} //get_str_repr

int CPositionsRefColl::compare_refs(const EditorInfo* ref1, const EditorInfo* ref2)
{
  if (!ref1 || !ref2) return 0;
  if (ref1->CurLine < ref2->CurLine)
    return -1;
  if (ref1->CurLine > ref2->CurLine)
    return 1;
  if (ref1->CurPos < ref2->CurPos)
    return -1;
  if (ref1->CurPos > ref2->CurPos)
    return 1;
  return 0;
} //compare_refs

void CPositionsRefColl::replaceby(EditorInfo* ref_to_replace, const EditorInfo* ref)
{
  ref_to_replace->CurLine = ref->CurLine;
  ref_to_replace->CurPos = ref->CurPos;
  ref_to_replace->TopScreenLine = ref->TopScreenLine;
  ref_to_replace->LeftPos = ref->LeftPos;
}

char* CPositionsRefColl::GetStringRepresentation()
{
  int str_len(0);
  CRefColl<EditorInfo>* fl = next;
  while (!fl->main())
  {
    str_len += lstrlen(get_str_repr(fl->Ref)) + 1;
    fl = fl->next;
  }
  if (str_len > MAX_BOOKMARKS_STR_LENGTH)
    str_len = MAX_BOOKMARKS_STR_LENGTH;
#ifdef _DEBUG
  FarSprintf(tmp_str, "str_len = %d", str_len);
  DebugString(tmp_str);
#endif
  if (!str_len)
    return NULL;
  fl = next;
  char *string_representation = new char[str_len + 1];
  while (!fl->main())
  {
    lstrcat(lstrcat(string_representation, get_str_repr(fl->Ref)), ",");
    fl = fl->next;
  }
  string_representation[str_len - 1] = '\0';
  return string_representation;
} //GetStringRepresentation

/*******************************************************************************
  TWindowData
*******************************************************************************/

TWindowData::TWindowData(int id, char* FileName)
{
  eid = id;
  FullFileName = (UCHAR*)FileName;
}

TWindowData::~TWindowData()
{
  clear(false);
}

void TWindowData::clear(bool RegistryAlso)
{
  if (FPositionsColl)
    FPositionsColl->Clear();
  if (FBookmarksStack)
  {
    FBookmarksStack->Clear();
    //while (!FBookmarksStack->Empty())
    //  delete FBookmarksStack->Pop();
  }
  if (RegistryAlso)
    RegistryStorage->DeleteRegValue(REG_KEY_BOOKMARKS, (char*)FullFileName.get());
}

void TWindowData::AddBookmark(int CurLine, int CurPos, int TopScreenLine, int LeftPos)
{
  if (!FPositionsColl)
    FPositionsColl = new CPositionsRefColl();

  struct EditorInfo* pEInfo = new EditorInfo;
  pEInfo->CurLine = CurLine;
  pEInfo->CurPos = CurPos;
  pEInfo->TopScreenLine = TopScreenLine;
  pEInfo->LeftPos = LeftPos;

  FPositionsColl->Add(pEInfo->CurLine, pEInfo);
#ifdef _DEBUG
  DebugString("after FPositionsColl->Add:");
  CRefColl<EditorInfo> *fl = FPositionsColl->next;
  while (!fl->main())
  {
    FarSprintf(tmp_str, "id = %d\n", fl->id());
    DebugString(tmp_str);
    fl = fl->next;
  }
#endif
} //AddBookmark

void TWindowData::MoveToNextBookmark(bool Next)
{
  if (!FPositionsColl) return;
  struct EditorInfo EInfo;
  struct WindowInfo wi;
  wi.Pos = eid;
  if (!Info.AdvControl(Info.ModuleNumber, ACTL_GETWINDOWINFO, (void*)&wi))
    return;
#ifdef _DEBUG
  FarSprintf(tmp_str, "Name = %s", wi.Name);
  DebugString(tmp_str);
#endif
  if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
    return;

  CRefColl<EditorInfo>* fl;
  if (Next)
  {
    fl = FPositionsColl->GetNextTo(EInfo.CurLine, EInfo.CurPos);
    fl = fl->main() ? FPositionsColl->next->main() ? NULL : FPositionsColl->next : fl;
  }
  else
  {
    fl = FPositionsColl->GetPrevTo(EInfo.CurLine, EInfo.CurPos);
    fl = fl->main() ? FPositionsColl->prev->main() ? NULL : FPositionsColl->prev : fl;
  }
  if (fl && !fl->main())
    set_cursor_pos(fl->Ref->CurPos, fl->Ref->CurLine, fl->Ref);
} //MoveToNextBookmark

void TWindowData::SaveBookmarks()
{
  if (!FPositionsColl) return;

  AutoPtr<char> positions_str (FPositionsColl->GetStringRepresentation());
  if (positions_str)
  {
    RegistryStorage->SetRegKey(REG_KEY_BOOKMARKS,
                               (const char*)FullFileName.get(), positions_str);
  }
} //SaveBookmarks

void TWindowData::LoadBookmarks()
{
  struct WindowInfo wi;
  wi.Pos = eid;
  if (!Info.AdvControl(Info.ModuleNumber, ACTL_GETWINDOWINFO, (void*)&wi))
    return;

  char s_Bookmarks[MAX_BOOKMARKS_STR_LENGTH];
  if (!RegistryStorage->GetRegKey(REG_KEY_BOOKMARKS, wi.Name, s_Bookmarks, "", MAX_BOOKMARKS_STR_LENGTH))
    return;
#ifdef _DEBUG
  FarSprintf(tmp_str, "s_Bookmarks = %s", s_Bookmarks);
  DebugString(tmp_str);
#endif
  SMatches m;
  if (!SplitRegExpr("/,/", s_Bookmarks, m))
    return;
  char str_repr[30]; int s = 0; int e = 0;
  for (int i = 0; i < m.CurMatch; i++)
  {
    int len = m.e[i] - m.s[i];
    if (len > 0)
    {
      lstrcpynA(str_repr, s_Bookmarks + s, len + 1);
      s = m.e[i] + 1;
#ifdef _DEBUG
      FarSprintf(tmp_str, "str_repr = %s", str_repr);
      DebugString(tmp_str);
#endif
    }
    int CurLine(0), CurPos(0), TopScreenLine(0), LeftPos(0);
    SMatches mr;
    if (SplitRegExpr("/:/", str_repr, mr))
    {
      int sr = 0;
      if (mr.CurMatch == 4)
      {
        char s_i[10];
        lstrcpynA(s_i, str_repr + 1 + mr.s[0], mr.e[0] - mr.s[0]);
        CurLine = FSF.atoi(s_i);
        lstrcpynA(s_i, str_repr + mr.s[1], mr.e[1] - mr.s[1] + 1);
        CurPos = FSF.atoi(s_i);
        lstrcpynA(s_i, str_repr + mr.s[2], mr.e[2] - mr.s[2] + 1);
        TopScreenLine = FSF.atoi(s_i);
        lstrcpynA(s_i, str_repr + mr.s[3], mr.e[3] - mr.s[3]);
        LeftPos = FSF.atoi(s_i);
        AddBookmark(CurLine, CurPos, TopScreenLine, LeftPos);
      }
    }
  }
} //LoadBookmarks

void TWindowData::PushBookmark(int CurLine, int CurPos, int TopScreenLine, int LeftPos)
{
  if (!FBookmarksStack)
    FBookmarksStack = new CBookmarksStack();

  struct EditorInfo* pEInfo = new EditorInfo;
  pEInfo->CurLine = CurLine;
  pEInfo->CurPos = CurPos;
  pEInfo->TopScreenLine = TopScreenLine;
  pEInfo->LeftPos = LeftPos;

  FBookmarksStack->Push(pEInfo);
}

void TWindowData::PopBookmark()
{
  if (!FBookmarksStack || FBookmarksStack->Empty()) return;
  struct EditorInfo* pEInfo = FBookmarksStack->Pop();
  if (!pEInfo) return;
  set_cursor_pos(pEInfo->CurPos, pEInfo->CurLine, pEInfo);
  delete pEInfo;
}

/*******************************************************************************
              class CSearchPaths
*******************************************************************************/

CSearchPaths::CSearchPaths(PSgmlEl elem)
{
  AddSearchPaths(elem);
  pathways = new CPathwaysArray(MAX_PATHWAYS_COUNT);
  pathways->add(get_def_path());
  was_found = new CFoundDataArray(MAX_FOUND_COUNT);
} //CSearchPaths

CSearchPaths::~CSearchPaths()
{
  for (int i = 0; i < EnvVarsCount; i++)
  {
    delete env_vars[EnvVarsCount].EnvVar;
    delete env_vars[EnvVarsCount].EnvVarValue;
  }
  free_find_data();
}

const static SCharData SFileNamesData = { { 0x0, 0x83FFE41A, 0x97FFFFFF, 0x7FFFFFE, 0, 0, 0, 0 } };

bool CSearchPaths::ProcessCtrlEnter(SLanguage* Language)
{
  crt::lmemcpy(&CharSet, (void *)&SFileNamesData, 32);
  if (!get_filename())
     return false;

  int pos = find_file(Language ? Language->ExcludedFileExts : NULL);
  if (!pos)
    return false;

  if (--pos)
    pos = ShowSelectFileMenu();
  if (pos == -1)
    return false;

  char open_file_name[MAX_PATH];
  if (!get_full_file_name(open_file_name, pos))
  {
    ClearAfterSearch();
    return false;
  }
  if (!file_exists(open_file_name))
    return false;

  int res = Info.Editor(open_file_name,
                 NULL,
                 0, 0, -1, -1,
                 0, //EF_NONMODAL|EF_ENABLE_F6,
                 0, 1);
  if (res == EEC_OPEN_ERROR)
  {
    return false; //тут ещё можно вывести сообщение об ошибке
  }

  return true;
} //ProcessCtrlEnter

void CSearchPaths::AddSearchPaths(PSgmlEl elem)
{
  DebugString("CSearchPaths::CSearchPaths::");

  PSgmlEl SearchPaths = GetChild(elem, s_SearchPaths);

  if (SearchPaths && SearchPaths->GetChrParam(s_Value))
  {
    char tmpPathWays[PWL];
    FSF.ExpandEnvironmentStr(SearchPaths->GetChrParam(s_Value), tmpPathWays, PWL);

    if (s_PathWays)
    {
      char *tmp_PathWays = new char[lstrlen(s_PathWays) + lstrlen(tmpPathWays)+3];
      lstrcat(lstrcat(lstrcpyA(tmp_PathWays, s_PathWays), tmpPathWays), ";");
      s_PathWays = tmp_PathWays;
    }
    else
    {
      s_PathWays = new char[lstrlen(tmpPathWays) + 3];
      lstrcat(lstrcpyA(s_PathWays, tmpPathWays), ";");
    }
    s_PathWays_len = lstrlen(s_PathWays);
  }

  PSgmlEl SpecVars = GetChild(elem, s_SpecVars);

  if (SpecVars)
  {
    PSgmlEl Var = GetChild(SpecVars, s_Var);

    while (Var && (EnvVarsCount < MAX_ENV_VARS))
    {
      if (Var->getname() && !lstrcmp(Var->getname(), s_Var))
      {
        InitParam(env_vars[EnvVarsCount].EnvVar, Var->GetChrParam(s_Name));
        InitParam(env_vars[EnvVarsCount].EnvVarValue, Var->GetChrParam(s_Value));
        EnvVarsCount++;
      }
      Var = Var->next();
    }
  }
} //AddSearchPaths

void CSearchPaths::MakePathWays()
{
  if (s_PathWays)
  {
    char *c, *p = s_PathWays;
    while ((c = crt::lstrchr(p, ';')) != NULL)
    {
      char* str = new char[c - p + 1];
      crt::lmemcpy(str, p, (int)(c - p));
      pathways->add(str);
      p = ++c;
    } //while
    pathways->add(p);
  }
  ResolveEnvVars();
} //MakePathWays

void CSearchPaths::PrepareForSearch()
{
  free_find_data();
  file_name = new char[MAX_PATH];
  last_path = new char[MAX_PATH];
} //PrepareForSearch

void CSearchPaths::ClearAfterSearch()
{
  free_find_data();
} //ClearAfterSearch

void CSearchPaths::free_find_data(void)
{
  was_found->cleanup();
} //free_find_data

int CSearchPaths::get_filename(void)
{
  PrepareForSearch();

  struct EditorGetString EStr; // editor  string
  EStr.StringNumber = -1; // current string
  if (Info.EditorControl(ECTL_GETSTRING, &EStr))
  {
    int mx, my;
    if (get_cursor_pos(mx, my))
    {
      int begin_word_pos;

      if (get_word(file_name, EStr.StringText, mx, EStr.StringLength, begin_word_pos))
      {
        char *beg, *end;
        int len = lstrlen(file_name);

        for (int i = 0; i < len; i++)
          if (file_name[i] == '/')
            file_name[i] = '\\';

        beg = file_name;
        while ((end = crt::lstrchr(beg, '\\')) != NULL)
          beg = end + 1;

        //- если всё просто и имя без пути -
        if (beg == file_name)
        {
          find_file_name = file_name;

          find_file_path = NULL;
          return true;
        }

        if ((beg-file_name == 1) && (file_name[0] == '\\'))
        {
          find_file_name = beg;
          find_file_path = (char *)path_root;
          return true;
        }

        *(beg-1) = 0;
        find_file_name = beg;
        find_file_path = file_name;

        return true;
      }
    }
  }
  ClearAfterSearch();
  return false;
} //get_filename

int CSearchPaths::find_file(char *ExcludedFileExts)
{
  MakePathWays();
  GetCurrentDirectory(MAX_PATH-1, last_path);

  HANDLE HF;
  WIN32_FIND_DATA FD;

  for (int m2 = 0; m2 < 2; m2++)
  {
    // первый проход на точное соотв. второй - any ext.
    for (int i = 0; i < pathways->count(); i++)
    {
      if (!SetCurrentDirectory(pathways->get(i)))
        continue;

      if (find_file_path)
        if (!SetCurrentDirectory(find_file_path))
          continue;

      if ((HF = FindFirstFile(find_file_name, &FD)) == INVALID_HANDLE_VALUE)
        continue;

      do
      {
        if (FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          continue;

        if (AlreadyFound(pathways->get(i), FD.cFileName))
          continue;

        if (ExcludedFileExts && FSF.ProcessName(ExcludedFileExts, FD.cFileName, PN_CMPNAMELIST|PN_SKIPPATH))
          continue;

        was_found->insert(FD.cFileName, i);
        //was_found[wf_count]->FileSize = FD.nFileSizeLow;
      } while (FindNextFile(HF, &FD) && was_found->count() < MAX_FOUND_COUNT);
      FindClose(HF);
    }//for

    if (!was_found->count()) lstrcat(find_file_name, ".*");
    else break;
  }//for

  SetCurrentDirectory(last_path);
  if (!was_found->count())
    ClearAfterSearch();
  return was_found->count();
} //find_file

int CSearchPaths::ShowSelectFileMenu()
{
  AutoPtr<FFarMenuItem, VectorPtr> fmi (new FFarMenuItem[was_found->count()]);
  for (int i = 0; i < was_found->count(); i++)
  {
    char tmp_str[MAX_PATH];
    FarSprintf(tmp_str, "%-12s in %s", was_found->Item(i)->file_name, pathways->get(was_found->Item(i)->path_index));
    fmi[i].Text = new char[MAX_PATH];
    if (i < 9)
      FarSprintf(fmi[i].Text, "&%d %s", i + 1, tmp_str);
    else if (i < 15)
      FarSprintf(fmi[i].Text, "&%c %s", i - 9 + 'A', tmp_str);
    else
      FarSprintf(fmi[i].Text, "%-2s%s", "", tmp_str);
  }
  AutoPtr<CPluginMenu> PluginMenu (new CPluginMenu(was_found->count(), fmi));

  char msgSelect[256], msgTotal[256];
  const char *PMStrings[] =
  {
    get_msg(MSelect),
    get_msg(MTotal)
  };

  FarSprintf(msgSelect, PMStrings[0], find_file_name);
  FarSprintf(msgTotal, PMStrings[1], was_found->count());

  int menu_code = PluginMenu->Execute(msgSelect, msgTotal);

  for (int i = 0; i < was_found->count(); i++)
    delete fmi[i].Text;

  if (menu_code == -1)
    ClearAfterSearch();
  return (menu_code);
} //ShowSelectFileMenu

int CSearchPaths::get_full_file_name(char *dest, int pos)
{
  return (get_full_file_name(dest, was_found->Item(pos)));
} //get_full_file_name

int CSearchPaths::get_full_file_name(char *dest, LPFND fnd)
{
  if (fnd->file_path)
    lstrcpyA(dest, fnd->file_path);
  else
    lstrcpyA(dest, pathways->get(fnd->path_index));

  if (find_file_path)
  {
    if (dest[lstrlen(dest)-1] != '\\')
      lstrcat(dest, "\\");
    lstrcat(dest, find_file_path);
  }

  if (dest[lstrlen(dest)-1] != '\\')
   lstrcat(dest, "\\");

  lstrcat(dest, fnd->file_name);

  return (int)lstrlen(dest);
} //get_full_file_name

int CSearchPaths::AlreadyFound(char *path, char *file_name)
{
  FOUND fnd;
  fnd.file_name = file_name;
  fnd.file_path = path;
  char cur_file_name[MAX_PATH]; int cur_file_name_len;

  if ((cur_file_name_len = get_full_file_name(cur_file_name, &fnd)) == 0)
    return false;

  for (int i = 0; i < was_found->count(); i++)
  {
    char file_name[MAX_PATH]; int file_name_len;
    if ((file_name_len = get_full_file_name(file_name, was_found->Item(i))) == 0)
      return false;

    if ((file_name_len == cur_file_name_len) &&
         !FSF.LStrnicmp(file_name, cur_file_name, file_name_len))
     return true;
  }
  return false;
} //AlreadyFound

void CSearchPaths::ResolveEnvVars(void)
{
#ifdef _DEBUG
  FarSprintf(tmp_str, " ===== ResolveEnvVars::count = %d; EnvVarsCount = %d", pathways->count(), EnvVarsCount);
  DebugString(tmp_str);
#endif

  for (int i = 0; i < pathways->count(); i++)
  {
    char *pw = pathways->get(i);
    if (!pw) continue;

    if ((*pw == '$') && (pw[1] == '(')) //нашли специальную переменную
    {
      for (int j = EnvVarsCount - 1; j >= 0; j--)
      {
        int EnvVarLen = lstrlen(env_vars[j].EnvVar);

        if (!FSF.LStrnicmp(&pw[2], env_vars[j].EnvVar, (int)EnvVarLen)  &&
             pw[EnvVarLen + 2] == ')')
        {
          //заменим $(ENVVAR) на EnvVarValue
          crt::size_t EnvVarValueLen = lstrlen(env_vars[j].EnvVarValue);
          crt::size_t pw_len = lstrlen(&pw[EnvVarLen + 3]);
          char *pathway = new char[EnvVarValueLen + pw_len + 1];
          lstrcpyA(pathway, env_vars[j].EnvVarValue);
          lstrcat(pathway, &pw[EnvVarLen + 3]);
          pathways->insert(i, pathway);
          pw = pathway;
        }
      }//for
    }//if
    char *spw = crt::lstrstr(pw, "\\...");
    if (spw) //нашли идентификатор подкаталогов
    {
      *spw = 0;
      FindAllSubDirs(pw);
    }
  }//for
} //ResolveEnvVars

void CSearchPaths::FindAllSubDirs(char *RootDir)
{
  HANDLE HF;
  WIN32_FIND_DATA FD;
  if (SetCurrentDirectory(RootDir))
  {
    if ((HF = FindFirstFile("*.*", &FD)) == INVALID_HANDLE_VALUE)
      return;
    do
    {
      if ((FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          FD.cFileName[0] != '.')
      {
        char *pathway = new char[lstrlen(RootDir) + lstrlen(FD.cFileName) + 3];
        lstrcat(lstrcat(lstrcpyA(pathway, RootDir), "\\"), FD.cFileName);
        pathways->add(pathway);
        FindAllSubDirs(pathway);
      }
    } while (FindNextFile(HF, &FD) && pathways->count() < MAX_PATHWAYS_COUNT);
    FindClose(HF);
  }
  SetCurrentDirectory(RootDir);
} //FindAllSubDirs

char* CSearchPaths::get_def_path(void)
{
  AutoPtr<char> def_path (new char[MAX_PATH]);

  struct EditorInfo EInfo;
  Info.EditorControl(ECTL_GETINFO, &EInfo);

  char *c, *p = (char *)EInfo.FileName;
  while ((c = crt::lstrchr(p, '\\')) != NULL)
    p = ++c;

  if (p == EInfo.FileName)
  {
    if (GetCurrentDirectory(MAX_PATH-1, def_path) == 0)
      return NULL;
    return def_path.Detach();
  }
  lstrcpynA(def_path, EInfo.FileName, (int)(p-1-EInfo.FileName) + 1);
  return def_path.Detach();
} //get_def_path

/*******************************************************************************
              class SLanguage
*******************************************************************************/

SLanguage::SLanguage(CXMLFile* owner, CSgmlEl* elem)
{
#ifdef _DEBUG
 FarSprintf(tmp_str, "s_Language::s_Language::1:elem = %s", elem->getname());
 DebugString(tmp_str);
#endif
  Owner = owner;
  if (elem->getname() && !lstrcmp(elem->getname(), s_Language))
  {
    InitParam(name, elem->GetChrParam(s_Name));
    InitParam(FileExts, elem->GetChrParam(s_FileExts));
    InitParam(ExcludedFileExts, elem->GetChrParam(s_ExcludedFileExts));
    InitParam(SourceFiles, elem->GetChrParam(s_SourceFiles));
    InitParam(Headers, elem->GetChrParam(s_Headers));

    if (GetChild(elem, s_Class))
      Class = new SClass(elem);

    if (GetChild(elem, s_GlobalProc))
      Method = new SMethod(elem, s_GlobalProc);
  }

  if (GetChild(elem, s_SearchPaths))
    Owner->AddSearchPaths(elem);

}

SLanguage::~SLanguage()
{
  delete Method;
  delete Class;

  delete Headers;
  delete SourceFiles;
  delete ExcludedFileExts;
  delete FileExts;
  delete name;
}

/*******************************************************************************
              class SClass
*******************************************************************************/

SClass::SClass(PSgmlEl elem)
{
  PSgmlEl Class = GetChild(elem, s_Class);
  if (Class)
  {
    InitParam(Name, Class->GetChrParam(s_Name));
    InitParam(Definition, Class->GetChrParam(s_Definition));
    InitParam(End, Class->GetChrParam(s_End));

    if (GetChild(Class, s_Method))
     Method = new SMethod(Class, s_Method);
  }
}

SClass::~SClass()
{
  delete Method;

  delete End;
  delete Definition;
  delete Name;
}

/*******************************************************************************
              class SMethod
*******************************************************************************/

SMethod::SMethod(PSgmlEl elem, const char* Tag)
{
  PSgmlEl Method = GetChild(elem, Tag);
  if (Method)
  {
    InitParam(Name, Method->GetChrParam(s_Name));
    InitParam(Type, Method->GetChrParam(s_Type));
    InitParam(Definition, Method->GetChrParam(s_Definition));
    InitParam(Implementation, Method->GetChrParam(s_Implementation));
  }
}

SMethod::~SMethod()
{
  delete Implementation;
  delete Definition;
  delete Type;
  delete Name;
}

/*******************************************************************************
              class CPluginMenu
*******************************************************************************/

CPluginMenu::CPluginMenu(int NumElements, PFFarMenuItem Elements, int FNumElements)
  : CRefArray<FarMenuItem>(NumElements)
{
  Setup(0, Elements, (-1 == FNumElements) ? NumElements : FNumElements);
}

void CPluginMenu::Setup(int From, PFFarMenuItem p, int ECount)
{
  for (int n = 0; n < ECount; n++)
    Setup(From+n, p[n]);
}

FarMenuItem *CPluginMenu::Setup(int num, const FFarMenuItem &mi)
{
  FarMenuItem *p = Item(num);
  if (!p) return NULL;

  lstrcpyA(p->Text, mi.Text ? mi.Text : "");
  p->Selected = mi.Selected;
  p->Checked = mi.Checked;
  p->Separator = mi.Separator;

  return p;
}

int CPluginMenu::Execute(const char *Title, const char *Bottom)
{
  return Info.Menu(Info.ModuleNumber, -1, -1, 0, FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE, Title,
               Bottom, //bottom
               "Contents", //helptopic
               NULL, NULL, //breakkeys, breakcode
               (FarMenuItem *)Items(), Count());
}

/*******************************************************************************
              class CXMLFile
*******************************************************************************/

CXMLFile::CXMLFile()
{
  init(NULL);
}

CXMLFile::CXMLFile(char *xml_file_name)
{
  init(xml_file_name);
}

CXMLFile::~CXMLFile()
{
}

void CXMLFile::init(char *file_name)
{
  if (file_name)
  {
    crt::size_t len = lstrlen(file_name) + 1;
    XMLFileName = new char[len];
    lstrcpyA(XMLFileName, file_name);
  }
  else
  {
    XMLFileName = new char[lstrlen(Info.ModuleName) + 1];
    lstrcpyA(XMLFileName, Info.ModuleName);
  }

  filepath = new char[MAX_PATH];
  int i = lstrlen(XMLFileName) - 1;
  while (XMLFileName[i] != '.')
    i--;
  lstrcpyA(filepath, XMLFileName);
  lstrcpyA(XMLFileName + i, ".xml\0");
  while (filepath[i] != '\\')
    i--;
  filepath[i+1] = 0;

  SearchPaths = new CSearchPaths(NULL);
  char *xml_data; UINT sz;
  GetFile(XMLFileName, xml_data, sz);
  loaddata(xml_data, sz);
  delete xml_data;
}

void CXMLFile::AddSearchPaths(PSgmlEl elem)
{
  if (!SearchPaths)
    SearchPaths = new CSearchPaths(NULL);
  SearchPaths->AddSearchPaths(elem);
}

bool CXMLFile::loaddata(char *data, int len)
{
  DebugString(" ============ loaddata");

  if (!(data && len))
    return false;

  AutoPtr<CSgmlEl> base (new CSgmlEl());
  if (!base)
    return false;

  if (!base->parse(data, len))
    return false;

  PSgmlEl basetag = searchbasetag(base);
#ifdef _DEBUG
  FarSprintf(tmp_str, "basetag = %x", basetag);
  DebugString(tmp_str);
#endif

  if (basetag && !FSF.LStrnicmp(basetag->GetChrParam(s_version), s_versionID, (int)lstrlen(s_versionID)) &&
     basetag->child())
  {
    readcdata(basetag);
    return true;
  }
  return false;
} //loaddata

void CXMLFile::readcdata(PSgmlEl basetag)
{
  char *param;

  PSgmlEl elem = basetag->child();
  if (!elem) return;
#ifdef _DEBUG
  FarSprintf(tmp_str, "\r\n ========== readcdata\r\nnelem = %s", elem->GetChrParam("name"));
  DebugString(tmp_str);
#endif

  for (; elem; elem = elem->next())
  {
    if (!elem->getname()) continue;
    if (!lstrcmp(elem->getname(), s_Include))
    {
      if (((param = elem->GetChrParam(s_File)) == NULL) || !filepath)
        continue;
      includefile(param);
    }
    else if (!lstrcmp(elem->getname(), s_Language))
    {
      struct EditorInfo EInfo;
      if (!Info.EditorControl(ECTL_GETINFO, &EInfo))
        continue;

      char *FileExts = elem->GetChrParam(s_FileExts);

      if (FileExts && !FSF.ProcessName(FileExts, (char *)EInfo.FileName, PN_CMPNAMELIST|PN_SKIPPATH))
        continue;

#ifdef _DEBUG
    FarSprintf(tmp_str, "readdata::EInfo.FileName = \r\n%s\r\n", EInfo.FileName);
    DebugString(tmp_str);
#endif

      Language = new SLanguage(this, elem);
    }
    else if (!lstrcmp(elem->getname(), s_SearchPaths))
    {
      SearchPaths->AddSearchPaths(basetag);
    }
  }
} //readcdata

bool CXMLFile::includefile(char *param)
{
  DebugString(" === includefile");

  char path[MAX_PATH];
  lstrcpyA(path, filepath);
  int i = lstrlen(path);
  for (; i; i--)
    if (path[i] == '\\' || path[i] == '/') break;
  lstrcpyA(path+i+1, param);

  char* data; UINT sz;
  GetFile(path, data, sz);
  if (!(data && sz))
    return false;

  AutoPtr<CSgmlEl> BaseEl (new CSgmlEl);

  if (BaseEl->parse(data, sz))
  {
    PSgmlEl tmpEl = searchbasetag(BaseEl);
    if (tmpEl && !FSF.LStrnicmp(tmpEl->GetChrParam(s_version), s_versionID, (int)lstrlen(s_versionID)) &&
        tmpEl->child())
    {
      readcdata(tmpEl);
      delete data;
      return true;
    }
  }
  delete data;
  return false;
} //includefile

PSgmlEl CXMLFile::searchbasetag(PSgmlEl elem)
{
  while (elem)
  {
    if (elem->gettype() == EBLOCKEDEL &&
         elem->getname() && !lstrcmp(elem->getname(), s_TextNavigate))
     return (elem);
    elem = elem->next();
  }
  return NULL;
} //searchbasetag
