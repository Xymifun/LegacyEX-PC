#ifndef GAME_UI_FJ_COMPONENTLOGO_HPP
#define GAME_UI_FJ_COMPONENTLOGO_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_ComponentLogo : public FJ_Document
{
public:
    FJ_ComponentLogo(fuiRenderNode* renderNode);
    void SetLogoLanguage(int language);
    static FJ_ComponentLogo* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();

private:
    int m_logoLanguage;
};

#endif
