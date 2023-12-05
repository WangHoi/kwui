#pragma once
#include "Object.h"

namespace base {

template <typename T>
class WeakObjectProxy : public Object {
public:
    WeakObjectProxy(T *ptr)
        : ptr_(ptr) {}
    ~WeakObjectProxy()
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

template <typename O>
class object_weakptr
{
public:
	object_weakptr()
		: ptr_(nullptr)
	{}
	object_weakptr(WeakObjectProxy<O>* ptr)
		: ptr_(ptr)
	{
		if (ptr_)
			ptr_->retain();
	}
	object_weakptr(const object_weakptr& o)
		: ptr_(o.ptr_)
	{
		if (ptr_)
			ptr_->retain();
	}
	object_weakptr(object_weakptr&& o) noexcept
		: ptr_(o.ptr_)
	{
		o.ptr_ = nullptr;
	}
	~object_weakptr()
	{
		if (ptr_)
			ptr_->release();
	}
	object_weakptr& operator=(const object_weakptr& o)
	{
		if (ptr_ != o.ptr_) {
			if (o.ptr_)
				o.ptr_->retain();
			if (ptr_)
				ptr_->release();
			ptr_ = o.ptr_;
		}
		return *this;
	}
	object_weakptr& operator=(object_weakptr&& o) noexcept
	{
		if (ptr_ != o.ptr_) {
			if (ptr_)
				ptr_->release();
			ptr_ = o.ptr_;
			o.ptr_ = nullptr;
		}
		return *this;
	}
	inline operator bool() const
	{
		return ptr_ != nullptr && ptr_->get() != nullptr;
	}
	inline O* operator->() const
	{
		return ptr_ ? ptr_->get() : nullptr;
	}
	inline O* get() const
	{
		return ptr_ ? ptr_->get() : nullptr;
	}
	object_refptr<O> upgrade() const {
		return ptr_ && ptr_->get() ? object_refptr<O>(ptr_->get()) : object_refptr<O>();
	}
private:
	WeakObjectProxy<O>* ptr_ = nullptr;
};

}
