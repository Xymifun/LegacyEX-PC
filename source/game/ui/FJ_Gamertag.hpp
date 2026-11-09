#ifndef GAME_UI_FJ_GAMERTAG_HPP
#define GAME_UI_FJ_GAMERTAG_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_Gamertag : public FJ_Document
{
public:
    FJ_Gamertag(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetGamertag(const std::wstring& gamertag);
};

#endif
