#pragma once

namespace base {

template<typename T>
class scoped_setter {
public:
	scoped_setter(T& ptr, T value)
		: ptr_(ptr), saved_(ptr)
	{
		ptr = value;
	}
	~scoped_setter()
	{
		ptr_ = saved_;
	}

private:
	T& ptr_;
	T saved_;
};

}