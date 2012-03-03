#include "string.hpp"

#define MEMORY_STEP (64)

void string::init(void)
{
  data=NULL;
  default_data[0]=0;
  current_size=0;
  actual_size=0;
  hash_start=0;
  heap=GetProcessHeap();
}

void string::copy(const string& Value)
{
  if(enlarge(Value.current_size))
  {
    crt::lmemcpy(data,Value.data,Value.current_size);
    current_size=Value.current_size;
    hash_start=Value.hash_start;
  }
}

void string::copy(const unsigned char *Value)
{
  int len=lstrlen((const char *)Value)+1;
  if(enlarge(len))
  {
    crt::lmemcpy(data,(void*)Value,len);
    current_size=len;
    hash_start=0;
  }
}

void string::copy(const unsigned char *Value, crt::size_t size)
{
  if(enlarge(size+1))
  {
    crt::lmemcpy(data,(void*)Value,size);
    data[size]=0;
    current_size=size+1;
    hash_start=0;
  }
}

bool string::enlarge(crt::size_t size)
{
  crt::size_t new_actual_size=(size/MEMORY_STEP+((size%MEMORY_STEP)?1:0))*MEMORY_STEP;
  if(actual_size>=new_actual_size) return true;
  void *new_data;
  if(data)
    new_data=HeapReAlloc(heap,HEAP_ZERO_MEMORY,data,new_actual_size);
  else
    new_data=HeapAlloc(heap,HEAP_ZERO_MEMORY,new_actual_size);
  if(new_data)
  {
    data=(unsigned char *)new_data;
    actual_size=new_actual_size;
    return true;
  }
  return false;
}

string::string()
{
  init();
}

string::string(const unsigned char *Value)
{
  init();
  copy(Value);
}

string::string(const unsigned char *Value,crt::size_t size)
{
  init();
  copy(Value,size);
}

string::string(const string& Value)
{
  init();
  copy(Value);
}

string::~string()
{
  if(data) HeapFree(heap,0,data);
}

string &string::operator=(const string& Value)
{
  if(this!=&Value)
    copy(Value);
  return *this;
}

string &string::operator=(const unsigned char *Value)
{
  copy(Value);
  return *this;
}

string &string::operator()(const unsigned char *Value,crt::size_t size)
{
  copy(Value,size);
  return *this;
}

string::operator const unsigned char *() const
{
  if(data) return data;
  return default_data;
}

unsigned char &string::operator[](crt::size_t index)
{
  if(index>=current_size) return default_data[0];
  return data[index];
}

crt::size_t string::length(void) const
{
  return current_size?current_size-1:0;
}

//void string::reverse(void)
//{
//  if(data) _strrev((char *)data);
//}

unsigned long string::hash(void)
{
  return ((((*this)[hash_start+0])*256+(*this)[hash_start+1])*256+(*this)[hash_start+2])*256+(*this)[hash_start+3];
}

void string::hash(crt::size_t pos)
{
  hash_start=pos;
}

unsigned char *string::get(void)
{
  if(data) return data;
  return default_data;
}

void string::clear(void)
{
  if(data&&actual_size)
  {
    current_size=1;
    data[0]=0;
  }
}

string& string::operator+=(unsigned char Value)
{
  if(enlarge(current_size+(current_size?1:2)))
  {
    if(!current_size) current_size++;
    data[current_size-1]=Value;
    data[current_size++]=0;
  }
  return *this;
}

string& string::operator+=(const char* Value)
{
  int value_len(lstrlen(Value));
  if(enlarge(current_size+(current_size ? value_len : value_len + 1)))
  {
    int old_current_size(current_size);
    lstrcat((char*)data, (char*)Value);
    current_size += value_len;
  }
  return *this;
}

int operator==(const string& x, const string& y)
{
  if(x.length()==y.length()) return crt::lmemcmp((const unsigned char *)x, (const unsigned char *)y, x.length());
  else return x.length()-y.length();
}
