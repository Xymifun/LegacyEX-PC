#ifndef GAME_CORE_INFOBARMANAGER_HPP
#define GAME_CORE_INFOBARMANAGER_HPP

class BufferBuilder;

class InfoBarManager
{
public:
    enum eTimingCagegory
    {
        eTimingCagegory_Frame,
        eTimingCagegory_Terrain,
        eTimingCagegory_TerrainTransparent,
        eTimingCagegory_Entities,
        eTimingCagegory_BlockEntities,
        eTimingCagegory_Particles,
        eTimingCagegory_UITick,
        eTimingCagegory_UIRender,
        eTimingCagegory_Count
    };

    static void StartTimingCategory(eTimingCagegory eCategory);
    static void EndTimingCategory(eTimingCagegory eCategory);

    static void SetCommandBufferSize(float fSize);

    static void Render(float fWidth, float fHeight);

private:
    static void RenderQuad(float fX, float fY, float fWidth, float fHeight, unsigned int uColour,
                           BufferBuilder* pBufferBuilder);
};

#endif
