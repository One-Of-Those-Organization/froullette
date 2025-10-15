#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#include <raylib.h>
#include <string>
#include <unordered_map>

struct TextureManager {
  public:
    std::unordered_map<std::string, Texture2D> mData;

    TextureManager();
    ~TextureManager();

    Texture2D *load_texture(std::string name, std::string img_path);
    Texture2D *get_texture(const char *name);
    void unload_texture(const char *name);
};

#endif // TEXTURE_MANAGER_H_
