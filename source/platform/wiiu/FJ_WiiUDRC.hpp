#ifndef PLATFORM_WIIU_FJ_WIIUDRC_HPP
#define PLATFORM_WIIU_FJ_WIIUDRC_HPP

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class fuiRenderNode;

void HandleDRCStatusChanges();

class FJ_WiiUDRC : public FJ_FuiNode
{
private:
    enum
    {
        MAX_TILES = 24
    };

    FJ_FuiNode* m_tiles[MAX_TILES];
    int m_tileCount;
    int m_tilesAcross;
    int m_tilesDown;
    bool m_backgroundVisible;

public:
    FJ_WiiUDRC(fuiRenderNode* pRenderNode);

    void UpdateTileBackground(FJ_Event* pEvent);
    static void _UpdateTileBackground(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void ShowDRCBackground(bool bShow);
    void ShowDRCIcon(bool bOn, bool bConnected);
    void SetSafeZone(int iTop, int iBottom, int iLeft, int iRight);
    static FJ_WiiUDRC* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    int GetIconX();
    int GetIconY();
    int GetIconWidth();
    int GetIconHeight();
    void SetAlpha(float fAlpha);
};

#endif
