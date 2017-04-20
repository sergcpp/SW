#include "GBackground.h"

#include <memory>

#include "../eng/Random.h"
#include "../eng/sys/AssetFile.h"
#include "../eng/sys/Log.h"
#include "../eng/ui/Renderer.h"

namespace GBackgroundConstants {
    const char TEXTURE_NAME[] = "./game_assets/textures/ui/sparkles.tga";
    const char BACK_NAME[] = "./game_assets/textures/ui/back_01.tga";
    
    const unsigned SPAWN_PERIOD = 500;
    const unsigned MAX_LIFETIME = 2000;
    const unsigned MAX_PARTICLES_COUNT = 16;
    const glm::vec2 POSITION_RAND = {-0.25f, 0.25f};
    const glm::vec2 VELOCITY_RAND = {-0.035f, 0.035f};    
}

GBackground::GBackground(const glm::vec2 &spawn_point, const glm::vec2 &vel, const ui::BaseElement *parent)
        : ui::BaseElement(glm::vec2(-1, -1), glm::vec2(2, 2), parent),
          spawn_point_(spawn_point), spawn_timer_(0), particle_vel_(vel) {
    using namespace GBackgroundConstants;
    
    std::unique_ptr<char[]> in_file_data;
    
    {   sys::AssetFile in_file(TEXTURE_NAME, sys::AssetFile::IN);

        size_t in_file_size = in_file.size();
        in_file_data.reset(new char[in_file_size]);
        in_file.Read(in_file_data.get(), in_file_size);
    }

    tex_ = R::LoadTexture2D(TEXTURE_NAME, in_file_data.get());

    {   sys::AssetFile in_file(BACK_NAME, sys::AssetFile::IN);

        size_t in_file_size = in_file.size();
        in_file_data.reset(new char[in_file_size]);
        in_file.Read(in_file_data.get(), in_file_size);
    }

    back_ = R::LoadTexture2D(BACK_NAME, in_file_data.get(), R::Bilinear, R::ClampToEdge);

    Resize(parent);
}

void GBackground::Resize(const ui::BaseElement *parent) {
    aspect_ = ((float)parent->size_px().y) / parent->size_px().x;
}

void GBackground::Update(int dt_ms) {
    using namespace glm;
    using namespace GBackgroundConstants;

    spawn_timer_ += dt_ms;
    if (spawn_timer_ > SPAWN_PERIOD &&
        (particles1_.size() + particles2_.size() + particles3_.size()) < MAX_PARTICLES_COUNT) {
        glm::vec2 pos = spawn_point_ + POSITION_RAND * vec2{Random::GetFloat(-1, 1), Random::GetFloat(-1, 1)};
        int type = Random::GetInt(0, 3);

        if (type <= 1) {
            particles1_.emplace_back(spawn_point_, particle_vel_);
        } else if (type == 2) {
            particles2_.emplace_back(spawn_point_, 1.8f * particle_vel_);
        } else if (type == 3) {
            particles3_.emplace_back(spawn_point_, 0.65f * particle_vel_);
        }
        
        spawn_timer_ = 0;
    }
    
    float dt_s = 0.001f * dt_ms;

    auto update_particle = [this, dt_ms, dt_s](Particle &p) {
        p.lifetime += dt_ms;

        p.pos += dt_s * p.vel;
        p.vel += VELOCITY_RAND * vec2{Random::GetFloat(-0.1f, 0.1f), Random::GetFloat(-1, 1)};
        if (p.lifetime > MAX_LIFETIME && !Check(p.pos)) {
            p.lifetime = 0;
            p.pos = spawn_point_ + POSITION_RAND * vec2{Random::GetFloat(-1, 1)};
        }
    };
    
    for (auto &p : particles1_) {
        update_particle(p);
    }

    for (auto &p : particles2_) {
        update_particle(p);
    }

    for (auto &p : particles3_) {
        update_particle(p);
    }
}

void GBackground::Draw(ui::Renderer *r) {
    using namespace glm;
    
    {   // draw background image
        const vec2 dims[] = {{1 - 4 * aspect_, -1}, {4 * aspect_, 4}};
        const vec2 uvs[] = {{0, 0}, {1, 1}};
        r->DrawImageQuad(back_, dims, uvs);
    }

    {   // draw far blurry particles
        const vec2 uvs[] = {{0.75f, 0}, {1, 0.5f}};
        for (auto &p : particles3_) {
            const vec2 dims[] = {p.pos, {0.05f * aspect_, 0.05f}};
            r->DrawImageQuad(tex_, dims, uvs);
        }
    }
    
    {   // draw middle-distant sharp particles
        const vec2 uvs[] = {{0, 0}, {0.25f, 0.5f}};
        for (auto &p : particles1_) {
            const vec2 dims[] = {p.pos, {0.02f * aspect_, 0.02f}};
            r->DrawImageQuad(tex_, dims, uvs);
        }
    }

    {   // draw near blurry particles
        const vec2 uvs[] = {{0.25f, 0}, {0.66f, 1}};
        for (auto &p : particles2_) {
            const vec2 dims[] = {p.pos, {0.05f * aspect_, 0.05f}};
            r->DrawImageQuad(tex_, dims, uvs);
        }
    }
}
