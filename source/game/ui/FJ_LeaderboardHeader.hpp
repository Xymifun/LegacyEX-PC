#ifndef GAME_UI_FJ_LEADERBOARDHEADER_HPP
#define GAME_UI_FJ_LEADERBOARDHEADER_HPP

#include <string>
#include "game/ui/FJ_FuiNode.hpp"

class FJ_LeaderboardHeader : public FJ_FuiNode
{
protected:
    FJ_FuiNode* m_icons[7];
    int m_entryPositions[7];
    float m_rankTextWidth;
    float m_gamertagTextWidth;
    bool m_small;
    int m_slotWidth;
    int m_slotAreaWidth;

public:
    FJ_LeaderboardHeader(fuiRenderNode* pRenderNode);
    virtual ~FJ_LeaderboardHeader() {}
    static FJ_LeaderboardHeader* Create(fuiRenderNode* pRenderNode);
    void Init(int count, FJ_FuiNode* parent, int x, int y);
    void SetupHeaders(std::wstring rank, std::wstring gamertag);
    fuiRenderNode* GetBitmapData(int type);
    void SetupSlot(int slot, int type);
    unsigned int getFirstEntryPos();
    int getEntrySpacing();
    unsigned int getIconHolderHalfWidth();
    void Destroy();
    virtual unsigned int getType();
    virtual void ASConstructor();
};

#endif
