#pragma once

enum Alignment {
    LEFT, CENTER, RIGHT
};

class HBox: public Object {
    public:
        std::vector<Object *>child;
        uint32_t _all_child_len = 0;
        uint32_t padding = 0;
        Alignment al = LEFT;

        HBox(): Object() {}
        ~HBox() = default;

        void render() override {};
        void logic(float dt) override {
            (void)dt;
        };

        void add_child(Object *obj) {
            child.push_back(obj);
        }
        void position_child() {
            float total_content_width = 0;
            for (const auto& c : this->child) {
                total_content_width += c->rec.width + (padding * 2);
            }

            float offset = rec.x;

            switch (al) {
            case LEFT:
                offset = rec.x;
                break;
            case CENTER:
                offset = rec.x + (rec.width - total_content_width) / 2.0f;
                break;
            case RIGHT:
                offset = rec.x + rec.width - total_content_width;
                break;
            }

            for (auto& c : this->child) {
                c->rec.x = offset + padding;
                c->rec.y = rec.y;
                offset = c->rec.x + c->rec.width + padding;
            }
            _all_child_len = (uint32_t)total_content_width;
        }
};
