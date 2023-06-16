#include <hook.h>
#include <symbol.h>
#include <nativejs.h>

#include "main.hpp"


float ItemPhysicModule::ROTATE_SPEED = 1.0f;


void ItemPhysicModule::initialize() {

	DLHandleManager::initializeHandle("libminecraftpe.so", "mcpe");

	// TODO

}


MAIN {
	Module* main_module = new ItemPhysicModule();
}


JS_MODULE_VERSION(ItemPhysicConfig, 1);
JS_EXPORT(ItemPhysicConfig, setRotateSpeed, "V(I)", (JNIEnv*, int speed) {
	ItemPhysicModule::ROTATE_SPEED = (float) speed;
	return 0;
});