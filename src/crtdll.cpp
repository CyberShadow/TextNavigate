#include "crtdll.h"
#include "TextNavigate.h"

#ifdef __cplusplus

void *__cdecl operator new(crt::size_t size)
{
  return crt::malloc(size); // winNew(char, size);
}

void *__cdecl operator new[](crt::size_t size)
{
  return ::operator new(size); //crt::malloc(size); //winNew(char, size);
}

void __cdecl operator delete(void *block)
{
  crt::free(block);
}

void __cdecl operator delete[](void *ptr)
{
  ::operator delete(ptr);
}

void cdecl _pure_error_()
{
}

#endif

#ifdef __cplusplus
namespace crt {
extern "C" {
#endif

// stdio.h
void* malloc(int size)
{
  return winNew(char, size > MIN_SIZE ? size : MIN_SIZE);
}

void free(void* ptr)
{
  winDel(ptr);
}

// mem.h

void* memset(void *s, int c, int n)
{
  BYTE *dst = (BYTE *) s;

  while (n--)
  {
    *dst = (BYTE) (unsigned) c;
    ++dst;
  }
  return s;
}

void* lmemcpy(void* dst, void* src, int num)
{
  if (!dst || !src || num <= 0)
    return 0;
  if (dst == src)
    return dst;

  char* _dst = (char*)dst;
  char* _src = (char*)src;
  if (_src < _dst)
    for (int i = num - 1; i >= 0; i--)
      _dst[i] = _src[i];
  else
    for (int i = 0; i < num; i++)
      _dst[i] = _src[i];

  return dst;
}

int lmemcmp(const void* b1, const void* b2, int num)
{
  for (int i = 0; i < num; i++)
    if (((unsigned char*)b1)[i] > ((unsigned char*)b2)[i])
      return 1;
    else if (((unsigned char*)b1)[i] < ((unsigned char*)b2)[i])
      return -1;
  return 0;
}

void* lmemmove(void* dest, const void* src, int n)
{
  if (n && (dest != src))
  {
    BYTE *s = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n);

    if (s)
    {
      lmemcpy(s, (void *)src, n);
      lmemcpy(dest, s, n);
      HeapFree(GetProcessHeap(), 0, (void *) s);
    }
  }
  return dest;
}

// string.h
char* lstrstr(const char* str1, const char* str2, bool case_sensitive)
{
  assert(str1 && str2);

  char _buf1[MAX_PATH];
  char _buf2[MAX_PATH];

  int l1 = lstrlen(str1);
  int l2 = lstrlen(str2);

  char* buf1 = (l2 + 1) > MAX_PATH ? (char*)malloc((l2 + 1) * sizeof(*buf1)) : _buf1;
  char* buf2 = (l2 + 1) > MAX_PATH ? (char*)malloc((l2 + 1) * sizeof(*buf2)) : _buf2;

  for (int i = 0; i <= l1 - l2; i++)
  {
    lstrcpyn(buf1, str1 + i, l2 + 1);
    lstrcpyn(buf2, str2, l2 + 1);
    if (case_sensitive && !lstrcmp(buf1, buf2) ||
       !case_sensitive && !lstrcmpi(buf1, buf2))
    {
      if (buf1 != _buf1)
        free(buf1);
      if (buf2 != buf2)
        free(buf2);
      return (char*)str1 + i;
    }
  }

  if (buf1 != _buf1)
    free(buf1);
  if (buf2 != buf2)
    free(buf2);
  return 0;
}

int lstrcmpn(const char* str1, const char* str2, int len, bool case_sensitive)
{
  if (!len)
    return 0;

  char _buf1[MAX_PATH];
  char _buf2[MAX_PATH];

  char* buf1 = (len + 1) > MAX_PATH ? (char*)malloc((len + 1) * sizeof(*buf1)) : _buf1;
  char* buf2 = (len + 1) > MAX_PATH ? (char*)malloc((len + 1) * sizeof(*buf2)) : _buf2;

  lstrcpyn(buf1, str1, len + 1);
  lstrcpyn(buf2, str2, len + 1);

  int res = case_sensitive ? lstrcmp(buf1, buf2) : lstrcmpi(buf1, buf2);

  if (buf1 != _buf1)
    free(buf1);
  if (buf2 != buf2)
    free(buf2);
  return res;
}

char* lstrchr(const char* str, char chr)
{
  if (!str)
    return 0;

  for (int len = 0; len < lstrlen(str); len++)
    if (str[len] == chr)
      return (char*)str + len;
  return 0;
}

int toupper(int c)
{
  return FSF.LUpper(c);
}

#ifdef __cplusplus
}
}
#endif
