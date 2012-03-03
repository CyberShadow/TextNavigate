//CRegExp.cpp

//
//  Copyright (c) Cail Lomecb (Igor Ruskih) 1999-2000 <ruiv@uic.nnov.ru>
//  You can use, modify, distribute this code or any other part
//  of colorer library in sources or in binaries only according
//  to Colorer License (see /doc/(rus/)?license.txt for more information).
//

//  Updated Sun 10 Sep 10:31:59 2000 by Alexander Nazarenko AKA /CorWin

#include "CRegExp.h"

//Up: /[A-Z \x80-\x9f \xf0 ]/x
//Lo: /[a-z \xa0-\xaf \xe0-\xef \xf1 ]/x
//Wd: /[\d _ A-Z a-z \xa0-\xaf \xe0-\xf1 \x80-\x9f]/x

// Dos866
const static SCharData 
   UCData  = { { 0x0, 0x0,       0x7fffffe,  0x0,       0xffffffff, 0x0,    0x0, 0x10000 } },
   LCData  = { { 0x0, 0x0,       0x0,        0x7fffffe, 0x0,        0xffff, 0x0, 0x2ffff } },
   WdData  = { { 0x0, 0x3ff0000, 0x87fffffe, 0x7fffffe, 0xffffffff, 0xffff, 0x0, 0x3ffff } },
   DigData = { { 0x0, 0x3ff0000, 0x0,        0x0,       0x0,        0x0,    0x0, 0x0     } };

// cp1251
// SCharData UCData  = {0x0, 0x0, 0x7fffffe, 0x0, 0x0, 0x0, 0xffffffff, 0x0},
//           LCData  = {0x0, 0x0, 0x0, 0x7fffffe, 0x0, 0x0, 0x0, 0xffffffff},
//           WdData  = {0x0, 0x3ff0000, 0x87fffffe, 0x7fffffe, 0x0, 0x0, 0xffffffff, 0xffffffff},
//           DigData = {0x0, 0x3ff0000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
//

int GetNumber(int *str, int s, int e)
{
  int r = 1, num = 0;
  if ( e < s )
    return -1;
  for ( int i = e-1 ; i >= s ; i-- )
  {
    if ( str[i] > '9' || str[i] < '0')
      return -1;
    num += (str[i]-0x30)*r;
    r *= 10;
  }
  return num;
}

bool IsDigit(char Symb)
{
  return DigData.GetBit(Symb);
}

bool IsWord(char Symb)
{
  return WdData.GetBit(Symb);
}

bool IsUpperCase(char Symb)
{
  return UCData.GetBit(Symb);
}

bool IsLowerCase(char Symb)
{
  return LCData.GetBit(Symb);
}

char LowCase(char Chr)
{
  if ( UCData.GetBit(Chr) )
    return (char)(Chr+(char)0x20);
  return Chr;
}

///////////////////////////////////////////////
void SRegInfo::init()
{
  Next = Parent = 0;
  un.Param = 0;
  Op = ReEmpty;
}

void SRegInfo::done()
{
  if ( Next )
  {
    Next->done();
    winDel(Next);
  }
  if ( un.Param )
    switch( Op )
    {
      case ReEnum:
      case ReNEnum:
        winDel(un.ChrClass);
        break;
      default:
        if ( Op > ReBlockOps && Op < ReSymbolOps || Op == ReBrackets )
        {
          un.Param->done();
          winDel(un.Param);
        }
        break;
    }
}

void SCharData::SetBit(unsigned char Bit)
{
  int p = Bit/8;
  CArr[p] |= (char)(1 << Bit%8);
}

void SCharData::ClearBit(unsigned char Bit)
{
  int p = Bit/8;
  CArr[p] &= (char)(~(1 << Bit%8));
}

bool SCharData::GetBit(unsigned char Bit) const
{
  int p = (unsigned char)Bit/8;
  return (CArr[p] & (1 << Bit%8))!=0;
}

    /////////////////////////////////////////////////////////////////
    //////////////////////  RegExp Class  ///////////////////////////
    /////////////////////////////////////////////////////////////////

CRegExp::CRegExp()
{
  Info = 0;
  Exprn = 0;
  NoMoves = false;
  Error = true;
  FirstChar = 0;
  CurMatch = 0;
}

CRegExp::CRegExp(char *Text)
{
  Info = 0;
  Exprn = 0;
  NoMoves = false;
  Error = true;
  FirstChar = 0;
  CurMatch = 0;
  if ( Text )
    SetExpr(Text);
}

