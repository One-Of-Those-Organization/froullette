#ifndef OBJECT_MANAGER_H_
#define OBJECT_MANAGER_H_

#include "../Object/Object.hpp"
#include "../def.hpp"
#include <vector>

struct ObjectManager {
  public:
    std::vector<sptr_t<Object>> mData;
    Game *mGame_ptr;

    ObjectManager();
    ~ObjectManager();

    sptr_t<Object> add_object(sptr_t<Object> obj);
    void rem_object(sptr_t<Object> obj);
    void rem_object(std::string &name);
    void rem_object(size_t z_index);
    sptr_t<Object> get_object(const char *name);
};

#endif // OBJECT_MANAGER_H_
