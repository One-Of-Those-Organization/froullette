#include "ShadersManager.hpp"

ShadersManager::ShadersManager() { mData.reserve(20); }

ShadersManager::~ShadersManager() {
    for (auto &[_, d] : mData) {
        UnloadShader(d);
    }
}

Shader *ShadersManager::add_shader(const char *name, const char *vertex,
                                   const char *fragment) {
    if (vertex == nullptr || vertex[0] == '\0')
        mData[name] = LoadShader(NULL, fragment);
    else
        mData[name] = LoadShader(vertex, fragment);

    return &mData[name];
}

Shader *ShadersManager::add_shader_from_mem(const char *name,
                                            const char *vertex,
                                            const char *fragment) {
    if (vertex == nullptr || vertex[0] == '\0')
        mData[name] = LoadShaderFromMemory(NULL, fragment);
    else
        mData[name] = LoadShaderFromMemory(vertex, fragment);

    return &mData[name];
}

Shader *ShadersManager::get_shader(const char *name) {
    auto it = mData.find(name);
    if (it != mData.end()) {
        return &it->second;
    }
    return nullptr;
}
