#ifndef __STRING_HPP
#define __STRING_HPP

#include <windows.h>
#include <stddef.h>

#include "crtdll.h"

class string
{
  private:
    crt::size_t current_size;
    crt::size_t actual_size;
    crt::size_t hash_start;
    unsigned char *data;
    unsigned char default_data[1];
    HANDLE heap;
    void init(void);
    void copy(const string& Value);
    void copy(const unsigned char *Value);
    void copy(const unsigned char *Value,crt::size_t size);
    bool enlarge(crt::size_t size);
  public:
    string();
    string(const unsigned char *Value);
    string(const unsigned char *Value,crt::size_t size);
    string(const string& Value);
    ~string();
    string &operator=(const string& Value);
    string &operator=(const unsigned char *Value);
    string &operator()(const unsigned char *Value, crt::size_t size);
    operator const unsigned char *() const;
    unsigned char &operator[](crt::size_t index);
    crt::size_t length(void) const;
    //void reverse(void);
    unsigned long hash(void);
    void hash(crt::size_t pos);
    unsigned char *get(void);
    void clear(void);
    string& operator+=(unsigned char Value);
    string& operator+=(const char* Value);
};

int operator==(const string& x,const string& y);

#endif /* __STRING_HPP */