CRegExp::~CRegExp()
{
  if ( Info )
  {
    Info->done();
    winDel(Info);
  }
}

bool CRegExp::SetExpr(char *Expr)
{
  if ( !this )
    return false;
  Error = true;
  CurMatch = 0;
  if ( SetExprLow(Expr) )
    Error = false;
  return !Error;
}

bool CRegExp::isok()
{
  return !Error;
}

bool CRegExp::SetExprLow(char *Expr)
{
  int Len = lstrlen(Expr);
  bool Ok = false;
  int s = 0, tmp;
  int EnterBr = 0, EnterGr = 0, EnterFg = 0;

  if ( Info )
  {
    Info->done();
    winDel(Info);
  }
  Info = winNew(SRegInfo, 1);
  Info->init();
  Exprn = winNew(int, Len);

  NoCase = false;
  Extend = false;
  if ( Expr[0] == '/' )
    s++;
  else
    return false;

  for (int i = Len; i > 0 && !Ok; i-- )
    if ( Expr[i] == '/')
    {
      Len = i-s;
      Ok = true;
      for (int j = i + 1; Expr[j]; j++)
      {
        if (Expr[j] == 'i')
          NoCase = true;
        if (Expr[j] == 'x')
          Extend = true;
      }
    }
  if (!Ok)
    return false;
  int pos = 0;
  for (int j = 0; j < Len ; j++, pos++ )
  {
    if (Extend && Expr[j+s] == ' ')
    {
      pos--;
      continue;
    }
    Exprn[pos] = (int)(unsigned char)Expr[j+s];
    if ( Expr[j+s] == BackSlash )
    {
      switch (Expr[j+s+1])
      {
        case 'd':
          Exprn[pos] = ReDigit;
          break;
        case 'D':
          Exprn[pos] = ReNDigit;
          break;
        case 'w':
          Exprn[pos] = ReWordSymb;
          break;
        case 'W':
          Exprn[pos] = ReNWordSymb;
          break;
        case 's':
          Exprn[pos] = ReWSpace;
          break;
        case 'S':
          Exprn[pos] = ReNWSpace;
          break;
        case 'u':
          Exprn[pos] = ReUCase;
          break;
        case 'l':
          Exprn[pos] = ReNUCase;
          break;
        case 't':
          Exprn[pos] = '\t';
          break;
        case 'n':
          Exprn[pos] = '\n';
          break;
        case 'r':
          Exprn[pos] = '\r';
          break;
        case 'b':
          Exprn[pos] = ReWBound;
          break;
        case 'B':
          Exprn[pos] = ReNWBound;
          break;
        case 'c':
          Exprn[pos] = RePreNW;
          break;
        case 'm':
          Exprn[pos] = ReStart;
          break;
        case 'M':
          Exprn[pos] = ReEnd;
          break;
        case 'x':
          tmp = crt::toupper(Expr[j+s+2])-0x30;
          tmp = (tmp>9?tmp-7:tmp)<<4;
          tmp += (crt::toupper(Expr[j+s+3])-0x30)>9?crt::toupper(Expr[j+s+3])-0x37:(crt::toupper(Expr[j+s+3])-0x30);
          Exprn[pos] = tmp;
          j+=2;
          break;
        case 'y':
          tmp = Expr[j+s+2] - 0x30;
          if ( tmp >= 0 && tmp <= 9 )
          {
            if ( tmp == 1 )
            {
              tmp = 10 + Expr[j+s+3] - 0x30;
              if (tmp >= 10 && tmp <= 19) j++;
              else tmp = 1;
            }
            Exprn[pos] = ReBkTrace + tmp;
            j++;
            break;
          }
        default:
          tmp = Expr[j+s+1] - 0x30;
          if ( tmp >= 0 && tmp <= 9 )
          {
            if ( tmp == 1 )
            {
              tmp = 10 + Expr[j+s+2] - 0x30;
              if (tmp >= 10 && tmp <= 19) j++;
              else tmp = 1;
            }
            Exprn[pos] = ReBkBrack + tmp;
            break;
          }
          else
            Exprn[pos] = Expr[j+s+1];
          break;
      }
      j++;
      continue;
    }
    if ( Expr[j+s] == ']' )
    {
      Exprn[pos] = ReEnumE;
      if ( EnterFg || !EnterGr )
        return false;
      EnterGr--;
    }
    if ( Expr[j+s] == '-' && EnterGr )
      Exprn[pos] = ReFrToEnum;
    if ( EnterGr )
      continue;
    if ( Expr[j+s] == '[' && Expr[j+s+1] == '^' )
    {
      Exprn[pos] = ReNEnumS;
      if ( EnterFg )
        return false;
      EnterGr++;
      j++;
      continue;
    }
    if ( Expr[j+s] == '*' && Expr[j+s+1] == '?' )
    {
      Exprn[pos] = ReNGMul;
      j++;
      continue;
    }
    if ( Expr[j+s] == '+' && Expr[j+s+1] == '?' )
    {
      Exprn[pos] = ReNGPlus;
      j++;
      continue;
    }
    if ( Expr[j+s] == '?' && Expr[j+s+1] == '?' )
    {
      Exprn[pos] = ReNGQuest;
      j++;
      continue;
    }
    if ( Expr[j+s] == '?' && Expr[j+s+1] == '#' && Expr[j+s+2]>='0' && Expr[j+s+2]<='9' )
    {
      Exprn[pos] = ReBehind+Expr[j+s+2]-0x30;
      j += 2;
      continue;
    }
    if ( Expr[j+s] == '?' && Expr[j+s+1] == '~' && Expr[j+s+2]>='0' && Expr[j+s+2]<='9')
    {
      Exprn[pos] = ReNBehind+Expr[j+s+2]-0x30;
      j += 2;
      continue;
    }
    if ( Expr[j+s] == '?' && Expr[j+s+1] == '=' )
    {
      Exprn[pos] = ReAhead;
      j++;
      continue;
    }
    if ( Expr[j+s] == '?' && Expr[j+s+1] == '!' )
    {
      Exprn[pos] = ReNAhead;
      j++;
      continue;
    }
    if ( Expr[j+s] == '(' )
    {
      Exprn[pos] = ReLBrack;
      if ( EnterFg )
        return false;
      EnterBr++;
    }
    if ( Expr[j+s] == ')' )
    {
      Exprn[pos] = ReRBrack;
      if ( !EnterBr || EnterFg )
        return false;
      EnterBr--;
    }
    if ( Expr[j+s] == '[' )
    {
      Exprn[pos] = ReEnumS;
      if ( EnterFg )
        return false;
      EnterGr++;
    }
    if ( Expr[j+s] == '{' )
    {
      Exprn[pos] = ReRangeS;
      if ( EnterFg )
        return false;
      EnterFg++;
    }
    if (Expr[j+s] == '}' && Expr[j+s+1] == '?')
    {
      Exprn[pos] = ReNGRangeE;
      if ( !EnterFg )
        return false;
      EnterFg--;
      j++;
      continue;
    }
    if ( Expr[j+s] == '}' )
    {
      Exprn[pos] = ReRangeE;
      if ( !EnterFg )
        return false;
      EnterFg--;
    }
    if (Expr[j+s] == '^') Exprn[pos] = ReSoL;
    if (Expr[j+s] == '$') Exprn[pos] = ReEoL;
    if (Expr[j+s] == '.') Exprn[pos] = ReAnyChr;
    if (Expr[j+s] == '*') Exprn[pos] = ReMul;
    if (Expr[j+s] == '+') Exprn[pos] = RePlus;
    if (Expr[j+s] == '?') Exprn[pos] = ReQuest;
    if (Expr[j+s] == '|') Exprn[pos] = ReOr;
  }
  if ( EnterGr || EnterBr || EnterFg )
    return false;
  Info->Op = ReBrackets;
  Info->un.Param = winNew(SRegInfo, 1);
  Info->un.Param->init();
  Info->s = (short)(CurMatch++);
  if (!SetStructs(Info->un.Param,0,(int)pos) )
    return false;
  Optimize();
  winDel(Exprn);
  return true;
}

