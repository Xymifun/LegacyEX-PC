#include "game/core/DefaultVertexFormat.hpp"

VertexFormat* DefaultVertexFormat::BLOCK = new VertexFormat();
VertexFormat* DefaultVertexFormat::ITEM = new VertexFormat();
VertexFormat* DefaultVertexFormat::OLDMODEL_POSITION_TEX_NORMAL = new VertexFormat();
VertexFormat* DefaultVertexFormat::PARTICLE_POSITION_TEX_COLOR_LMAP = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_COLOR = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_TEX = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_NORMAL = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_TEX_COLOR = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_TEX_NORMAL = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_TEX_LIGHTMAP_COLOR = new VertexFormat();
VertexFormat* DefaultVertexFormat::POSITION_TEX_COLOR_NORMAL = new VertexFormat();

const VertexFormatElement* DefaultVertexFormat::POSITION_3F = NULL;
const VertexFormatElement* DefaultVertexFormat::COLOR_4UB = NULL;
const VertexFormatElement* DefaultVertexFormat::TEX_2F = NULL;
const VertexFormatElement* DefaultVertexFormat::TEX_2S = NULL;
const VertexFormatElement* DefaultVertexFormat::NORMAL_3B = NULL;
const VertexFormatElement* DefaultVertexFormat::PADDING_1B = NULL;

void DefaultVertexFormat::staticCtor()
{
    POSITION_3F = new VertexFormatElement(0, VertexFormatElement::Type::FLOAT, VertexFormatElement::Usage::POSITION, 3);
    COLOR_4UB = new VertexFormatElement(0, VertexFormatElement::Type::UBYTE, VertexFormatElement::Usage::COLOR, 4);
    TEX_2F = new VertexFormatElement(0, VertexFormatElement::Type::FLOAT, VertexFormatElement::Usage::UV, 2);
    TEX_2S = new VertexFormatElement(1, VertexFormatElement::Type::SHORT, VertexFormatElement::Usage::UV, 2);
    NORMAL_3B = new VertexFormatElement(0, VertexFormatElement::Type::BYTE, VertexFormatElement::Usage::NORMAL, 3);
    PADDING_1B = new VertexFormatElement(0, VertexFormatElement::Type::BYTE, VertexFormatElement::Usage::PADDING, 1);

    BLOCK->addElement(POSITION_3F);
    BLOCK->addElement(COLOR_4UB);
    BLOCK->addElement(TEX_2F);
    BLOCK->addElement(TEX_2S);

    ITEM->addElement(POSITION_3F);
    ITEM->addElement(COLOR_4UB);
    ITEM->addElement(TEX_2F);
    ITEM->addElement(NORMAL_3B);
    ITEM->addElement(PADDING_1B);

    OLDMODEL_POSITION_TEX_NORMAL->addElement(POSITION_3F);
    OLDMODEL_POSITION_TEX_NORMAL->addElement(TEX_2F);
    OLDMODEL_POSITION_TEX_NORMAL->addElement(NORMAL_3B);
    OLDMODEL_POSITION_TEX_NORMAL->addElement(PADDING_1B);

    PARTICLE_POSITION_TEX_COLOR_LMAP->addElement(POSITION_3F);
    PARTICLE_POSITION_TEX_COLOR_LMAP->addElement(TEX_2F);
    PARTICLE_POSITION_TEX_COLOR_LMAP->addElement(COLOR_4UB);
    PARTICLE_POSITION_TEX_COLOR_LMAP->addElement(TEX_2S);

    POSITION->addElement(POSITION_3F);

    POSITION_COLOR->addElement(POSITION_3F);
    POSITION_COLOR->addElement(COLOR_4UB);

    POSITION_TEX->addElement(POSITION_3F);
    POSITION_TEX->addElement(TEX_2F);

    POSITION_NORMAL->addElement(POSITION_3F);
    POSITION_NORMAL->addElement(NORMAL_3B);
    POSITION_NORMAL->addElement(PADDING_1B);

    POSITION_TEX_COLOR->addElement(POSITION_3F);
    POSITION_TEX_COLOR->addElement(TEX_2F);
    POSITION_TEX_COLOR->addElement(COLOR_4UB);

    POSITION_TEX_NORMAL->addElement(POSITION_3F);
    POSITION_TEX_NORMAL->addElement(TEX_2F);
    POSITION_TEX_NORMAL->addElement(NORMAL_3B);
    POSITION_TEX_NORMAL->addElement(PADDING_1B);

    POSITION_TEX_LIGHTMAP_COLOR->addElement(POSITION_3F);
    POSITION_TEX_LIGHTMAP_COLOR->addElement(TEX_2F);
    POSITION_TEX_LIGHTMAP_COLOR->addElement(TEX_2S);
    POSITION_TEX_LIGHTMAP_COLOR->addElement(COLOR_4UB);

    POSITION_TEX_COLOR_NORMAL->addElement(POSITION_3F);
    POSITION_TEX_COLOR_NORMAL->addElement(TEX_2F);
    POSITION_TEX_COLOR_NORMAL->addElement(COLOR_4UB);
    POSITION_TEX_COLOR_NORMAL->addElement(NORMAL_3B);
    POSITION_TEX_COLOR_NORMAL->addElement(PADDING_1B);
}
