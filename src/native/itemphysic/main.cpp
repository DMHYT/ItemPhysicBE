#include <hook.h>
#include <symbol.h>
#include <nativejs.h>

#include <innercore/global_context.h>
#include <innercore/vtable.h>

#include "main.hpp"


std::unordered_map<long long, float> ItemPhysicModule::Rotation::data;

float ItemPhysicModule::Rotation::get(Actor* actor) {
	auto uid = actor->getUniqueID();
	if(uid == nullptr) return 0.0f;
	if(data.find(uid->id) == data.end()) data[uid->id] = 0.0f;
	return data.at(uid->id);
}

void ItemPhysicModule::Rotation::set(Actor* actor, float xRot) {
	auto uid = actor->getUniqueID();
	if(uid != nullptr) data[uid->id] = xRot;
}

void ItemPhysicModule::Rotation::remove(Actor* actor) {
	auto uid = actor->getUniqueID();
	if(uid != nullptr && data.find(uid->id) != data.end()) data.erase(uid->id);
}


float ItemPhysicModule::ROTATE_SPEED = 1.0f;
long long ItemPhysicModule::lastTickTime = 0ll;


bool ItemPhysicModule::is3D(ItemStackBase* stack) {
	if(stack->isBlock()) return true;
	auto item = stack->getItem();
	if(item == nullptr) return false;
	VTABLE_FIND_OFFSET(Item_getBlockShape, _ZTV4Item, _ZNK4Item13getBlockShapeEv);
	return VTABLE_CALL<int>(Item_getBlockShape, item) != -1;
}


bool ItemPhysicModule::render(BaseActorRenderContext* ctx, ItemActor* itemActor, int itemCount, float scaleFactor) {
	if(itemActor->life == 0) return false;
	auto rand = GlobalContext::getLevel()->getRandom();
	rand->_setSeed(itemActor->itemStack.isNull() ? 187 : itemActor->itemStack.getId() + itemActor->itemStack.getAuxValue());
	auto& mat = ctx->getWorldMatrix();
	auto& renderer = ctx->getItemInHandRenderer();
	auto upperRef = mat.push();
	// neutralizing vanilla item rotation and floating
	if(!itemActor->renderingMotionFlag) {
		float offsetY = mce::Math::sin(itemActor->transformationOffset + (((float) itemActor->life + ctx->tickDelta) / 10.0f)) * .1f + .1f;
		offsetY /= scaleFactor;
		float rotationYaw = (itemActor->transformationOffset + (((float) itemActor->life + ctx->tickDelta) / 20.0f)) * 57.296f;
		upperRef->translate(0.0f, -offsetY, 0.0f);
		upperRef->rotate(-rotationYaw, 0.0f, 1.0f, 0.0f);
	}
	// rotating the item 90 degrees on X axis, to make it look like lying on the ground
	upperRef->rotate(90.0f, 1.0f, 0.0f, 0.0f);
	// setting the rendered item rotation yaw
	upperRef->rotate(itemActor->rotation.y, 0.0f, 0.0f, 1.0f);
	// handling rotations
	bool isItem3D = is3D(&itemActor->itemStack);
	float rotateBy = (float)(getTimeNanoseconds() - lastTickTime) / 200000000.0f * ROTATE_SPEED * 57.296f;
	// slowing down the rotation when the item entity movement is slowed down by a block like cobweb, sweet berry bush etc.
	if(itemActor->stuckX != 0.0f || itemActor->stuckY != 0.0f || itemActor->stuckZ != 0.0f)
		rotateBy *= itemActor->stuckX * .2f;
	if(itemActor->onGround) {
		Rotation::set(itemActor, 0.0f);
	} else {
		rotateBy *= 2.0f;
		// TODO: working with fluid viscosity
		Rotation::set(itemActor, Rotation::get(itemActor) + rotateBy);
	}
	if(isItem3D) upperRef->translate(0.0f, -.2f, -.08f);
	else upperRef->translate(0.0f, 0.0f, -.04f);
	if(isItem3D) upperRef->translate(0.0f, .2f, 0.0f);
	upperRef->rotate(Rotation::get(itemActor), 0.0f, 1.0f, 0.0f);
	if(isItem3D) upperRef->translate(0.0f, -.2f, 0.0f);
	if(!isItem3D) upperRef->translate(0.0f, 0.0f, -.09375f * (itemCount - 1) * .5f);
	// rendering
	for(int i = 0; i < itemCount; ++i) {
		auto ref = mat.push();
		if(i > 0) {
			float tx = (rand->nextFloat() * 2.0f - 1.0f) * .15f;
			float ty = (rand->nextFloat() * 2.0f - 1.0f) * .15f;
			float tz = (rand->nextFloat() * 2.0f - 1.0f) * .15f;
			ref->translate(tx, ty, tz);
		}
		renderer.renderItem(*ctx, *itemActor, itemActor->itemStack, false, (ItemContextFlags) 2, false);
		ref.release();
		upperRef->translate(0.0f, 0.0f, .09375f);
	}
	upperRef.release();
	return true;
}


void ItemPhysicModule::initialize() {

	DLHandleManager::initializeHandle("libminecraftpe.so", "mcpe");

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN12ItemRenderer16_renderItemGroupER22BaseActorRenderContextR9ItemActorif"),
		LAMBDA((HookManager::CallbackController* controller, ItemRenderer* renderer, BaseActorRenderContext& ctx, ItemActor& itemActor, int itemCount, float scaleFactor), {
			if(render(&ctx, &itemActor, itemCount, scaleFactor)) {
				controller->prevent();
			}
		}, ),
		HookManager::CALL | HookManager::LISTENER | HookManager::CONTROLLER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN16InGamePlayScreen6renderER13ScreenContextRK17FrameRenderObject"),
		LAMBDA((void* inGamePlayScreen, void* screenContext, void* frameRenderObject), {
			lastTickTime = getTimeNanoseconds();
		}, ),
		HookManager::RETURN | HookManager::LISTENER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN5Level22removeEntityReferencesER5Actorb"),
		LAMBDA((Level* level, Actor& removedActor), {
			if(level->isClientSide()) {
				Rotation::remove(&removedActor);
			}
		}, ),
		HookManager::CALL | HookManager::LISTENER
	);

}


MAIN {
	Module* main_module = new ItemPhysicModule();
}


JS_MODULE_VERSION(ItemPhysicConfig, 1);
JS_EXPORT(ItemPhysicConfig, setRotateSpeed, "V(I)", (JNIEnv*, int speed) {
	ItemPhysicModule::ROTATE_SPEED = (float) speed;
	return 0;
});