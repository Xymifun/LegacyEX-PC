#ifndef GAME_UI_FJ_INGAMEHOSTOPTIONS_HPP
#define GAME_UI_FJ_INGAMEHOSTOPTIONS_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_InGameHostOptions : public FJ_Document
{
public:
    explicit FJ_InGameHostOptions(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void InitMenu();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
