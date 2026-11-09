#ifndef GAME_UI_FJ_LABEL_HUD_SPLASHBIG_HPP
#define GAME_UI_FJ_LABEL_HUD_SPLASHBIG_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_HUD_SplashBig : public FJ_Label
{
    friend class fuiRenderNodeTimeline;

public:
    explicit FJ_Label_HUD_SplashBig(fuiRenderNode* renderNode);
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void Init(const std::wstring& label);
    virtual void SetLabel(const std::wstring& label);

protected:
    bool m_animated;
};

#endif
