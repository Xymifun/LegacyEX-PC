#ifndef GAME_ITEM_ITEMTRANSFORMS_HPP
#define GAME_ITEM_ITEMTRANSFORMS_HPP

class ItemInstance;
class ItemTransform;
class LivingEntity;

class ItemTransforms
{
public:
    static float offsetTranslateX;
    static float offsetTranslateY;
    static float offsetTranslateZ;
    static float offsetRotationX;
    static float offsetRotationY;
    static float offsetRotationZ;
    static float offsetScaleX;
    static float offsetScaleY;
    static float offsetScaleZ;

    enum TransformType
    {
        NONE,
        THIRD_PERSON_LEFT_HAND,
        THIRD_PERSON_RIGHT_HAND,
        FIRST_PERSON_LEFT_HAND,
        FIRST_PERSON_RIGHT_HAND,
        HEAD,
        GUI,
        GROUND,
        FIXED
    };

    ItemTransforms();
    ItemTransforms(const ItemTransforms& other);
    ItemTransforms(ItemTransform* thirdPersonLeftHand, ItemTransform* thirdPersonRightHand,
                   ItemTransform* firstPersonLeftHand, ItemTransform* firstPersonRightHand, ItemTransform* head,
                   ItemTransform* gui, ItemTransform* ground, ItemTransform* fixed);

    ItemTransform* getTransform(TransformType type) const;
    ItemTransforms* set_thirdperson_righthand(float rotationX, float rotationY, float rotationZ, float translationX,
                                              float translationY, float translationZ, float scaleX, float scaleY,
                                              float scaleZ);
    ItemTransforms* set_thirdperson_lefthand(float rotationX, float rotationY, float rotationZ, float translationX,
                                             float translationY, float translationZ, float scaleX, float scaleY,
                                             float scaleZ);
    ItemTransforms* set_thirdperson_hands(float rotationX, float rotationY, float rotationZ, float translationX,
                                          float translationY, float translationZ, float scaleX, float scaleY,
                                          float scaleZ);

    static ItemTransforms* GetDefaultTransforms();
    static ItemTransforms* getArmourStandTransforms(const ItemInstance* item, LivingEntity* entity);

    ItemTransforms* set_firstperson_lefthand(float rotationX, float rotationY, float rotationZ, float translationX,
                                             float translationY, float translationZ, float scaleX, float scaleY,
                                             float scaleZ);
    ItemTransforms* set_firstperson_righthand(float rotationX, float rotationY, float rotationZ, float translationX,
                                              float translationY, float translationZ, float scaleX, float scaleY,
                                              float scaleZ);
    ItemTransforms* set_ground(float rotationX, float rotationY, float rotationZ, float translationX,
                               float translationY, float translationZ, float scaleX, float scaleY, float scaleZ);
    ItemTransforms* set_fixed(float rotationX, float rotationY, float rotationZ, float translationX, float translationY,
                              float translationZ, float scaleX, float scaleY, float scaleZ);
    ItemTransforms* set_gui(float rotationX, float rotationY, float rotationZ, float translationX, float translationY,
                            float translationZ, float scaleX, float scaleY, float scaleZ);
    ItemTransforms* set_head(float rotationX, float rotationY, float rotationZ, float translationX, float translationY,
                             float translationZ, float scaleX, float scaleY, float scaleZ);
    ItemTransforms* set_firstperson_hands(float rotationX, float rotationY, float rotationZ, float translationX,
                                          float translationY, float translationZ, float scaleX, float scaleY,
                                          float scaleZ);

    static ItemTransforms* getTransforms(const ItemInstance* item, LivingEntity* entity);
    static void apply(ItemTransform* transform, bool leftHand);
    void apply(TransformType type);
    bool hasTransform(TransformType type);

private:
    ItemTransform* mThirdPersonLeftHand;
    ItemTransform* mThirdPersonRightHand;
    ItemTransform* mFirstPersonLeftHand;
    ItemTransform* mFirstPersonRightHand;
    ItemTransform* mHead;
    ItemTransform* mGui;
    ItemTransform* mGround;
    ItemTransform* mFixed;
};

#endif
