#pragma once

namespace base {

template<typename T>
class scoped_setter {
public:
	scoped_setter(T& ptr, T value)
		: ptr_(ptr), val_(value) {}
	~scoped_setter()
	{
		ptr_ = val_;
	}

private:
	T& ptr_;
	T val_;
};

}