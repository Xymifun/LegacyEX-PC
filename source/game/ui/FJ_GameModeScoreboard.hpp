#ifndef GAME_UI_FJ_GAMEMODESCOREBOARD_HPP
#define GAME_UI_FJ_GAMEMODESCOREBOARD_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_GameModeScoreboard : public FJ_Document
{
private:
    int m_scoreboardType;
    std::vector<FJ_FuiNode*> m_titleSlots;
    std::vector<FJ_FuiNode*> m_statIconHolders;
    int m_statIconCount;
    int m_killedByLabelY;
    int m_killedByLabelHeight;

public:
    FJ_GameModeScoreboard(fuiRenderNode* renderNode);

    static FJ_GameModeScoreboard* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetupStatIconHolders();
    void SetScoreboardType(int type);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    virtual void handlePress(FJ_Event* event);
    void InitNumColumns(int numColumns);
    void SetTitleSlotText(int slot, const std::wstring& text);
    void ShowTitleSlot(int slot, bool show);
    void SetTitleIconTextureName(int slot, const std::wstring& textureName);
    void SetStatIconTextureName(int slot, const std::wstring& textureName);
    void SetHasPlayerBeenKilled(bool killed);
};

#endif
