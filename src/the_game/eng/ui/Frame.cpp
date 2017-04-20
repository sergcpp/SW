#include "Frame.h"

#include <memory>

#include <sys/AssetFile.h>

#include "Renderer.h"

ui::Frame::Frame(const R::Texture2DRef &tex, const glm::vec2 &offsets,
                 const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
    : BaseElement(pos, size, parent), tex_(tex), frame_offset_(offsets[0]), frame_offset_uv_(offsets[1]) {}

ui::Frame::Frame(const char *tex_name, const glm::vec2 &offsets,
                 const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
    : BaseElement(pos, size, parent), frame_offset_(offsets[0]), frame_offset_uv_(offsets[1]) {

    sys::AssetFile in_file(tex_name, sys::AssetFile::IN);
    size_t in_file_size = in_file.size();
    std::unique_ptr<char[]> data(new char[in_file_size]);
    in_file.Read(data.get(), in_file_size);

    tex_ = R::LoadTexture2D(tex_name, data.get());

    Resize(parent);
}

void ui::Frame::Resize(const ui::BaseElement *parent) {
    BaseElement::Resize(parent);
    
    const glm::vec2 off = 1.0f * glm::vec2{frame_offset_} * dims_[1] / (glm::vec2)dims_px_[1];
    const glm::vec2 min = dims_[0], max = dims_[0] + dims_[1];

    positions_ = {min.x, min.y, 0,
                  min.x + off.x, min.y, 0,
                  min.x + off.x, min.y + off.y, 0,
                  min.x, min.y + off.y, 0,

                  min.x + off.x, max.y - off.y, 0,
                  min.x, max.y - off.y, 0,

                  max.x - off.x, min.y + off.y, 0,
                  max.x - off.x, max.y - off.y, 0,

                  max.x - off.x, min.y, 0,

                  max.x, min.y, 0,
                  max.x, min.y + off.y, 0,

                  max.x, max.y - off.y, 0,

                  max.x, max.y, 0,
                  max.x - off.x, max.y, 0,

                  min.x + off.x, max.y, 0,
                  min.x, max.y, 0,
    };
    
    uvs_ = {0, 0,
            frame_offset_uv_, 0,
            frame_offset_uv_, frame_offset_uv_,
            0, frame_offset_uv_,

            frame_offset_uv_, 1 - frame_offset_uv_,
            0, 1- frame_offset_uv_,
                              
            1 - frame_offset_uv_, frame_offset_uv_,
            1 - frame_offset_uv_, 1 - frame_offset_uv_,

            1 - frame_offset_uv_, 0,

            1, 0,
            1, frame_offset_uv_,

            1, 1 - frame_offset_uv_,

            1, 1,
            1 - frame_offset_uv_, 1,

            frame_offset_uv_, 1,
            0, 1,
    };

    indices_ = {0, 1, 2,
                0, 2, 3,

                2, 4, 3,
                3, 4, 5,

                2, 6, 7,
                2, 7, 4,

                1, 8, 6,
                1, 6, 2,

                8, 9, 10,
                8, 10, 6,

                6, 10, 11,
                6, 11, 7,

                7, 11, 12,
                7, 12, 13,

                4, 7, 13,
                4, 13, 14,

                5, 4, 14,
                5, 14, 15,
    };
}

void ui::Frame::Draw(ui::Renderer *r) {
    r->DrawUIElement(tex_, PRIM_TRIANGLE, positions_, uvs_, indices_);
}
