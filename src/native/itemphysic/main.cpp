#include <hook.h>
#include <symbol.h>
#include <nativejs.h>

#include <innercore/global_context.h>
#include <innercore/id_conversion_map.h>
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
ItemActor* ItemPhysicModule::currentRenderedItemActor = nullptr;


int ItemPhysicModule::getModelCount(ItemStackBase* stack) {
	unsigned short count = stack->getCount();
	if(count > 48) return 5;
	if(count > 32) return 4;
	if(count > 16) return 3;
	if(count > 1) return 2;
	return 1;
}


bool ItemPhysicModule::isConduitOrSkull(ItemStackBase* stack) {
	int id = IdConversion::dynamicToStatic(stack->getId(), IdConversion::ITEM);
	return id == -157 || id == 397;
}


void ItemPhysicModule::neutralizeVanillaRenderTransformations(BaseActorRenderContext* ctx, const MatrixStack::MatrixStackRef& ref, ItemActor* itemActor, float scaleFactor) {
	if(!itemActor->renderingMotionFlag) {
		float offsetY = mce::Math::sin(itemActor->transformationOffset + (((float) itemActor->life + ctx->tickDelta) / 10.0f)) * .1f + .1f;
		offsetY /= scaleFactor;
		float rotationYaw = (itemActor->transformationOffset + (((float) itemActor->life + ctx->tickDelta) / 20.0f)) * 57.296f;
		ref->translate(0.0f, -offsetY, 0.0f);
		ref->rotate(-rotationYaw, 0.0f, 1.0f, 0.0f);
	}
	// rotating the item 90 degrees on X axis, to make it look like lying on the ground
	ref->rotate(90.0f, 1.0f, 0.0f, 0.0f);
	// setting the rendered item rotation yaw
	ref->rotate(itemActor->rotation.y, 0.0f, 0.0f, 1.0f);
}


void ItemPhysicModule::applyRotations(const MatrixStack::MatrixStackRef& ref, ItemActor* itemActor, float scaleFactor, bool isItem3D, bool applyTranslations) {
	float rotateBy = (float)(getTimeNanoseconds() - lastTickTime) / 400000000.0f * ROTATE_SPEED * 57.296f;
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
	if(applyTranslations) {
		if(isItem3D) ref->translate(0.0f, .08f / scaleFactor, .2f / scaleFactor);
		else ref->translate(0.0f, -.05f / scaleFactor, 0.0f);
	}
	if(isItem3D && applyTranslations) ref->translate(0.0f, 0.0f, .2f / scaleFactor);
	ref->rotate(Rotation::get(itemActor), 0.0f, 1.0f, 0.0f);
	if(isItem3D && applyTranslations) ref->translate(0.0f, 0.0f, -.2f / scaleFactor);
}


bool ItemPhysicModule::render(BaseActorRenderContext* ctx, ItemActor* itemActor, float scaleFactor) {
	// conduit and all types of skulls are not supported currently
	if(isConduitOrSkull(&itemActor->itemStack)) return false;
	if(itemActor->life == 0) return false;
	auto rand = GlobalContext::getLevel()->getRandom();
	rand->_setSeed(itemActor->itemStack.isNull() ? 187 : itemActor->itemStack.getId() + itemActor->itemStack.getAuxValue());
	auto& mat = ctx->getWorldMatrix();
	auto& renderer = ctx->getItemInHandRenderer();
	auto upperRef = mat.push();
	neutralizeVanillaRenderTransformations(ctx, upperRef, itemActor, scaleFactor);
	bool isItem3D = renderer._canTessellateAsBlockItem(itemActor->itemStack);
	applyRotations(upperRef, itemActor, scaleFactor, isItem3D, true);
	if(!isItem3D && itemActor->itemStack.isBlock()) upperRef->translate(0.0f, 0.0f, .3f / scaleFactor);
	int itemCount = getModelCount(&itemActor->itemStack);
	if(!isItem3D) upperRef->translate(0.0f, 0.0f, -.09375f * (itemCount - 1) * .5f / scaleFactor);
	// rendering
	for(int i = 0; i < itemCount; ++i) {
		auto ref = mat.push();
		if(i > 0 && isItem3D) {
			float tx = (rand->nextFloat() * 2.0f - 1.0f) * .15f / scaleFactor;
			float ty = (rand->nextFloat() * 2.0f - 1.0f) * .15f / scaleFactor;
			float tz = (rand->nextFloat() * 2.0f - 1.0f) * .15f / scaleFactor;
			ref->translate(tx, ty, tz);
		}
		renderer.renderItem(*ctx, *itemActor, itemActor->itemStack, false, (ItemContextFlags) 2, false);
		ref.release();
		if(!isItem3D) upperRef->translate(0.0f, 0.0f, .09375f / scaleFactor);
	}
	upperRef.release();
	return true;
}


