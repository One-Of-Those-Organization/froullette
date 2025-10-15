#include "ObjectManager.hpp"

// TODO: Make it store randomly then use cpp quick sort function using the
// mZIndex to sort them. the correct z-index.

ObjectManager::ObjectManager() {
    mData.reserve(20);
    mGame_ptr = nullptr;
}

ObjectManager::~ObjectManager() {}

sptr_t<Object> ObjectManager::add_object(sptr_t<Object> obj) {
    obj->mGame_ptr = mGame_ptr;
    if ((size_t)obj->mZIndex >= mData.size()) {
        obj->mZIndex = mData.size() - 1;
        mData.push_back(obj);
        return obj;
    }
    mData.insert(mData.begin() + obj->mZIndex, obj);

    for (size_t i = obj->mZIndex; i < mData.size(); i++) {
        auto &current = mData[i];
        current->mZIndex = i;
    }

    return obj;
}

void ObjectManager::rem_object(sptr_t<Object> obj) {
    for (size_t i = 0; i < mData.size(); i++) {
        auto &d = mData[i];
        if (d == obj)
            mData.erase(mData.begin() + i);
        break;
    }
}

void ObjectManager::rem_object(size_t z_index) {
    mData.erase(mData.begin() + z_index);
}

void ObjectManager::rem_object(std::string &name) {
    for (size_t i = 0; i < mData.size(); i++) {
        auto &d = mData[i];
        if (d->mName == name)
            mData.erase(mData.begin() + i);
        break;
    }
}

sptr_t<Object> ObjectManager::get_object(const char *name) {
    for (size_t i = 0; i < mData.size(); i++) {
        if (mData[i]->mName == name)
            return mData[i];
    }
    return nullptr;
}
