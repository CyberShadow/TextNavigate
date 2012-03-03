//pclasses.h
#ifndef __PCLASSES_H
#define __PCLASSES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "crtdll.h"
#include "commons.h"
#include "string.hpp"
#include "Dialog.h"
#include "sgml.h"
#include "CRegExp.h"
#include "Dialog.h"
#include "tools.h"
#include "TextNavigate.h"
#include "reg.h"

typedef struct _FOUND
{
  unsigned int path_index;
  const char *file_name;
  const char *file_path;
} FOUND, *LPFND;

#define MAX_ENV_VARS 16
typedef struct ENV_VAR
{
 char *EnvVar;
 char *EnvVarValue;
} ENV_VARS[MAX_ENV_VARS];

#define PWL 0x400
#define MAX_PATHWAYS_COUNT 0x100   //кол-во pathways
#define MAX_FOUND_COUNT 50         //кол-во wasfound

#define MAX_WORD_LENGTH 100
#define MAX_INCREMENTAL_SEARCH_LENGTH 20

//forward declarations
struct SLanguage;
class CXMLFile;

/*******************************************************************************
              class CFoundDataArray
*******************************************************************************/

class CFoundDataArray : public CRefArray<_FOUND>
{
  private:
    int m_count;
  public:
    CFoundDataArray(int Capacity) : CRefArray<_FOUND>(Capacity), m_count(0)
    { }

    ~CFoundDataArray()
    {
      cleanup();
    }

    void insert(char* FileName, int index)
    {
      int len = lstrlen(FileName);
      _FOUND* item = Item(m_count);
      item->file_name = new char[len + 1];
      lstrcpyA((char*)item->file_name, FileName);
      item->file_path = NULL;
      item->path_index = index;
      m_count++;
    }

    void cleanup()
    {
      for (int i = 0; i < m_count; i++)
      {
        if (Item(i)->file_name)
          delete Item(i)->file_name;
        if (Item(i)->file_path)
          delete Item(i)->file_path;
      }
      m_count = 0;
    }

    int count()
    {
      return m_count;
    }
}; //CFoundDataArray

/*******************************************************************************
              class CPathwaysArray
*******************************************************************************/

class CPathwaysArray
{
  private:
    char** str_array;
    int m_MaxCount;
    int m_count;
  public:
    CPathwaysArray(int MaxCount) : m_MaxCount(MaxCount), m_count(0)
    {
      str_array = new char*[MaxCount];
    }

    char* operator[](int index)
    {
      return str_array[index];
    }

    const char* operator[](int index) const
    {
      return str_array[index];
    }

    void insert(int index, char* str)
    {
      _delete(index);
      str_array[index] = str;
    }

    void add(char* str)
    {
      if (str && *str && m_count < MAX_PATHWAYS_COUNT)
        insert(m_count++, str);
    }

    char* get(int index)
    {
      return operator[](index);
    }

    void _delete(int index)
    {
      delete str_array[index];
      str_array[index] = NULL;
    }

    int count()
    {
      return m_count;
    }

    void cleanup()
    {
      for (int i = 0; i < m_count; i++)
        _delete(i);
      m_count = 0;
    }
}; //CPathwaysArray

/*******************************************************************************
              class CSearchPaths
*******************************************************************************/

class CSearchPaths
{
  private:
    AutoPtr<char> s_PathWays;
    int s_PathWays_len;
    AutoPtr<char> file_name;
    AutoPtr<char> last_path;
    AutoPtr<CPathwaysArray> pathways;
    AutoPtr<CFoundDataArray> was_found;
    char *find_file_name; //указатель на имя
    char *find_file_path; //указатель на путь
    ENV_VARS env_vars;
    int EnvVarsCount;

    char *get_def_path();
    void ResolveEnvVars();
    int AlreadyFound(char *path, char *file_name);
    int get_full_file_name(char *dest, LPFND fnd); //!!!

    void free_find_data();
    void PrepareForSearch();
    void MakePathWays();
    void FindAllSubDirs(char *RootDir);
 public:
   CSearchPaths(PSgmlEl elem);
   ~CSearchPaths();

   bool ProcessCtrlEnter(SLanguage* Language);

   int get_filename();
   int find_file(char *ExcludedFileExts);
   int get_full_file_name(char *dest, int pos);
   void ClearAfterSearch();

   void AddSearchPaths(PSgmlEl elem);
   int ShowSelectFileMenu();
}; //CSearchPaths

/*******************************************************************************
              struct SMethod
*******************************************************************************/

