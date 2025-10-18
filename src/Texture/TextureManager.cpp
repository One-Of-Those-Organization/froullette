#include "TextureManager.hpp"

TextureManager::TextureManager() { mData.reserve(20); };

Texture2D *TextureManager::get_texture(const char *name) {
    for (auto &data : mData) {
        if (data.first == name) {
            return &data.second;
        }
    }
    return nullptr;
};

Texture2D *TextureManager::load_texture(std::string name, std::string img_path) {
    Texture2D *ctxt = get_texture(name.c_str());
    if (ctxt != nullptr) {
        return nullptr;
    }
    Texture2D txt = LoadTexture(img_path.c_str());
    mData.insert_or_assign(name, txt);
    return &mData[name];
}

void TextureManager::unload_texture(const char *name) {
    Texture2D *ctxt = get_texture(name);
    if (ctxt != nullptr) {
        for (auto &data : mData) {
            if (data.first == name) {
                UnloadTexture(data.second);
                break;
            }
        }
        mData.erase(name);
    }
}

TextureManager::~TextureManager() {
    for (auto &data : mData) {
        UnloadTexture(data.second);
    }
};
