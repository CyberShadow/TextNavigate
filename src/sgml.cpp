//sgml.cpp

//
//  Copyright (c) Cail Lomecb (Igor Ruskih) 1999-2001 <ruiv@uic.nnov.ru>
//  You can use, modify, distribute this code or any other part
//  of this program in sources or in binaries only according
//  to License (see /doc/license.txt for more information).
//

// creates object tree structure on html/xml files

#include "sgml.h"
#include "TextNavigate.h"

bool inline isspace(char c)
{
  if (c == 0x20 || c == '\t' || c == '\r' || c == '\n') return true;
  return false;
};

bool GetNumber(char *str, int &res, int s, int e)
{
  int r = 1, num = 0;
  if ( e < s )
    return false;
  for ( int i = e-1 ; i >= s ; i-- )
  {
    if ( str[i] > '9' || str[i] < '0')
      return false;
    num += (str[i] - 0x30)*r;
    r *= 10;
  }
  res = num;
  return true;
}

CSgmlEl::CSgmlEl()
{
  eparent= 0;
  enext  = 0;
  eprev  = 0;
  echild = 0;
  chnum  = 0;
  type   = EBASEEL;

  name[0] = 0;
  content = 0;
  contentsz = 0;
  parnum = 0;
}

CSgmlEl::~CSgmlEl()
{
  if (type == EBASEEL) enext->destroylevel();
  if (echild) echild->destroylevel();
  // if (name) delete[] name;
  if (content) delete content;
  for (int i=0; i < parnum; i++)
  {
    if (params[i][0]) delete params[i][0];
    if (params[i][1]) delete params[i][1];
  }
}

PSgmlEl CSgmlEl::createnew(ElType type, PSgmlEl parent, PSgmlEl after)
{
  PSgmlEl El = new CSgmlEl;
  El->type = type;
  if (parent){
    El->enext = parent->echild;
    El->eparent = parent;
    if (parent->echild) parent->echild->eprev = El;
    parent->echild = El;
    parent->chnum++;
    parent->type = EBLOCKEDEL;
  } else
    if (after) after->insert(El);
  return El;
}

bool CSgmlEl::init()
{
  return true;
}

bool CSgmlEl::parse(const char *src,int sz)
{
PSgmlEl Child, Parent, Next; // = 0;
int i, j, lins, line;
int ls, le, rs, re, empty;

  // start object - base
  type = EBASEEL;
  Next = this;

  lins = line = 0;
  for (i = 0; i < sz; i++){
    if (i >= sz) continue;

    // comments
//    if ( *((int*)(src+i)) == '--!<' && i+4 < sz){
    if ( src[i] == '<' && src[i+1] == '!' && src[i+2] == '-' && src[i+3] == '-' && i+4 < sz){
      i += 4;
      while((src[i] != '-' || src[i+1] != '-' || src[i+2] != '>') && i+3 < sz) i++;
      i+=3;
    };
    line = i;

    if ( src[i] == '<' || i >= sz-1){
      while(line > lins){
        // linear
        j = lins;
        while(isspace(src[j]) && j < i){
          j++;
        };
        if(j == i) break; // empty text
        Child = createnew(EPLAINEL,0,Next);
        Child->init();
        Child->setcontent(src + lins, i - lins);
        Next = Child;
        break;
      };
      if (i == sz-1) continue;
      // start or single tag
      if (src[i+1] != '/'){
        Child = createnew(ESINGLEEL,NULL,Next);
        Next  = Child;
        Child->init();
        j = i+1;
        while (src[i] != '>' && !isspace(src[i]) && i < sz) i++;
        // Child->name = new char[i-j+1];
        if (i-j > MAXTAG) i = j + MAXTAG - 1;
        lstrcpynA(Child->name, src+j, i-j + 1);
        Child->name[i-j] = 0;
        // parameters
        Child->parnum = 0;
        while(src[i] != '>' && Child->parnum < MAXPARAMS && i < sz){
          ls = i;
          while (isspace(src[ls]) && ls < sz) ls++;
          le = ls;
          while (!isspace(src[le]) && src[le]!='>' && src[le]!='=' && le < sz) le++;
          rs = le;
          while (isspace(src[rs]) && rs < sz) rs++;
          empty = 1;
          if (src[rs] == '='){
            empty = 0;
            rs++;
            while (isspace(src[rs]) && rs < sz) rs++;
            re = rs;
            if (src[re] == '"'){
              while(src[++re] != '"' && re < sz);
              rs++;
              i = re+1;
            }
            else if (src[re] == '\'')
            {
              while(src[++re] != '\'' && re < sz);
              rs++;
              i = re+1;
            }
            else
            {
              while (!isspace(src[re]) && src[re] != '>' && re < sz) re++;
              i = re;
            };
          }
          else
            i = re = rs;

          if (ls == le) continue;
          if (rs == re && empty){
            rs = ls;
            re = le;
          };
          int pn = Child->parnum;
          Child->params[pn][0] = new char[le-ls+1];
          lstrcpynA(Child->params[pn][0], src+ls, le-ls + 1);
          Child->params[pn][0][le-ls] = 0;
          Child->params[pn][1] = new char[re-rs+1];
          lstrcpynA(Child->params[pn][1], src+rs, re-rs + 1);
          Child->params[pn][1][re-rs] = 0;
          Child->parnum++;
          substquote(Child->params[pn], "&lt;", '<');
          substquote(Child->params[pn], "&gt;", '>');
          substquote(Child->params[pn], "&amp;", '&');
          substquote(Child->params[pn], "&quot;", '"');
        };
        lins = i+1;
      } else {  // end tag
        j = i+2;
        i+=2;
        while (src[i] != '>' && !isspace(src[i]) && i < sz) i++;
        int cn = 0;
        for(Parent = Next; Parent->eprev; Parent = Parent->eprev, cn++){
          if(!*Parent->name) continue;
          int len = lstrlen(Parent->name);
          if (len != i-j) continue;
          if (Parent->type != ESINGLEEL ||
            //strnicmp((char*)src+j, Parent->name, len)) continue;
            FSF.LStrnicmp((char*)src+j, Parent->name, (int)len)) continue;

          break;
        };
        if(Parent && Parent->eprev){
          Parent->echild = Parent->enext;
          Parent->chnum = cn;
          Parent->type = EBLOCKEDEL;
          Child = Parent->echild;
          if (Child) Child->eprev = 0;
          while(Child){
            Child->eparent = Parent;
            Child = Child->enext;
          };
          Parent->enext = 0;
          Next = Parent;
        };
        while(src[i] != '>' && i < sz) i++;
        lins = i+1;
      };
    };
  };
////
  return true;
};

