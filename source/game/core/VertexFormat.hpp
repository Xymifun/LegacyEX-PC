#ifndef GAME_CORE_VERTEXFORMAT_HPP
#define GAME_CORE_VERTEXFORMAT_HPP

#include <string>
#include <vector>

class VertexFormatElement;

class VertexFormat
{
public:
    VertexFormat();
    VertexFormat(VertexFormat* other);
    int getElementCount();
    const VertexFormatElement* getElement(int index);
    int getVertexSize();
    void clear();
    bool hasNormal();
    int getNormalOffset();
    bool hasColor();
    int getColorOffset();
    bool hasUv(int index);
    int getUvOffset(int index);
    std::wstring toString();
    int getIntegerSize();
    std::vector<const VertexFormatElement*>& getElements();
    int getOffset(int index);
    bool equals(VertexFormat* other);
    bool hasPositionElement();
    VertexFormat* addElement(const VertexFormatElement* element);

private:
    std::vector<const VertexFormatElement*> elements;
    std::vector<int> offsets;
    int vertexSize;
    int colorOffset;
    std::vector<int> uvOffsets;
    int normalOffset;
};

#endif
