#ifndef GAME_UI_FJ_DRAGONHEALTH_HPP
#define GAME_UI_FJ_DRAGONHEALTH_HPP

#include <string>

#include "game/ui/FJ_ProgressBar.hpp"

class FJ_Label;
class fuiRenderNode;

class FJ_DragonHealth : public FJ_ProgressBar
{
public:
    explicit FJ_DragonHealth(fuiRenderNode* renderNode);
    virtual ~FJ_DragonHealth() {}
    virtual void Init(const std::wstring& label);
    virtual void SetLabel(const std::wstring& label);

private:
    FJ_Label* m_pLabel;
};

#endif
