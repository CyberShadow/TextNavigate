//CRegExp.h

//
//  Copyright (c) Cail Lomecb (Igor Ruskih) 1999-2000 <ruiv@uic.nnov.ru>
//  You can use, modify, distribute this code or any other part
//  of colorer library in sources or in binaries only according
//  to Colorer License (see /doc/(rus/)?license.txt for more information).
//
#ifndef __CailRegExp__
#define __CailRegExp__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "crtdll.h"

#define MatchesNum 0x10

enum EOps
{
  ReBlockOps = 0x1000,
  ReMul,              // *
  RePlus,             // +
  ReQuest,            // ?
  ReNGMul,            // *?
  ReNGPlus,           // +?
  ReNGQuest,          // ??
  ReRangeN,           // {n,}
  ReRangeNM,          // {n,m}
  ReNGRangeN,         // {n,}?
  ReNGRangeNM,        // {n,m}?
  ReOr,               // |
  ReBehind  = 0x1100, // ?#n
  ReNBehind = 0x1200, // ?~n
  ReAhead   = 0x1300, // ?=
  ReNAhead  = 0x1400, // ?!

  ReSymbolOps = 0x2000,
  ReEmpty,
  ReSymb,             // a b \W \s ...
  ReEnum,             // []
  ReNEnum,            // [^]
  ReBrackets,         // (...)
  ReBkTrace = 0x2100, // \yN
  ReBkBrack = 0x2200 // \N
};

enum ESymbols
{
  ReAnyChr = 0x4000,  // .
  ReSoL,              // ^
  ReEoL,              // $
  ReDigit,            // \d
  ReNDigit,           // \D
  ReWordSymb,         // \w
  ReNWordSymb,        // \W
  ReWSpace,           // \s
  ReNWSpace,          // \S
  ReUCase,            // \u
  ReNUCase ,          // \l
  ReWBound,           // \b
  ReNWBound,          // \B
  RePreNW,            // \c
  ReStart,            // \m

  ReEnd,              // \M
  ReChr    = 0x0      // Char in Lower Byte
};

enum ETempSymb
{
  ReTemp = 0x7000,
  ReLBrack, ReRBrack,
  ReEnumS, ReEnumE, ReNEnumS,
  ReRangeS, ReRangeE, ReNGRangeE, ReFrToEnum
};

#define BackSlash '\\'

typedef union SCharData
{
  int  IArr[8];
  char CArr[32];
  void SetBit(unsigned char Bit);
  void ClearBit(unsigned char Bit);
  bool GetBit(unsigned char Bit) const;
} *PCharData;

typedef struct SRegInfo
{
  void init();
  void done();

  SRegInfo()  { init(); };
  ~SRegInfo() { done(); };
  
  EOps   Op;
  union
  {
    SRegInfo *Param;
    int Symb;
    PCharData ChrClass;
  } un;
  
  short int s, e;
  SRegInfo *Parent;
  SRegInfo *Next;
} *PRegInfo;

typedef struct SMatches
{
  short int s[MatchesNum];
  short int e[MatchesNum];
  int CurMatch;
} *PMatches;

typedef class CRegExp
{
  PRegInfo Info;
  PMatches BkTrace;
  bool NoCase,Extend,NoMoves;
  bool Error;
  int  *Exprn;
  char *toParse, *StartPos;
  char *End,*Start;
  char *BkStr;
  int  FirstChar;

  bool SetExprLow(char *Expr);
  bool SetStructs(PRegInfo &Info,int st,int end);
  void Optimize();
  bool CheckSymb(int Symb,bool Inc);
  bool LowParse(PRegInfo Re);
  bool LowParseRe(PRegInfo &Next);
  bool LowCheckNext(PRegInfo Re);
  bool ParseRe(char *Str);
  bool QuickCheck();
 public:
  PMatches Matches;
  int Ok, CurMatch;
  
  CRegExp();
  CRegExp(char *Text);
  ~CRegExp();
  
  bool isok();
  bool SetNoMoves(bool Moves);
  bool SetBkTrace(char *Str,PMatches Trace);
  bool SetExpr(char *Expr);
  bool Parse(char *Str, PMatches Mtch);
  bool Parse(char *Str, char *Sol, char *Eol, PMatches Mtch, int Moves = -1);
  //bool static Evaluate(char *Expr,char *Str);
} *PRegExp;

#endif
