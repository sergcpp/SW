#include "TimedInput.h"

#include <queue>
#include <mutex>

struct InputManagerImp {
    std::function<void(InputManager::Event &)> input_converters[InputManager::NUM_EVENTS];
    std::queue<InputManager::Event> input_buffer;
#if !defined(__EMSCRIPTEN__)
    std::mutex buffer_mtx;
#endif
};

InputManager::InputManager() {
    imp_ = new InputManagerImp();
}

InputManager::~InputManager() {
    delete imp_;
}

void InputManager::SetConverter(RawInputEvent evt_type, std::function<void(Event &)> conv) {
    imp_->input_converters[evt_type] = conv;
}

void InputManager::AddRawInputEvent(Event &evt) {
#if !defined(__EMSCRIPTEN__)
    std::lock_guard<std::mutex> lock(imp_->buffer_mtx);
#endif
	if (imp_->input_buffer.size() > 10) {
		return;
	}
    auto conv = imp_->input_converters[evt.type];
    if (conv) {
        conv(evt);
    }
    imp_->input_buffer.push(evt);
}

bool InputManager::PollEvent(unsigned int time, Event &evt) {
#if !defined(__EMSCRIPTEN__)
    std::lock_guard<std::mutex> lock(imp_->buffer_mtx);
#endif
    if (imp_->input_buffer.empty()) {
        return false;
    } else {
        evt = imp_->input_buffer.front();
        if (evt.time_stamp <= time) {
            imp_->input_buffer.pop();
            return true;
        } else {
            return false;
        }
    }
}

void InputManager::ClearBuffer() {
#if !defined(__EMSCRIPTEN__)
    std::lock_guard<std::mutex> lock(imp_->buffer_mtx);
#endif
    while (imp_->input_buffer.size()) {
        imp_->input_buffer.pop();
    }
}