#pragma once

#include <vector>

#include "../eng/ren/Texture.h"
#include "../eng/ui/BaseElement.h"

class GBackground : ui::BaseElement {
    R::Texture2DRef tex_, back_;
    
    struct Particle {
        unsigned lifetime;
        glm::vec2 pos, vel;

		Particle(const glm::vec2 &_pos, const glm::vec2 &_vel) : lifetime(0), pos(_pos), vel(_vel) {}
    };

    float aspect_;
    std::vector<Particle> particles1_, particles2_, particles3_;
    glm::vec2 spawn_point_, particle_vel_;
    unsigned spawn_timer_;
 public:
    GBackground(const glm::vec2 &spawn_point, const glm::vec2 &vel, const ui::BaseElement *parent);

    void Resize(const BaseElement *parent) override;
    
    void Update(int dt_ms);
    
    void Draw(ui::Renderer *r) override;
};

