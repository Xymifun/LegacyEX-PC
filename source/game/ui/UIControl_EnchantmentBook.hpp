#ifndef GAME_UI_UICONTROL_ENCHANTMENTBOOK_HPP
#define GAME_UI_UICONTROL_ENCHANTMENTBOOK_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/ui/UIControl.hpp"
#include "game/util/Random.hpp"

class BookModel;
class ItemInstance;
struct fuiRect;

class UIControl_EnchantmentBook : public UIControl
{
public:
    UIControl_EnchantmentBook();
    virtual ~UIControl_EnchantmentBook() {}

    void tickBook();
    void render(fuiRect* rect);

private:
    BookModel* mBookModel;
    int mBookModelAlignment;
    Random mRandom;
    int mBookTick;
    float mBookOpen;
    float mBookOpenPrev;
    float mBookOpenTarget;
    float mBookOpenSpeed;
    float mBookRotation;
    float mBookRotationPrev;
    not_null_ptr<ItemInstance> mItemInstance;
    int mUnused;
};

#endif
