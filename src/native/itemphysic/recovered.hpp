#ifndef ITEMPHYSIC_RECOVERED_HPP
#define ITEMPHYSIC_RECOVERED_HPP


struct Vec2 {
    float x, y;
};

struct ActorUniqueID {
    long long id;
};

class Actor {
    public:
    char filler1[232]; // 232
    Vec2 rotation; // 240;
    char filler2[136]; // 376
    bool onGround; // 377
    char filler3[95]; // 472
    float stuckX, stuckY, stuckZ; // 484
    ActorUniqueID* getUniqueID() const;
};

class ItemStackBase {
    public:
    char filler[96];
    bool isNull() const;
    int getId() const;
    int getAuxValue() const;
    bool isBlock() const;
    void* getItem() const;
};

class ItemStack : public ItemStackBase { public: };

class ItemActor : public Actor {
    public:
    char filler4[772]; // 1256
    ItemStack itemStack; // 1352
    int life; // 1356
    char filler5[8]; // 1364
    float transformationOffset; // 1368
    char filler6[8]; // 1376
    bool renderingMotionFlag; // 1377 + 3
};

class Matrix {
    public:
    void translate(float x, float y, float z);
    void rotate(float angle, float multiplierX, float multiplierY, float multiplierZ);
};

class MatrixStack {
    public:
    class MatrixStackRef {
        public:
        char filler[8];
        Matrix* operator->() const;
        void release();
    };
    MatrixStackRef push();
};

enum ItemContextFlags: int {};

class BaseActorRenderContext;
class ItemInHandRenderer {
    public:
    void renderItem(BaseActorRenderContext&, Actor&, const ItemStack&, bool, ItemContextFlags, bool);
};

class BaseActorRenderContext {
    public:
    char filler[12]; // 12
    float tickDelta; // 16
    MatrixStack& getWorldMatrix();
    ItemInHandRenderer& getItemInHandRenderer();
};

class ItemRenderer;

class Core {
    public:
    class Random {
        public:
        void _setSeed(unsigned int);
        float nextFloat();
    };
};

class Level {
    public:
    bool isClientSide() const;
    Core::Random* getRandom() const;
};

namespace mce {
    namespace Math {
        float sin(float);
    }
}


#endif //ITEMPHYSIC_RECOVERED_HPP