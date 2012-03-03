//commons.h
#ifndef __COMMONS_H
#define __COMMONS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "crtdll.h"

/*******************************************************************************
              template class CRefColl
*******************************************************************************/

template<class T, bool owns = true>
class CRefColl
{
  private:
    bool _main;
    int _id;

  protected:
    T *_insert_before(CRefColl *to, int fid = -1, T *ref = NULL)
    {
      CRefColl *fl = to->prev;
      fl->next = new CRefColl(false);
      fl->next->prev = fl;
      fl = fl->next;
      fl->next = to;
      to->prev = fl;
      fl->_id = fid;
      fl->Ref = ref;
      return ref;
    }

    T *_insert_after(CRefColl *to, int fid = -1, T *ref = NULL)
    {
      return _insert_before(to->next, fid, ref);
    }

  public:
    T *Ref;
    CRefColl *next, *prev;

    explicit CRefColl(bool Main = false, T *ref = NULL)
    {
      _id = -1;
      _main = Main;
      next = prev = this;
      Ref = ref;
    }

    ~CRefColl()
    {
      if (!_main)
      {
        prev->next = next;
        next->prev = prev;
        if (owns && Ref) delete Ref;
        return;
      }
      CRefColl *fl = next;
      while (fl != this)
      {
        fl = fl->next;
        if (owns)
          delete fl->prev->Ref;
        delete fl->prev;
      }
      if (owns)
        delete Ref;
    }

    int Size()
    {
      int size(0);
      CRefColl *fl = next;
      while (fl != this)
      {
        size++;
        fl = fl->next;
      }
      return size;
    }

    bool main() { return _main; }
    int id() { return _id; }

    CRefColl* GetRefColl(int fid)
    {
      CRefColl *fl = next;
      while (fl != this)
      {
        if (fl->_id == fid) return fl;
        fl = fl->next;
      }
      return NULL;
    }

    T* Get(int fid)
    {
      CRefColl *fl = next;
      while (fl != this)
      {
        if (fl->_id == fid) return fl->Ref;
        fl = fl->next;
      }
      return NULL;
    }

    virtual T *Add(int fid = -1, T *&ref = (T *)NULL)
    {
      return _insert_before(this, fid, ref);
    }

    bool Delete(int fid)
    {
      CRefColl *fl = GetRefColl(fid);
      if (!fl) return false;
      delete fl;
      return true;
    }

    void Clear()
    {
      CRefColl *fl = next;
      while (fl != this)
      {
        CRefColl *_next = fl->next;
        delete fl;
        fl = _next;
      }
    }
}; //CRefColl

/*******************************************************************************
                     struct ScalarPtr
 Safepointer policy that defines a scalar pointer.
 Provides scalar 'delete' and doesn't provide the subscription method.
*******************************************************************************/

struct ScalarPtr
{
  template<class T>
  static void destroy(T *ptr) { delete ptr; }
}; //ScalarPtr

/*******************************************************************************
                     struct VectorPtr
 Safepointer policy that defines a vector pointer.
 Provides vector 'delete []' and the subscription method.
*******************************************************************************/

struct VectorPtr
{
  template<typename T>
  static void destroy(T *ptr) { delete [] ptr; }
  template<typename T, typename Ndx>
  static T &subscript(T *ptr, Ndx ndx) { return ptr[ndx]; }
  //template<typename T, typename Ndx>
  //static T* subscript(T* ptr, Ndx ndx) { return (ptr + ndx); }
}; //VectorPtr

/*******************************************************************************
                      template class AutoPtr
 Safe pointer template.
 A safe pointer automatically calls 'delete' for the owned object while being
 destructed itself.
*******************************************************************************/

template <typename type, class VPolicy = ScalarPtr>
class AutoPtr
{
  private:
    type* m_Data;

  public:
    typedef typename VPolicy VectorPolicy;
    typedef AutoPtr<type, VectorPolicy> self;

    AutoPtr(type* ptr = NULL) : m_Data(ptr)
    { }

    ~AutoPtr()
    {
      //assert(!(!m_Data && m_Size));
      if (m_Data)
      {
        VectorPolicy::destroy(Detach());
      }
    }

    operator type*()
    {
      return m_Data;
    }

    operator const type*() const
    {
      return m_Data;
    }

    DWORD Size()
    {
      return m_Size;
    }

    type* ptr() const { return m_Data; }

    type* operator->() const { return m_Data; }
    type& operator *() const { return *m_Data; }

    type& operator[](int ndx) const
    {
      return VectorPolicy::subscript(ptr(), ndx);
    }

    void Attach(type* ptr) throw()
    {
      Detach();
      m_Data = pData;
    }

    type *Detach()
    {
      type *old = m_Data;
      m_Data = NULL;
      return old;
    }

    self &operator=(type* ptr)
    {
      if (ptr != m_Data)
      {
        type* p = m_Data;
        m_Data = ptr;
        VectorPolicy::destroy(p);
      }
      return *this;
    }
}; //AutoPtr

/*******************************************************************************
                    class AutoPtr<char>
 Since the most often case with safe pointers to char is, actually, pointer
 to a string, provide full specialization that always uses VectorPolicy
*******************************************************************************/

template<>
class AutoPtr<char> : public AutoPtr<char, VectorPtr>
{
  public:
    typedef AutoPtr<char, VectorPtr> Parent;

    explicit AutoPtr(char *ptr = NULL) :
       Parent(ptr)
    {}

    AutoPtr<char> &operator=(char *ptr)
    {
      Parent::operator=(ptr);
      return *this;
    }
}; //AutoPtr<char>


/*******************************************************************************
              class CRefArray
*******************************************************************************/

template<class T>
class CRefArray
{
  private:
    AutoPtr<T, VectorPtr> items;
    int count;
  public:
    CRefArray(int Count) { count = Count; items = new T[count]; }
    virtual ~CRefArray() { }
    T *Item(int num) const { return (T *)((num >= 0 && num < count) ? (items + num) : NULL); }
    T *operator[](int num) const { return Item(num); }
    int Count(void) const { return count; }
    T *Items(void) const { return (T *)items.ptr(); }
}; //CRefArray

/*******************************************************************************
                      template class Stack
*******************************************************************************/

template<class T, int max_size = -1>
class CStack
{
  public:
    CStack() { }
    ~CStack() { Clear(); }
    
    bool Empty()
    {
      return (0 == _data.Size());
    }
    
    void Push(T* t)
    {
      _data.Add(_data.Size() + 1, t);
    }

    T* Pop()
    {
      int size(_data.Size());
      if (!size)
        return NULL;
      T* t = _data.Get(size);
      _data.Delete(size);
      return t;
    }

    void Clear()
    {
      while (!Empty())
        delete Pop();
    }

  private:
    CRefColl<T, false> _data;
}; //CStack

#endif /* __COMMONS_H */
