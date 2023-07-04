#include <chrono>
#include <unordered_map>

#include <mod.h>

#include <innercore/vtable.h>

#include "recovered.hpp"

#ifndef ITEMPHYSIC_MAIN_HPP
#define ITEMPHYSIC_MAIN_HPP


class ItemPhysicModule : public Module {
    public:
    class Rotation {
        public:
        static std::unordered_map<long long, float> data;
        static float get(Actor*);
        static void set(Actor*, float);
        static void remove(Actor*);
    };
    static float ROTATE_SPEED;
    static long long lastTickTime;
    static inline long long getTimeNanoseconds() {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    }
    static bool is3D(ItemStackBase* stack);
    static bool render(BaseActorRenderContext* ctx, ItemActor* itemActor, int itemCount, float scaleFactor);
    ItemPhysicModule(): Module("itemphysic") {}
    virtual void initialize();
};


#endif //ITEMPHYSIC_MAIN_HPP