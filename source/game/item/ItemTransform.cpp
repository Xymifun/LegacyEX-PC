#include "game/item/ItemTransform.hpp"

#include "game/core/Mth.hpp"

ItemTransform* ItemTransform::NO_TRANSFORM = new ItemTransform(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f),
                                                               Vector3f(1.0f, 1.0f, 1.0f), false);

ItemTransform::ItemTransform(Vector3f rotation, Vector3f translation, Vector3f scale, bool clampValues)
{
    this->rotation = rotation;
    this->translation = translation;
    this->scale = scale;
    if (clampValues)
    {
        this->translation = Vector3f(Mth::clamp(translation.x * 0.0625f, -5.0f, 5.0f),
                                     Mth::clamp(translation.y * 0.0625f, -5.0f, 5.0f),
                                     Mth::clamp(translation.z * 0.0625f, -5.0f, 5.0f));
        this->scale = Vector3f(Mth::clamp(scale.x, -4.0f, 4.0f), Mth::clamp(scale.y, -4.0f, 4.0f),
                               Mth::clamp(scale.z, -4.0f, 4.0f));
    }
}

ItemTransform* ItemTransform::deserialize(float rotationX, float rotationY, float rotationZ, float translationX,
                                          float translationY, float translationZ, float scaleX, float scaleY,
                                          float scaleZ)
{
    Vector3f rotation(rotationX, rotationY, rotationZ);
    Vector3f translation(translationX, translationY, translationZ);
    Vector3f scale(scaleX, scaleY, scaleZ);
    return new ItemTransform(rotation, translation, scale, true);
}