struct SMethod
{
  SMethod(PSgmlEl elem, const char* Tag);
  ~SMethod();

  char *Name, *Type, *Definition, *Implementation;
}; //SMethod

/*******************************************************************************
              struct SClass
*******************************************************************************/

struct SClass
{
  SClass(PSgmlEl elem);
  ~SClass();

  char *Name, *Definition, *End;
  SMethod* Method;
}; //SClass

/*******************************************************************************
              struct SLanguage
*******************************************************************************/

struct SLanguage
{
  SLanguage(CXMLFile* owner, PSgmlEl elem = NULL);
  ~SLanguage();

  char *name, *FileExts, *ExcludedFileExts, *SourceFiles, *Headers;
  SClass* Class;
  SMethod* Method;
  CXMLFile* Owner;
}; //SLanguage

/*******************************************************************************
              class CXMLFile
*******************************************************************************/

class CXMLFile
{
  private:
    AutoPtr<char> XMLFileName;
    AutoPtr<char> filepath;

    PSgmlEl searchbasetag(PSgmlEl base);
    bool loaddata(char *data, int len);
    bool includefile(char *param);
    void readcdata(PSgmlEl basetag);
  public:
    CXMLFile();
    CXMLFile(char *xml_file_name);
    ~CXMLFile();
    void init(char *file_name);
    void AddSearchPaths(PSgmlEl elem);

    SLanguage* Language;
    AutoPtr<CSearchPaths> SearchPaths;
}; //CXMLFile

/*******************************************************************************
              class CSortedRefColl
  collection sorted by id
*******************************************************************************/

template<class T, bool DeleteEqual = false>
class CSortedRefColl : public CRefColl<T>
{
  typedef CRefColl<T> ancestor;

  protected:
    virtual int compare_refs(const T* ref1, const T* ref2)
    { return 0; }
    virtual void replaceby(T* ref_to_replace, const T* ref)
    { }

  public:
    explicit CSortedRefColl() : ancestor(true)
    { }

    T *Add(int fid = -1, T *&ref = (T *)NULL)
    {
      ancestor *pos = NULL;
      ancestor *fl = next;
      while (fl != this)
      {
        if (fl->id() <= fid && (fl->next->id() > fid || fl->next->main()))
        {
          pos = fl;
          break;
        }
        fl = fl->next;
      }
      fl = pos ? pos : this;
      if (fl->id() == fid && (0 == compare_refs(fl->Ref, ref)))
      {
        if (!DeleteEqual)
        {
          replaceby(fl->Ref, ref);
          delete ref;
          ref = NULL;
          return fl->Ref;
        }
        else
        {
          delete fl;
          return ref;
        }
      }
      else
        return _insert_after(fl, fid, ref);
    }
}; //CSortedRefColl

/*******************************************************************************
              class CPositionsRefColl
  sorted collection of cursor positions
*******************************************************************************/

class CPositionsRefColl : public CSortedRefColl<EditorInfo, true>
{
  typedef CSortedRefColl<EditorInfo, true> ancestor;

  private:
    char* get_str_repr(const EditorInfo* pEditorInfo);

  protected:
    int compare_refs(const EditorInfo* ref1, const EditorInfo* ref2);
    void replaceby(EditorInfo* ref_to_replace, const EditorInfo* ref);

  public:
    CPositionsRefColl() : ancestor()
    { }

    CRefColl<EditorInfo>* GetNextTo(int row, int pos);
    CRefColl<EditorInfo>* GetPrevTo(int row, int pos);

    char* GetStringRepresentation();
}; //CPositionsRefColl

/*******************************************************************************
              class CXMLFilesColl
  collection of XML files
*******************************************************************************/

class CXMLFilesColl : public CRefColl<CXMLFile>
{
  public:
    CXMLFilesColl(CXMLFile* XMLFile = NULL) : CRefColl<CXMLFile>(true, XMLFile)
    { }
}; //CXMLFilesColl

/*******************************************************************************
              class CBookmarksStack
  stacked bookmarks
*******************************************************************************/
class CBookmarksStack : public CStack<EditorInfo>
{
  public:
    CBookmarksStack() : CStack<EditorInfo>()
    { }
   
}; //CBookmarksStack

/*******************************************************************************
              class TWindowData
  holds editor window info
*******************************************************************************/
class TWindowData
{
private:
  int eid; //editor id
  string FullFileName;
  AutoPtr<CPositionsRefColl> FPositionsColl; //standard bookmarks
  AutoPtr<CBookmarksStack> FBookmarksStack; //stacked bookmarks
public:
  TWindowData(int id, char* FileName);
  ~TWindowData();
  void clear(bool RegistryAlso);
  