void CRegExp::Optimize()
{
  PRegInfo Next = Info;
  FirstChar = 0;
  while ( Next )
  {
    if ( Next->Op == ReBrackets || Next->Op == RePlus  || Next->Op == ReNGPlus )
    {
      Next = Next->un.Param;
      continue;
    }
    if ( Next->Op == ReSymb )
    {
      if ( Next->un.Symb & 0xFF00 &&  Next->un.Symb != ReSoL && Next->un.Symb != ReWBound )
        break;
      FirstChar = Next->un.Symb;
      break;
    }
    break;
  }
}

bool CRegExp::SetStructs(PRegInfo &re, int start, int end)
{
  PRegInfo Next, Prev, Prev2;
  int comma, st, en, ng, i, j, k, EnterBr;
  bool Add;
  if ( end - start < 0 )
    return false;
  Next = re;
  for (i = start; i < end; i++){
    Add = false;
    // Ops
    if ( Exprn[i] > ReBlockOps && Exprn[i] < ReSymbolOps )
    {
      Next->un.Param = 0;
      Next->Op = (EOps)Exprn[i];
      Add = true;
    }
    // {n,m}
    if ( Exprn[i] == ReRangeS )
    {
      st = i;
      en = -1;
      comma = -1;
      ng = 0;
      for ( j = i ; j < end ; j++ )
      {
        if ( Exprn[j] == ReNGRangeE )
        {
          en = j;
          ng = 1;
          break;
        }
        if ( Exprn[j] == ReRangeE )
        {
          en = j;
          break;
        }
        if ( (char)Exprn[j] == ',' )
          comma = j;
      }
      if ( en == -1 )
        return false;
      if ( comma == -1 )
        comma = en;
      Next->s = (char)GetNumber(Exprn, st+1, comma);
      if ( comma != en )
        Next->e = (char)GetNumber(Exprn, comma+1, en);
      else
        Next->e = Next->s;
      Next->un.Param = 0;
      Next->Op = ng ? ReNGRangeNM : ReRangeNM;
      if ( en-comma == 1 )
      {
        Next->e = -1;
        Next->Op = ng ? ReNGRangeN : ReRangeN;
      }
      i = j;
      Add = true;
    }
    // [] [^]
    if ( Exprn[i] == ReEnumS || Exprn[i] == ReNEnumS )
    {
      Next->Op = (Exprn[i] == ReEnumS) ? ReEnum : ReNEnum;
      for ( j = i+1 ; j < end ; j++ )
        if ( Exprn[j] == ReEnumE )
          break;
      if ( j == end )
        return false;
      Next->un.ChrClass = winNew(SCharData, 1);
      ZeroMemory(Next->un.ChrClass, 32);
      for ( j = i+1 ; Exprn[j] != ReEnumE ; j++ )
      {
        if ( Exprn[j+1] == ReFrToEnum )
        {
          for ( i = (Exprn[j] & 0xFF) ; i < (Exprn[j+2] & 0xFF) ; i++)
            Next->un.ChrClass->SetBit((unsigned char)(i & 0xFF));
          j++;
          continue;
        }
        switch(Exprn[j])
        {
          case ReDigit:
            for ( k = 0x30 ; k < 0x40 ; k++ )
              if ( IsDigit((char)k) )
                Next->un.ChrClass->SetBit((unsigned char)k);
            break;
          case ReNDigit:
            for ( k = 0x30 ; k < 0x40 ; k++)
              if ( !IsDigit((char)k) )
                Next->un.ChrClass->SetBit((unsigned char)k);
            break;
          case ReWordSymb:
            for ( k = 0 ; k < 256 ; k++)
              if ( IsWord((char)k) )
                Next->un.ChrClass->SetBit((unsigned char)k);
            break;
          case ReNWordSymb:
            for ( k = 0 ; k < 256 ; k++ )
              if ( !IsWord((char)k) )
                Next->un.ChrClass->SetBit((unsigned char)k);
            break;
          case ReWSpace:
            Next->un.ChrClass->SetBit((unsigned char)0x20);
            break;
          case ReNWSpace:
            FillMemory(Next->un.ChrClass->IArr, 0xFF, 32);
            Next->un.ChrClass->ClearBit((unsigned char)0x20);
            break;
          default:
            if ( !(Exprn[j] & 0xFF00) )
              Next->un.ChrClass->SetBit((unsigned char)(Exprn[j] & 0xFF));
            break;
        }
      }
      Add = true;
      i=j;
    }
    // ( ... )
    if ( Exprn[i] == ReLBrack )
    {
      EnterBr = 1;
      for ( j = i+1 ; j < end ; j++ )
      {
        if ( Exprn[j] == ReLBrack )
          EnterBr++;
        if ( Exprn[j] == ReRBrack )
          EnterBr--;
        if ( !EnterBr )
          break;
      }
      if ( EnterBr )
        return false;
      Next->Op = ReBrackets;
      Next->un.Param = winNew(SRegInfo, 1);
      Next->un.Param->init();
      Next->un.Param->Parent = Next;
      Next->s = (short)(CurMatch++);
      if ( CurMatch > MatchesNum )
        CurMatch = MatchesNum;
      if ( !SetStructs(Next->un.Param, i+1, j) )
        return false;
      Add = true;
      i = j;
    }
    if ( (Exprn[i]&0xFF00) == ReBkTrace )
    {
      Next->Op = ReBkTrace;
      Next->un.Symb = Exprn[i] & 0xFF;
      Add = true;
    }
    if ( (Exprn[i]&0xFF00) == ReBkBrack )
    {
      Next->Op = ReBkBrack;
      Next->un.Symb = Exprn[i] & 0xFF;
      Add = true;
    }
    if ( (Exprn[i]&0xFF00) == ReBehind )
    {
      Next->Op = ReBehind;
      Next->s = (short)(Exprn[i] & 0xFF);
      Add = true;
    }
    if ( (Exprn[i]&0xFF00) == ReNBehind )
    {
      Next->Op = ReNBehind;
      Next->s = (short)(Exprn[i] & 0xFF);
      Add = true;
    }
    // Chars
    if ( Exprn[i] >= ReAnyChr && Exprn[i] < ReTemp || Exprn[i] < 0x100 )
    {
      Next->Op = ReSymb;
      Next->un.Symb = Exprn[i];
      Add = true;
    }
    // Next
    if ( Add && i != end-1 )
    {
      Next->Next = winNew(SRegInfo, 1);
      Next->Next->init();
      Next->Next->Parent = Next->Parent;
      Next = Next->Next;
    }
  }
  Next = re;
  Prev = Prev2 = 0;
  while( Next )
  {
    if ( Next->Op > ReBlockOps && Next->Op < ReSymbolOps )
    {
      if ( !Prev )
        return false;
      if ( !Prev2 )
        re = Next;
      else
        Prev2->Next = Next;
      //if (Prev->Op > ReBlockOps && Prev->Op < ReSymbolOps) return false;
      Prev->Parent = Next;
      Prev->Next = 0;
      Next->un.Param = Prev;
      Prev = Prev2;
    }
    Prev2 = Prev;
    Prev = Next;
    Next = Next->Next;
  }
  return true;
}

