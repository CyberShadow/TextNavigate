#ifndef __CRTDLL_H
#define __CRTDLL_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MIN_SIZE 32
#define winNew(type, size) (type*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size)*sizeof(type));
#define winDel(var) HeapFree(GetProcessHeap(), 0, var);

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#ifdef FillMemory
#undef FillMemory
#endif

#define ZeroMemory(ptr, size) { memset(ptr, 0, size); }
#define FillMemory(ptr, c, size) { memset(ptr, c, size); }

#ifdef __cplusplus
namespace crt {
extern "C" {
#endif
typedef unsigned int size_t;

// stdlib.h
void* malloc(int size);
void free(void* ptr);

// mem.h
void *memset(void *s, int c, int n);
void* lmemcpy(void* dst, void* src, int num);
int lmemcmp(const void* b1, const void* b2, int num);
void* lmemmove(void* dest, const void* src, int n);

// string.h
//char *lstrrchr(const char *s, int c);
char* lstrstr(const char *string, const char *strCharSet, bool case_sensitive = false);
int lstrcmpn(const char* str1, const char* str2, int len, bool case_sensitive = false);
char* lstrchr(const char* str, char chr);

int toupper(int c);

#ifdef __cplusplus
}
}
#endif

extern void *__cdecl operator new(size_t size);
extern void *__cdecl operator new[](size_t size);
extern void __cdecl operator delete(void *block);
extern void __cdecl operator delete[](void *ptr);
void cdecl _pure_error_();

#endif /* __CRTDLL_H */
