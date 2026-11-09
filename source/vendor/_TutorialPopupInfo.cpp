#include "vendor/_TutorialPopupInfo.hpp"

#include "game/item/ItemInstance.hpp"

_TutorialPopupInfo::_TutorialPopupInfo()
    : scene(0),
      title(L""),
      description(L""),
      icon(ItemInstance::EMPTY),
      iconItemId(-1),
      iconAuxValue(0),
      showIcon(false),
      showTitle(true),
      showExitScreenshot(false),
      displayTime(-1),
      tutorial(0)
{
}