/////////////////////////////////////////////////////////////////
/////////////////////////  Parsing  /////////////////////////////
/////////////////////////////////////////////////////////////////

bool CRegExp::CheckSymb(int Symb, bool Inc)
{
  bool Res;
  switch(Symb)
  {
    case ReAnyChr:
      if ( toParse >= End )
        return false;
      if ( Inc ) toParse++;
      return true;
    case ReSoL:
      return Start == toParse;
    case ReEoL:
      return End == toParse;
    case ReDigit:
      if ( toParse >= End )
        return false;
      Res = (*toParse >= 0x30 && *toParse <= 0x39);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReNDigit:
      if ( toParse >= End )
        return false;
      Res = !(*toParse >= 0x30 && *toParse <= 0x39);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReWordSymb:
      if ( toParse >= End )
        return false;
      Res = IsWord(*toParse);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReNWordSymb:
      if ( toParse >= End )
        return false;
      Res = !IsWord(*toParse);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReWSpace:
      if ( toParse >= End )
        return false;
      Res = (*toParse == 0x20 || *toParse == '\t');
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReNWSpace:
      if ( toParse >= End )
        return false;
      Res = !(*toParse == 0x20 || *toParse == '\t');
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReUCase:
      if ( toParse >= End )
        return false;
      Res = IsUpperCase(*toParse);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReNUCase:
      if ( toParse >= End )
        return false;
      Res = IsLowerCase(*toParse);
      if ( Res && Inc )
        toParse++;
      return Res;
    case ReWBound:
      if ( toParse >= End )
        return true;
      return IsWord(*toParse) && (toParse == Start || !IsWord(*(toParse-1)));
    case ReNWBound:
      if ( toParse >= End )
        return true;
      return !IsWord(*toParse) && IsWord(*(toParse-1));
    case RePreNW:
      if ( toParse >= End )
        return true;
      return (toParse == Start || !IsWord(*(toParse-1)));
    case ReStart:
      Matches->s[0] = (short)(toParse-Start);
      return true;
    case ReEnd:
      Matches->e[0] = (short)(toParse-Start);
      return true;
    default:
      if ( (Symb & 0xFF00) || toParse >= End )
        return false;
      if ( NoCase )
      {
        if ( LowCase(*toParse) != LowCase((char)(Symb & 0xFF)) )
          return false;
      }
      else if (*toParse != (char)(Symb&0xFF))
        return false;
      if ( Inc )
        toParse++;
      return true;
  }
}

