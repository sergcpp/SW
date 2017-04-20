#ifndef GAME_BASE_H
#define GAME_BASE_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "Config.h"
#include "FrameInfo.h"

class GameBase {
protected:
    std::map<std::string, std::shared_ptr<void>> components_;
    FrameInfo fr_info_;
public:
    GameBase(int w, int h, const char *local_dir);
    virtual ~GameBase();

    virtual void Resize(int w, int h);

    void Start();
    void Frame();
    void Quit();

    template <class T>
    void SetComponent(const std::string &name, const std::shared_ptr<T> &p) {
        components_[name] = p;
    }

    template <class T>
    std::shared_ptr<T> GetComponent(const std::string &name) {
        auto it = components_.find(name);
        if (it != components_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    std::atomic_bool terminated;
    int width, height;
};

#endif // GAME_BASE_H
