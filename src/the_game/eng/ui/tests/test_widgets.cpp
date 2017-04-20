#include "test_common.h"

#include "../BaseElement.h"

void test_widgets() {
    using namespace glm;

    {   // BaseElement tests
        {   // Simple element
            ui::RootElement root({1000, 1000});
            ui::BaseElement el({-0.5f, -0.5f}, {1, 1}, &root);

            assert(el.pos() == vec2(-0.5f, -0.5f));
            assert(el.size() == vec2(1, 1));
            assert(el.pos_px() == ivec2(250, 250));
            assert(el.size_px() == ivec2(500, 500));

            root.set_zone({2000, 2000});
            el.Resize(&root);
            el.Resize(&root);

            assert(el.pos() == vec2(-0.5f, -0.5f));
            assert(el.size() == vec2(1, 1));
            assert(el.pos_px() == ivec2(500, 500));
            assert(el.size_px() == ivec2(1000, 1000));

            root.set_zone({1000, 1000});
            el.Resize({0, 0}, {0.5f, 0.5f}, &root);

            assert(el.pos() == vec2(0, 0));
            assert(el.size() == vec2(0.5f, 0.5f));
            assert(el.pos_px() == ivec2(500, 500));
            assert(el.size_px() == ivec2(250, 250));

            assert(el.Check(vec2(0.25f, 0.25f)));
            assert_false(el.Check(vec2(-0.25f, 0.25f)));
            assert_false(el.Check(vec2(-0.25f, -0.25f)));
            assert_false(el.Check(vec2(0.25f, -0.25f)));

            assert(el.Check(ivec2(600, 600)));
            assert_false(el.Check(ivec2(-600, 600)));
            assert_false(el.Check(ivec2(-600, -600)));
            assert_false(el.Check(ivec2(600, -600)));
        }

        {   // Parenting
            ui::RootElement root({1000, 1000});
            ui::BaseElement par_el({0, 0}, {1, 1}, &root);
            ui::BaseElement child_el({0, 0}, {1, 1}, &par_el);

            assert(child_el.pos() == vec2(0.5f, 0.5f));
            assert(child_el.size() == vec2(0.5f, 0.5f));
            assert(child_el.pos_px() == ivec2(750, 750));
            assert(child_el.size_px() == ivec2(250, 250));

            par_el.Resize(&root);
            child_el.Resize(&par_el);

            assert(child_el.pos() == vec2(0.5f, 0.5f));
            assert(child_el.size() == vec2(0.5f, 0.5f));
            assert(child_el.pos_px() == ivec2(750, 750));
            assert(child_el.size_px() == ivec2(250, 250));

            par_el.Resize({0.5f, 0.5f}, {0.5f, 0.5f}, &root);
            child_el.Resize(&par_el);

            assert(child_el.pos() == vec2(0.75f, 0.75f));
            assert(child_el.size() == vec2(0.25f, 0.25f));
            assert(child_el.pos_px() == ivec2(875, 875));
            assert(child_el.size_px() == ivec2(125, 125));
        }
    }

    {   // LinearLayout tests
        // TODO
    }

}