bool CRegExp::LowParseRe(PRegInfo &Next)
{
  PRegInfo OrNext;
  int i, match, sv;
  char *tStr;

  switch( Next->Op )
  {
    case ReSymb:
      if ( !CheckSymb(Next->un.Symb, true) )
        return false;
      break;
    case ReEmpty:
      break;
    case ReBkTrace:
      if ( !BkStr | !BkTrace )
        return false;
      sv = Next->un.Symb;
      tStr = toParse;
      for ( i = BkTrace->s[sv] ; i < BkTrace->e[sv] ; i++)
      {
        if ( *tStr != BkStr[i] || End == tStr )
          return false;
        tStr++;
      }
      toParse = tStr;
      break;
    case ReBkBrack:
      sv = Next->un.Symb;
      tStr = toParse;
      if ( Matches->s[sv] == -1 || Matches->e[sv] == -1 )
        return false;
      for ( i = Matches->s[sv] ; i < Matches->e[sv] ; i++ )
      {
        if ( *tStr != Start[i] || End == tStr )
          return false;
        tStr++;
      }
      toParse = tStr;
      break;
    case ReBehind:
      sv = Next->s;
      tStr = toParse;
      toParse -= sv;
      if ( !LowParse(Next->un.Param) )
        return false;
      toParse = tStr;
      break;
    case ReNBehind:
      sv = Next->s;
      tStr = toParse;
      toParse -= sv;
      if ( LowParse(Next->un.Param) )
        return false;
      toParse = tStr;
      break;
    case ReAhead:
      tStr = toParse;
      if ( !LowParse(Next->un.Param) )
        return false;
      toParse = tStr;
      break;
    case ReNAhead:
      tStr = toParse;
      if ( LowParse(Next->un.Param) )
        return false;
      toParse = tStr;
      break;
    case ReEnum:
      if ( toParse >= End )
        return false;
      if ( !Next->un.ChrClass->GetBit(*toParse) )
        return false;
      toParse++;
      break;
    case ReNEnum:
      if ( toParse >= End )
        return false;
      if ( Next->un.ChrClass->GetBit(*toParse) )
        return false;
      toParse++;
      break;
    case ReBrackets:
      match = Next->s;
      sv = (int)(toParse-Start);
      tStr = toParse;
      if ( LowParse(Next->un.Param) )
      {
        if ( match || (Matches->s[match] == -1) )
          Matches->s[match] = (short)sv;
        if ( match || (Matches->e[match] == -1) )
          Matches->e[match] = (short)(toParse-Start);
        return true;
      }
      toParse = tStr;
      return false;
    case ReMul:
      tStr = toParse;
      while ( LowParse(Next->un.Param) )
        ;
      while( !LowCheckNext(Next) && tStr < toParse )
        toParse--;
      break;
    case ReNGMul:
      do
      {
        if ( LowCheckNext(Next) )
          break;
      } while (LowParse(Next->un.Param));
      break;
    case RePlus:
      tStr = toParse;
      match = false;
      while ( LowParse(Next->un.Param) )
        match = true;
      if ( !match )
        return false;
      while ( !LowCheckNext(Next) && tStr < toParse )
        toParse--;
      break;
    case ReNGPlus:
      if ( !LowParse(Next->un.Param) )
        return false;
      do
      {
        if ( LowCheckNext(Next) )
          break;
      } while (LowParse(Next->un.Param));
      break;
    case ReQuest:
      LowParse(Next->un.Param);
      break;
    case ReNGQuest:
      if ( LowCheckNext(Next) )
        break;
      if ( !LowParse(Next->un.Param) )
        return false;
      break;
    case ReOr:
      OrNext = Next;
      // tStr = toParse;
      if ( LowParse(Next->un.Param) )
      {
        while ( OrNext && OrNext->Op == ReOr )
          OrNext = OrNext->Next;
        /*if (!LowCheckNext(OrNext)){
          toParse = tStr;
          OrNext = Next;
        }*/
      }
      Next = OrNext;
      break;
    case ReRangeN:
      tStr = toParse;
      i = 0;
      while ( LowParse(Next->un.Param) )
        i++;
      do
      {
        if ( i < Next->s )
        {
          toParse = tStr;
          return false;
        }
        i--;
      } while ( !LowCheckNext(Next) && tStr < toParse-- );
      break;
    case ReNGRangeN:
      tStr = toParse;
      i = 0;
      while ( LowParse(Next->un.Param) )
      {
        i++;
        if ( i >= Next->s && LowCheckNext(Next) )
          break;
      }
      if ( i < Next->s )
      {
        toParse = tStr;
        return false;
      }
      break;
    case ReRangeNM:
      tStr = toParse;
      i = 0;
      while ( i < Next->s && LowParse(Next->un.Param) )
        i++;
      if ( i < Next->s )
      {
        toParse = tStr;
        return false;
      }
      while ( i < Next->e && LowParse(Next->un.Param) )
        i++;
      while(!LowCheckNext(Next))
      {
        i--;
        toParse--;
        if ( i < Next->s )
        {
          toParse = tStr;
          return false;
        }
      }
      break;
    case ReNGRangeNM:
      tStr = toParse;
      i = 0;
      while ( i < Next->s && LowParse(Next->un.Param) )
        i++;
      if ( i < Next->s )
      {
        toParse = tStr;
        return false;
      }
      while( !LowCheckNext(Next) )
      {
        i++;
        if ( !LowParse(Next->un.Param) || i > Next->e )
        {
          toParse = tStr;
          return false;
        }
      }
      break;
  }
  return true;
}

