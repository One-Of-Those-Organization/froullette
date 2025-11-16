#ifndef OBJECT_MANAGER_H_
#define OBJECT_MANAGER_H_

#include <vector>
#include <algorithm>
#include "Object.hpp"
#include "../Game/GameState.hpp"

struct ObjectSlot {
    int id;
    int z;
    Object* obj;
};

struct ObjectManager {
public:
    std::vector<ObjectSlot> objects;
    std::vector<Object*> sorted;
    int counter;

    ObjectManager() : counter(0) {}
    ~ObjectManager() { clear(); }

    int add_object(Object* obj, int z) {
        ObjectSlot slot;
        slot.id = counter++;
        slot.z = z;
        slot.obj = obj;
        slot.obj->show = true;
        slot.obj->state = obj->state;
        objects.push_back(slot);
        regenerate_sorted_data();
        return slot.id;
    }

    void rem_object(int id) {
        for (size_t i = 0; i < objects.size(); ++i) {
            if (objects[i].id == id) {
                delete objects[i].obj;
                objects.erase(objects.begin() + i);
                break;
            }
        }
        regenerate_sorted_data();
    }

    Object* get_object(int id) {
        for (auto& s : objects) {
            if (s.id == id)
                return s.obj;
        }
        return nullptr;
    }

    void regenerate_sorted_data() {
        sorted.clear();
        sorted.reserve(objects.size());

        for (auto& s : objects)
            sorted.push_back(s.obj);

        std::sort(objects.begin(), objects.end(),
            [](const ObjectSlot& a, const ObjectSlot& b) {
                return a.z < b.z;
            });
    }

    void clear() {
        for (auto& s : objects)
            delete s.obj;
        objects.clear();
        sorted.clear();
    }
};

#endif // OBJECT_MANAGER_H_
