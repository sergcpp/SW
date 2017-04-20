#ifndef GOCOMPONENT_H
#define GOCOMPONENT_H

#include "GoAlloc.h"
#include "GoID.h"

#include "ren/SparseArray.h"

class GameObject;

class GoComponent {
    GameObject *owner_;
public:
    GoComponent() : owner_(nullptr) {}
    virtual ~GoComponent() {}

    virtual GoID id() const = 0;

    virtual void Update() {}

    GameObject *owner() const { return owner_; }
    void set_owner(GameObject *go) { owner_ = go; }
};

#define DEF_ID(x)                       \
    virtual GoID id() const override {  \
        return static_id();             \
    }                                   \
    static GoID static_id() {           \
        return GoID((x));               \
    }

#endif // GOCOMPONENT_H
