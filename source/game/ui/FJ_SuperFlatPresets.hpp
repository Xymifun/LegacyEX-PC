#ifndef GAME_UI_FJ_SUPERFLATPRESETS_HPP
#define GAME_UI_FJ_SUPERFLATPRESETS_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SuperFlatPresets : public FJ_Document
{
public:
    FJ_SuperFlatPresets(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