bool CRegExp::LowCheckNext(PRegInfo Re)
{
  PRegInfo Next;
  char *tmp = toParse;
  Next = Re;
  do
  {
    if ( Next && Next->Op == ReOr )
      while ( Next && Next->Op == ReOr )
        Next = Next->Next;
    if ( Next->Next && !LowParse(Next->Next) )
    {
      toParse = tmp;
      Ok = false;
      return false;
    }
    Next = Next->Parent;
  } while ( Next );
  toParse = tmp;
  if ( Ok != false )
    Ok = true;
  return true;
}

bool CRegExp::LowParse(PRegInfo Re)
{
  while( Re && toParse <= End )
  {
    if ( !LowParseRe(Re) )
      return false;
    if ( Re )
      Re = Re->Next;
  }
  return true;
}

bool CRegExp::QuickCheck()
{
  if ( !NoMoves || !FirstChar )
    return true;
  switch ( FirstChar )
  {
    case ReSoL:
      if ( toParse != Start )
        return false;
      return true;
    case ReWBound:
      return IsWord((char)*toParse) && (toParse == Start || !IsWord((char)*(toParse-1)));
    default:
      if ( NoCase && LowCase(*toParse) != LowCase((char)FirstChar) )
        return false;
      if ( !NoCase && *toParse != (char)FirstChar )
        return false;
      return true;
  }
}

