#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <memory>
#include <vector>

namespace R {
	typedef void(*TaskFunc)(void *arg);
	struct Task {
		TaskFunc	func;
		void		*arg;
	};

	struct TaskList : public std::vector<Task> {
		std::shared_ptr<void> done_event;

		TaskList();
		TaskList(size_t size) : TaskList() {
			this->reserve(size);
		}

		void Submit();
		void Wait();
	};

	void AddTaskList(TaskList &&list);
	void AddSingleTask(TaskFunc func, void *arg);
	void ProcessSingleTask(TaskFunc func, void *arg);

	template<typename T>
	void ProcessSingleTask(T func) {
		auto f = new T(func);
		ProcessSingleTask([](void *arg){
			auto ff = (T *)arg;
			(*ff)();
			delete ff;
		}, f);
	}

	void ProcessTasks();
}

#endif // RENDER_THREAD_H