#ifndef OBJECT_MANAGER_H_
#define OBJECT_MANAGER_H_

#include <vector>
#include <algorithm>
#include "Object.hpp"
#include "../Game/GameState.hpp"

struct ObjectSlot {
    int id;
    int z;
    int _oldz;
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
        slot._oldz = z;
        slot.obj = obj;
        slot.obj->id = slot.id;
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

    int get_zindex(int id) {
        for (auto& s : objects) {
            if (s.id == id) {
                return s.z;
            }
        }
        return -1;
    }

    bool update_zindex(int id, int new_zindex) {
        for (auto& s : objects) {
            if (s.id == id) {
                if (s.z != new_zindex) s._oldz = s.z;
                s.z = new_zindex;
                regenerate_sorted_data();
                return true;
            }
        }
        return false;
    }

    void revert_zindex(int id) {
        for (auto& s : objects) {
            if (s.id == id) {
                s.z = s._oldz;
                regenerate_sorted_data();
                return;
            }
        }
    }

    void switch_zindex(int id1, int id2) {
        ObjectSlot *o1 = nullptr;
        ObjectSlot *o2 = nullptr;
        for (auto& s : objects) {
            if (o1 && o2) break;
            if (s.id == id1) o1 = &s;
            if (s.id == id2) o2 = &s;
        }
        o1->_oldz = o1->z;
        o2->_oldz = o2->z;

        // NOTE: The fancy xor operator switching
        o1->z ^= o2->z;
        o2->z ^= o1->z;
        o1->z ^= o2->z;
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
        std::sort(objects.begin(), objects.end(),
        [](const ObjectSlot& a, const ObjectSlot& b) {
            return a.z < b.z;
        });

        sorted.clear();
        for (auto& s : objects)
        sorted.push_back(s.obj);
    }

    void clear() {
        for (auto& s : objects)
            delete s.obj;
        objects.clear();
        sorted.clear();
    }
};

#endif // OBJECT_MANAGER_H_
