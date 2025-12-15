#pragma once

#include "Object.hpp"
#include "rlgl.h"

class Desk : public Object {
public:
    Vector2 angle; // x: top skew, y: height shrink/stretch factor

    Desk() : angle{0.0f, 0.2f} {}
    virtual ~Desk() = default;

    void render() override {
        if (!show) return;

        if (text != nullptr && text->width > 0 && text->height > 0) {
            DrawDeskTrapezoidTextured();
        } else DrawDeskTrapezoid();
    }

    void logic(float dt) override {
        (void)dt;
    }

private:
    void DrawDeskTrapezoid() {
        float x = rec.x;
        float y = rec.y;
        float w = rec.width;
        float h = rec.height;

        // Calculate how much to extend the bottom to maintain full width at top
        float shrink = angle.y * w * 0.5f;  // How much the top is narrower
        float skew = angle.x * w * 0.5f;    // Horizontal skew

        // Top vertices (use the original rectangle bounds)
        Vector2 topLeft  = {x, y};
        Vector2 topRight = {x + w, y};

        // Bottom vertices (extended outward to create trapezoid effect)
        Vector2 bottomLeft  = {x - shrink + skew, y + h};
        Vector2 bottomRight = {x + w + shrink + skew, y + h};

        Color color = this->color;

        rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // triangle 1
        rlVertex2f(bottomLeft.x, bottomLeft.y);
        rlVertex2f(bottomRight.x, bottomRight.y);
        rlVertex2f(topRight.x, topRight.y);

        // triangle 2
        rlVertex2f(bottomLeft.x, bottomLeft.y);
        rlVertex2f(topRight.x, topRight.y);
        rlVertex2f(topLeft.x, topLeft.y);

        rlEnd();
    }

    void DrawDeskTrapezoidTextured() {
        float x = rec.x;
        float y = rec.y;
        float w = rec.width;
        float h = rec.height;

        // Calculate how much to extend the bottom to maintain full width at top
        float shrink = angle.y * w * 0.5f;  // How much the top is narrower
        float skew = angle.x * w * 0.5f;    // Horizontal skew

        // Top vertices (use the original rectangle bounds)
        Vector2 topLeft  = {x, y};
        Vector2 topRight = {x + w, y};

        // Bottom vertices (extended outward to create trapezoid effect)
        Vector2 bottomLeft  = {x - shrink + skew, y + h};
        Vector2 bottomRight = {x + w + shrink + skew, y + h};

        // UV coordinates for texture mapping
        Vector2 uvTopLeft     = {0.0f, 0.0f};  // Top-left of texture
        Vector2 uvTopRight    = {1.0f, 0.0f};  // Top-right of texture
        Vector2 uvBottomLeft  = {0.0f, 1.0f};  // Bottom-left of texture
        Vector2 uvBottomRight = {1.0f, 1.0f};  // Bottom-right of texture

        Color color = WHITE;

        rlSetTexture(text->id);  // Bind the texture
        rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Triangle 1: bottomLeft -> bottomRight -> topRight
        rlTexCoord2f(uvBottomLeft.x, uvBottomLeft.y);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        rlTexCoord2f(uvBottomRight.x, uvBottomRight.y);
        rlVertex2f(bottomRight.x, bottomRight.y);

        rlTexCoord2f(uvTopRight.x, uvTopRight.y);
        rlVertex2f(topRight.x, topRight.y);

        // Triangle 2: bottomLeft -> topRight -> topLeft
        rlTexCoord2f(uvBottomLeft.x, uvBottomLeft.y);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        rlTexCoord2f(uvTopRight.x, uvTopRight.y);
        rlVertex2f(topRight.x, topRight.y);

        rlTexCoord2f(uvTopLeft.x, uvTopLeft.y);
        rlVertex2f(topLeft.x, topLeft.y);

        rlEnd();
        rlSetTexture(0);  // Unbind texture
    }
};
