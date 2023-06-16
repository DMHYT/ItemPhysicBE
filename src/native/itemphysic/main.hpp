#include <mod.h>

#ifndef ITEMPHYSIC_MAIN_HPP
#define ITEMPHYSIC_MAIN_HPP


class ItemPhysicModule : public Module {
    public:
    static float ROTATE_SPEED;
    ItemPhysicModule(): Module("itemphysic") {}
    virtual void initialize();
};


#endif //ITEMPHYSIC_MAIN_HPP