void CSgmlEl::substquote(TParams par, char *sstr, char c)
{
 int len = (int)lstrlen(sstr);
 int plen = (int)lstrlen(par[1]);

  for (int i = 0; i <= plen-len; i++)
    if (!crt::lstrcmpn(par[1]+i, sstr, len)){
      par[1][i] = c;
      for(int j = i+1; j <= plen-len+1; j++)
        par[1][j] = par[1][j+len-1];
      plen -= len-1;
    };
};

bool CSgmlEl::setcontent(const char *src,int sz)
{
  content = new char[sz + 1];
  crt::lmemmove(content,src,sz);
  content[sz]=0;
  contentsz = sz;
  return true;
};

void CSgmlEl::insert(PSgmlEl El)
{
  El->eprev = this;
  El->enext = this->enext;
  El->eparent = this->eparent;
  if (this->enext) this->enext->eprev = El;
  this->enext = El;
};

// recursive deletion
void CSgmlEl::destroylevel()
{
  if (enext) enext->destroylevel();
  delete this;
};

PSgmlEl CSgmlEl::parent()
{
  return eparent;
};

PSgmlEl CSgmlEl::next()
{
  return enext;
};

PSgmlEl CSgmlEl::prev()
{
  return eprev;
};

PSgmlEl CSgmlEl::child()
{
  return echild;
};

ElType  CSgmlEl::gettype()
{
  return type;
};

char *CSgmlEl::getname()
{
  if (!*name) return NULL;
  return name;
};

char *CSgmlEl::getcontent()
{
  return content;
};

int CSgmlEl::getcontentsize()
{
  return contentsz;
};

char* CSgmlEl::GetParam(int no)
{
  if (no >= parnum) return 0;
  return params[no][0];
};

char* CSgmlEl::GetChrParam(const char* par)
{
  for (int i=0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0])){
      return params[i][1];
    };
  return 0;
};

bool CSgmlEl::GetIntParam(const char* par, int& result)
{
int res = 0;
  for (int i=0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0]))
    {
      //bool b = get_number(params[i][1],&res);
      bool b = GetNumber(params[i][1], res, 0, (int)lstrlen(params[i][1]));

      result = res;
      if (!b) result = 0;
      return b;
    };
  return false;
};
/*
bool CSgmlEl::GetFltParam(char *par, double *result)
{
double res;
  for (int i = 0; i < parnum; i++)
    if (!stricmp(par,params[i][0])){
      bool b = get_number(params[i][1],&res);
      *result = (double)res;
      if (!b) *result = 0;
      return b;
    };
  return false;
};
*/
PSgmlEl CSgmlEl::search(const char* TagName)
{
PSgmlEl Next = this->enext;
  while(Next){
    if (!FSF.LStricmp(TagName,Next->name)) return Next;
    Next = Next->enext;
  };
  return Next;
};

PSgmlEl CSgmlEl::enumchilds(int no)
{
PSgmlEl El = this->echild;
  while(no && El){
    El = El->enext;
    no--;
  };
  return El;
};

PSgmlEl CSgmlEl::fprev()
{
PSgmlEl El = this;
  if (!El->eprev) return El->eparent;
  if (El->eprev->echild)
    return El->eprev->echild->flast();
  return El->eprev;
};

PSgmlEl CSgmlEl::fnext()
{
PSgmlEl El = this;
  if (El->echild) return El->echild;
  while(!El->enext){
    El = El->eparent;
    if (!El) return 0;
  };
  return El->enext;
};

PSgmlEl CSgmlEl::ffirst()
{
PSgmlEl Prev = this;
  while(Prev){
    if (!Prev->eprev) return Prev;
    Prev = Prev->eprev;
  };
  return Prev;
};

PSgmlEl CSgmlEl::flast()
{
PSgmlEl Nxt = this;
  while(Nxt->enext || Nxt->echild){
    if (Nxt->enext){
      Nxt = Nxt->enext;
      continue;
    };
    if (Nxt->echild){
      Nxt = Nxt->echild;
      continue;
    };
  };
  return Nxt;
}
