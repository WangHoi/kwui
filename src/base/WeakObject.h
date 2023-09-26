#pragma once
#include "Object.h"

namespace base {

template <typename T>
class WeakObject : public Object {
public:
    WeakObject(T *ptr)
        : ptr_(ptr) {}
    ~WeakObject()
    {
        ptr_ = nullptr;
    }
    inline void clear()
    {
        ptr_ = nullptr;
    }
    inline T* get() const
    {
        return ptr_;    
    }

private:
    T *ptr_;
};

}
