#pragma once
#include "Object.hpp"
#include <functional>

class ShadedObject: public Object {
public:
  Shader* shader;
  std::function<void(ShadedObject *)>shaders_update_callback = nullptr;
  ShadedObject(): Object() {};
  virtual ~ShadedObject() = default;

  void render() override {
    if (!show || !shader) return;
    BeginShaderMode(*shader);
    Object::render();
    EndShaderMode();
  }

  void logic(float dt) override {
    (void)dt;
    if (!show || !shader) return;
    if (shaders_update_callback) shaders_update_callback(this);
  }
};