void ItemPhysicModule::initialize() {

	DLHandleManager::initializeHandle("libminecraftpe.so", "mcpe");

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

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN12ItemRenderer16_renderItemGroupER22BaseActorRenderContextR9ItemActorif"),
		LAMBDA((HookManager::CallbackController* controller, ItemRenderer* renderer, BaseActorRenderContext& ctx, ItemActor& itemActor, int itemCount, float scaleFactor), {
			if(render(&ctx, &itemActor, scaleFactor)) {
				controller->prevent();
			}
		}, ),
		HookManager::CALL | HookManager::LISTENER | HookManager::CONTROLLER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN12ItemRenderer6renderER22BaseActorRenderContextR15ActorRenderData"),
		LAMBDA((ItemRenderer* renderer, BaseActorRenderContext& ctx, ActorRenderData& data), {
			currentRenderedItemActor = data.actor;
		}, ),
		HookManager::CALL | HookManager::LISTENER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN12ItemRenderer6renderER22BaseActorRenderContextR15ActorRenderData"),
		LAMBDA((ItemRenderer* renderer, BaseActorRenderContext& ctx, ActorRenderData& data), {
			currentRenderedItemActor = nullptr;
		}, ),
		HookManager::RETURN | HookManager::LISTENER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN14BannerRenderer6renderER22BaseActorRenderContextR20BlockActorRenderData"),
		LAMBDA((BannerRenderer* renderer, BaseActorRenderContext& ctx, BlockActorRenderData& data), {
			if(currentRenderedItemActor != nullptr) {
				auto ref = ctx.getWorldMatrix().push();
				neutralizeVanillaRenderTransformations(&ctx, ref, currentRenderedItemActor, .25f);
				applyRotations(ref, currentRenderedItemActor, .25f, false, false);
			}
		}, ),
		HookManager::CALL | HookManager::LISTENER
	);
	
	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN14BannerRenderer6renderER22BaseActorRenderContextR20BlockActorRenderData"),
		LAMBDA((BannerRenderer* renderer, BaseActorRenderContext& ctx, BlockActorRenderData& data), {
			if(currentRenderedItemActor != nullptr) ctx.getWorldMatrix().pop();
		}, ),
		HookManager::RETURN | HookManager::LISTENER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN14ShieldRenderer6renderER22BaseActorRenderContextR15ActorRenderData"),
		LAMBDA((ShieldRenderer* renderer, BaseActorRenderContext& ctx, ActorRenderData& data), {
			if(currentRenderedItemActor != nullptr) {
				auto ref = ctx.getWorldMatrix().push();
				neutralizeVanillaRenderTransformations(&ctx, ref, data.actor, .25f);
				applyRotations(ref, data.actor, .25f, false, false);
			}
		}, ),
		HookManager::CALL | HookManager::LISTENER
	);

	HookManager::addCallback(
		SYMBOL("mcpe", "_ZN14ShieldRenderer6renderER22BaseActorRenderContextR15ActorRenderData"),
		LAMBDA((ShieldRenderer* renderer, BaseActorRenderContext& ctx, ActorRenderData& data), {
			if(currentRenderedItemActor != nullptr) ctx.getWorldMatrix().pop();
		}, ),
		HookManager::RETURN | HookManager::LISTENER
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