#include "TaskQueue.h"
#include "log.h"

namespace base {

TaskQueue::TaskId TaskQueue::add(std::function<void()>&& fn)
{
	auto id = next_task_id_;
	++next_task_id_;
	if (next_task_id_ == 0)
		next_task_id_ = 1;
	DCHECK(!contains(next_task_id_)) << "TaskQueue wrap overflow.";
	tasks_[id] = fn;
	return id;
}

bool TaskQueue::remove(TaskId id)
{
	auto it = tasks_.find(id);
	if (it == tasks_.end())
		return false;
	tasks_.erase(it);
	return true;
}

bool TaskQueue::contains(TaskId id) const
{
	return (tasks_.find(id) != tasks_.end());
}

void TaskQueue::run()
{
	auto tsks = std::move(tasks_);
	tasks_.clear();
	for (auto& p : tsks) {
		p.second();
	}
}

}
