#ifndef GAME_CORE_DEFAULTVERTEXFORMAT_HPP
#define GAME_CORE_DEFAULTVERTEXFORMAT_HPP

#include "game/core/VertexFormat.hpp"
#include "game/core/VertexFormatElement.hpp"

class DefaultVertexFormat
{
public:
    static VertexFormat* BLOCK;
    static VertexFormat* ITEM;
    static VertexFormat* OLDMODEL_POSITION_TEX_NORMAL;
    static VertexFormat* PARTICLE_POSITION_TEX_COLOR_LMAP;
    static VertexFormat* POSITION;
    static VertexFormat* POSITION_COLOR;
    static VertexFormat* POSITION_TEX;
    static VertexFormat* POSITION_NORMAL;
    static VertexFormat* POSITION_TEX_COLOR;
    static VertexFormat* POSITION_TEX_NORMAL;
    static VertexFormat* POSITION_TEX_LIGHTMAP_COLOR;
    static VertexFormat* POSITION_TEX_COLOR_NORMAL;

    static const VertexFormatElement* POSITION_3F;
    static const VertexFormatElement* COLOR_4UB;
    static const VertexFormatElement* TEX_2F;
    static const VertexFormatElement* TEX_2S;
    static const VertexFormatElement* NORMAL_3B;
    static const VertexFormatElement* PADDING_1B;

    static void staticCtor();
};

#endif