bool CRegExp::ParseRe(char *Str)
{
  if ( Error )
    return false;
  toParse = Str;
  if ( !QuickCheck() )
    return false;
  for ( int i = 0 ; i < MatchesNum ; i++ )
    Matches->s[i] = Matches->e[i] = -1;
  Matches->CurMatch = CurMatch;
  Ok = -1;
  do
  {
    if ( !LowParse(Info) )
    {
      if ( NoMoves )
        return false;
    }
    else
      return true;
    toParse = ++Str;
  } while ( toParse != End+1 );
  return false;
}

bool CRegExp::Parse(char *Str,char *Sol, char *Eol, PMatches Mtch, int Moves)
{
  if ( !this )
    return false;
  bool s = NoMoves;
  if ( Moves != -1 )
    NoMoves = Moves != 0;
  Start = Sol;
  End   = Eol;
  Matches = Mtch;
  bool r = ParseRe(Str);
  NoMoves = s;
  return r;
}

bool CRegExp::Parse(char *Str, PMatches Mtch)
{
  if ( !this )
    return false;
  Start = Str;
  End = Start+lstrlen(Start);
  Matches = Mtch;
  return ParseRe(Str);
}

bool CRegExp::SetNoMoves(bool Moves)
{
  NoMoves = Moves;
  return true;
}

bool CRegExp::SetBkTrace(char *Str,PMatches Trace)
{
  BkTrace = Trace;
  BkStr = Str;
  return true;
}
