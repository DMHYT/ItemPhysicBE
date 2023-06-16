declare function WRAP_NATIVE(module: "ItemPhysicConfig"): {
    setRotateSpeed(speed: number): void;
}

WRAP_NATIVE("ItemPhysicConfig").setRotateSpeed(__config__.getInteger("rotateSpeed"));