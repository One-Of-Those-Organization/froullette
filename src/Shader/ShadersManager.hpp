#ifndef SHADERS_MANAGER_H_
#define SHADERS_MANAGER_H_

#include <raylib.h>
#include <string>
#include <unordered_map>

struct ShadersManager {
  private:
    std::unordered_map<std::string, Shader> mData;

  public:
    ShadersManager();
    ~ShadersManager();

    Shader *add_shader(const char *name, const char *vertex, const char *fragment);
    Shader *add_shader_from_mem(const char *name, const char *vertex, const char *fragment);
    Shader *get_shader(const char *name);
};

#endif // SHADERS_MANAGER_H_
