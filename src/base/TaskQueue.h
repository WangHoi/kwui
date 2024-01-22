#pragma once
#include "fifo_map.hpp"
#include <functional>

namespace base {

class TaskQueue {
public:
	typedef size_t TaskId;

	TaskId add(std::function<void()>&& fn);
	bool remove(TaskId id);
	bool contains(TaskId id) const;
	void run();

private:
	nlohmann::fifo_map<TaskId, std::function<void()>> tasks_;
	TaskId next_task_id_ = 1;
};

}
