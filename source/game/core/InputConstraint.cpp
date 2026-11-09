#include "game/core/InputConstraint.hpp"

#include "game/core/GameSettings.hpp"
#include "platform/wiiu/CInput.hpp"

InputConstraint::~InputConstraint()
{
}

int InputConstraint::getType()
{
    return 0;
}

bool InputConstraint::isMappingConstrained(int playerId, int mapping)
{
    if (constrainedMapping == mapping || mapping < EControllerActions_Scoreboard)
        return true;

    return (g_pInput->GetGameJoypadMaps(playerId, constrainedMapping) & g_pInput->GetGameJoypadMaps(playerId, mapping))
           != 0;
}
