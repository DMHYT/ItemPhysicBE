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
    static ItemActor* currentRenderedItemActor;
    static inline long long getTimeNanoseconds() {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
    }
    static int getModelCount(ItemStackBase* stack);
    static bool isConduitOrSkull(ItemStackBase* stack);
    static void neutralizeVanillaRenderTransformations(BaseActorRenderContext* ctx, const MatrixStack::MatrixStackRef& ref, ItemActor* itemActor, float scaleFactor);
    static void applyRotations(const MatrixStack::MatrixStackRef& ref, ItemActor* itemActor, float scaleFactor, bool isItem3D, bool applyTranslations);
    static bool render(BaseActorRenderContext* ctx, ItemActor* itemActor, float scaleFactor);
    ItemPhysicModule(): Module("itemphysic") {}
    virtual void initialize();
};


#endif //ITEMPHYSIC_MAIN_HPP