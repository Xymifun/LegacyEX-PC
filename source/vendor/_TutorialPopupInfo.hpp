#ifndef SOURCE_VENDOR_TUTORIALPOPUPINFO_HPP
#define SOURCE_VENDOR_TUTORIALPOPUPINFO_HPP

#include "game/item/not_null_ptr.hpp"

class UIScene;
class ItemInstance;
class Tutorial;

class _TutorialPopupInfo
{
public:
    _TutorialPopupInfo();

    UIScene* scene;
    const wchar_t* title;
    const wchar_t* description;
    not_null_ptr<ItemInstance> icon;
    int iconItemId;
    int iconAuxValue;
    bool showIcon;
    bool showTitle;
    bool showExitScreenshot;
    int displayTime;
    Tutorial* tutorial;
};

#endif
