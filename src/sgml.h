//sgml.h

//
//  Copyright (c) Cail Lomecb (Igor Ruskih) 1999-2001 <ruiv@uic.nnov.ru>
//  You can use, modify, distribute this code or any other part
//  of this program in sources or in binaries only according
//  to License (see /doc/license.txt for more information).
//
#ifndef __SGML_H
#define __SGML_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// sometimes need it...
bool get_number(char *str, int *res);

typedef class CSgmlEl *PSgmlEl;
typedef class CSgmlEdit *PSgmlEdit;
typedef char  *TParams[2];

#define MAXPARAMS 0x20
#define MAXTAG 0x10
#define SP 1

enum ElType{
  EPLAINEL, ESINGLEEL, EBLOCKEDEL, EBASEEL
};

class CSgmlEl
{
  //for derived classes
  virtual PSgmlEl createnew(ElType type, PSgmlEl parent, PSgmlEl after);
  virtual bool init();

  void destroylevel();
  void insert(PSgmlEl El);
  bool setcontent(const char *src,int sz);
  void substquote(TParams par, char *sstr, char c);

  char *content;
  int contentsz;
  char name[MAXTAG];

  PSgmlEl eparent;
  PSgmlEl enext;
  PSgmlEl eprev;
  PSgmlEl echild;
  int chnum;
  ElType type;

  TParams params[MAXPARAMS];
  int parnum;

 public:
  CSgmlEl();
  ~CSgmlEl();
  bool parse(const char *src,int sz);

  virtual PSgmlEl parent();
  virtual PSgmlEl next();
  virtual PSgmlEl prev();
  virtual PSgmlEl child();

  ElType gettype();
  char *getname();
  char *getcontent();
  int getcontentsize();

  char *GetParam(int no);
  char *GetChrParam(const char* par);
  bool GetIntParam(const char* par, int& result);
  //bool GetFltParam(char *par,double *result);

  PSgmlEl search(const char* TagName);
  PSgmlEl enumchilds(int no);
  // in full hierarchy
  virtual PSgmlEl fprev();
  virtual PSgmlEl fnext();
  virtual PSgmlEl ffirst();
  virtual PSgmlEl flast();
};

#endif /* __SGML_H */
