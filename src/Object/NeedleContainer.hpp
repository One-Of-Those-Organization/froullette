#pragma once
#include "ObjectManager.hpp"
#include <vector>

// NOTE: This class job is just to keep track the id so the game knows what id is the needle, as for the needle itself it will be managed by the ObjectManger as usual.

class NeedleContainer: public Object {
    public:
        std::vector<int> needles;
        ObjectManager *om;

        NeedleContainer(ObjectManager *om): om(om) {}
        ~NeedleContainer() = default;
};
