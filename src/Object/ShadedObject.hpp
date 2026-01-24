#pragma once
#include "Object.hpp"
#include "raylib.h"
#include <functional>

class ShadedObject: public Object {
public:
  int blend_type = BLEND_ALPHA;
  bool use_blend = false;
  Shader* shader;
  std::function<void(ShadedObject *)>shaders_update_callback = nullptr;
  ShadedObject(): Object() {};
  virtual ~ShadedObject() = default;

  void render() override {
    if (!show || !shader) return;
    if (use_blend) BeginBlendMode(blend_type);
    BeginShaderMode(*shader);
    Object::render();
    EndShaderMode();
    if (use_blend) EndBlendMode();
  }

  void logic(float dt) override {
    (void)dt;
    if (!show || !shader) return;
    if (shaders_update_callback) shaders_update_callback(this);
  }
};
