#pragma once

#include <cassert>

namespace base {

class Object {
public:
	Object()
		: ref_count_(0)
	{}
	inline virtual ~Object() {}
	inline void retain()
	{
		++ref_count_;
	}
	inline void release()
	{
		assert(ref_count_ > 0);
		--ref_count_;
		if (ref_count_ == 0) {
			delete this;
		}
	}
protected:
	int ref_count_;
};

template <typename O>
class object_refptr
{
public:
	object_refptr()
		: ptr_(nullptr)
	{}
	object_refptr(O* ptr)
		: ptr_(ptr)
	{
		ptr_->retain();
	}
	object_refptr(const object_refptr& o)
		: ptr_(o.ptr_)
	{
		ptr_->retain();
	}
	object_refptr(object_refptr&& o) noexcept
		: ptr_(o.ptr_)
	{
		o.ptr_ = nullptr;
	}
	~object_refptr()
	{
		if (ptr_)
			ptr_->release();
	}
	object_refptr& operator=(const object_refptr& o)
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
	object_refptr& operator=(object_refptr&& o)
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
		return ptr_ != nullptr;
	}
	inline O* operator->() const
	{
		return ptr_;
	}
	O* get() const {
		return ptr_;
	}
private:
	O* ptr_ = nullptr;
};

}