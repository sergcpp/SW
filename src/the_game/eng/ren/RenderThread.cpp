#include "RenderThread.h"

#include <mutex>
#include <thread>

#include "RingBuffer.h"

namespace R {
	RingBuffer<TaskList> task_lists(128);
	std::mutex add_list_mtx;
}

R::TaskList::TaskList() {
	done_event = std::make_shared<std::atomic_bool>();
	*(std::atomic_bool*)done_event.get() = false;
}

void R::TaskList::Submit() {
	R::AddTaskList(std::move(*this));
}

void R::TaskList::Wait() {
#ifndef __EMSCRIPTEN__
	while (!*(std::atomic_bool*)done_event.get()) {
		std::this_thread::yield();
	}
#endif
}

void R::AddTaskList(TaskList &&list) {
#ifndef __EMSCRIPTEN__
	std::lock_guard<std::mutex> lck(add_list_mtx);
	task_lists.Push(std::move(list));
#else
	for (auto &t : list) {
		t.func(t.arg);
	}
	*(std::atomic_bool*)list.done_event.get() = true;
#endif
}

void R::AddSingleTask(TaskFunc func, void *arg) {
#ifndef __EMSCRIPTEN__
	TaskList list;
	list.push_back({ func, arg });
	list.Submit();
#else
	func(arg);
#endif
}

void R::ProcessSingleTask(TaskFunc func, void *arg) {
#ifndef __EMSCRIPTEN__
	TaskList list;
	list.push_back({ func, arg });
	list.Submit();
	list.Wait();
#else
	func(arg);
#endif
}

void R::ProcessTasks() {
#ifndef __EMSCRIPTEN__
	TaskList list;
	while (task_lists.Pop(list)) {
		for (auto &t : list) {
			t.func(t.arg);
		}
		*(std::atomic_bool*)list.done_event.get() = true;
	}
#endif
}