  void AddBookmark(int CurLine, int CurPos, int TopScreenLine, int LeftPos);
  void MoveToNextBookmark(bool Next);
  void LoadBookmarks();
  void SaveBookmarks();

  void PushBookmark(int CurLine, int CurPos, int TopScreenLine, int LeftPos);
  void PopBookmark();
};

class CWindowsColl : public CRefColl<TWindowData>
{
  public:
    CWindowsColl(TWindowData* window_data = NULL)
    : CRefColl<TWindowData>(true, window_data)
    { }
};

/*******************************************************************************
              class CTextNavigate
*******************************************************************************/

enum TEditorState {
  esStateNormal,
  esCtrlKPressed,
  esCtrlIPressed
};

class CTextNavigate
{
  private:
    CRegExp reg;
    int FKeyCode, FPrevKeyCode;
    TEditorState FEditorState;
    char FIncrementalSearchBuffer[MAX_INCREMENTAL_SEARCH_LENGTH];
    char *FIncrementalSearchBufferEnd;

    CXMLFile* XMLFile;
    AutoPtr<CXMLFilesColl> XMLFilesColl;
    AutoPtr<CWindowsColl> windows;

    char word[MAX_WORD_LENGTH];

    char MethodType[80];
    char MethodName[100];
    char ClassName[80];

    bool IsHeader;
    bool GlobalProc;
    char MethodDefinition[100];
    char ClassDefinition[100];
    char MethodImplementation[200];
    int MethodNamePos;

    SClass* Class;
    SMethod* Method;

    int strreplace(char *str, const char *pattern, const char *value);
    void ReplaceSpecRegSymbols(char *str);
    int GetMatch(char *Match, const SMatches &m, const char *str, int n);
    void DrawTitle();

    int do_search(const char* String, const char* substr, bool SearchUp, int begin_word_pos, int &CurLine, bool SearchSelection, bool casesensitive);

    bool SearchBackward(char *RegExpr, int &CurLine, char *&StringText, SMatches &m);
    bool SearchForward(char *RegExpr, int &CurLine, char *&StringText, SMatches &m);
    bool IsInClassDefinition(int CurLine);
    bool IsGlobalMethodImplementation(int CurLine);
    int SearchForMethodImplementation(int &CurLine, int GlobalProc);
    bool SearchForMethodImplementation2(int &CurLine, int &x_pos);
    bool SearchForMethodDefinition2(int &CurLine, bool GlobalProc);
    bool SearchForMethodDefinition(int &CurLine, int &x_pos, bool GlobalProc);
    bool FindMethodDefinition(int &CurLine, int &x_pos);
    bool FindMethodImplementation(int &CurLine, int &x_pos);

    void MoveToNextBookmark(bool Next);
    void SaveBookmarks(int id);
    void LoadBookmarks(int id);
    
    void SelectFound(int StringNumber, int StartPos, int Len);

  public:
    CTextNavigate();
    ~CTextNavigate();

    int ProcessEditorInput(const INPUT_RECORD *Rec);
    int ProcessEditorEvent(int Event, void *Param);

    int processCtrlAltUpDown(int FKeyCode, int FPrevKeyCode);
    int processCtrlEnter(void);
    int processCtrlShiftUpDown(int FKeyCode);

    void AddBookmark();
    void ClearBookmarks(int id, bool RegistryAlso);

    void StartIncrementalSearch();

    void Push();
    void Pop();
    
    int ShowPluginMenu();
    void config_plugin();
};

struct FFarMenuItem
{
  char *Text;
  int Selected;
  int Checked;
  int Separator;
};

typedef struct FFarMenuItem* PFFarMenuItem;
typedef struct FarMenuItem* PFarMenuItem;
typedef class CPluginMenu* PPluginMenu;

/*******************************************************************************
              class CPluginMenu
*******************************************************************************/

class CPluginMenu : public CRefArray<FarMenuItem>
{
 public:
  CPluginMenu(int NumElements, PFFarMenuItem Elements, int FNumElements = -1);
  FarMenuItem *Setup(int num, const FFarMenuItem &mi);
  void Setup(int From, PFFarMenuItem p, int count);
  int Execute(const char *Title, const char *Bottom);
};

extern CTextNavigate* TextNavigate;

#endif /* __PCLASSES_H */
