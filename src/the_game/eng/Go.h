#ifndef GO_H
#define GO_H

#include "GoID.h"

namespace {
    const int MAX_GO_COMPONENTS = 32;
}

class GoComponent;

class GameObject {
    GoID id_;
    GoComponent *components_[MAX_GO_COMPONENTS];
    int num_components_;
public:
    GameObject(const GoID &id) : id_(id), num_components_(0) {}
    GameObject(const GameObject &rhs) = delete;
    GameObject(GameObject &&rhs);
    ~GameObject();

    GameObject &operator=(const GameObject &rhs) = delete;
    GameObject &operator=(GameObject &&rhs);

    GoID id() const { return id_; }
    void set_id(const GoID &id) { id_ = id; }

    GoComponent *GetComponent(const GoID &id);

    template<typename T>
    T *GetComponent() {
        return (T *)GetComponent(T::static_id());
    }

    void AddComponent(GoComponent *c);
    void ClearComponents();
};

#endif // GO_H
