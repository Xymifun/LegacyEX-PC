#include "platform/wiiu/InputManager.hpp"
#include "platform/wiiu/CInput.hpp"

unsigned char InputManager_IsShowingErrorViewer()
{
    return g_pInputInstance->IsShowingErrorViewer();
}

unsigned char InputManager_IsShowingOnscreenKeyboard()
{
    return g_pInputInstance->IsShowingOnscreenKeyboard();
}
