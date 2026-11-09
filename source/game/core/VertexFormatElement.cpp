#include "game/core/VertexFormatElement.hpp"

const VertexFormatElement::Usage* VertexFormatElement::Usage::POSITION = new Usage(L"Position", eUsage_Position);
const VertexFormatElement::Usage* VertexFormatElement::Usage::NORMAL = new Usage(L"Normal", eUsage_Normal);
const VertexFormatElement::Usage* VertexFormatElement::Usage::COLOR = new Usage(L"Vertex Color", eUsage_Color);
const VertexFormatElement::Usage* VertexFormatElement::Usage::UV = new Usage(L"UV", eUsage_Uv);
const VertexFormatElement::Usage* VertexFormatElement::Usage::BONE_MATRIX = new Usage(L"Bone Matrix",
                                                                                      eUsage_BoneMatrix);
const VertexFormatElement::Usage* VertexFormatElement::Usage::BLEND_WEIGHT = new Usage(L"Blend Weight",
                                                                                       eUsage_BlendWeight);
const VertexFormatElement::Usage* VertexFormatElement::Usage::PADDING = new Usage(L"Padding", eUsage_Padding);

const VertexFormatElement::Type* VertexFormatElement::Type::FLOAT = new Type(4, L"Float", 0, eType_Float);
const VertexFormatElement::Type* VertexFormatElement::Type::UBYTE = new Type(1, L"Unsigned Byte", 0,
                                                                             eType_UnsignedByte);
const VertexFormatElement::Type* VertexFormatElement::Type::BYTE = new Type(1, L"Byte", 0, eType_Byte);
const VertexFormatElement::Type* VertexFormatElement::Type::USHORT = new Type(2, L"Unsigned Short", 0,
                                                                              eType_UnsignedShort);
const VertexFormatElement::Type* VertexFormatElement::Type::SHORT = new Type(2, L"Short", 0, eType_Short);
const VertexFormatElement::Type* VertexFormatElement::Type::UINT = new Type(4, L"Unsigned Int", 0, eType_UnsignedInt);
const VertexFormatElement::Type* VertexFormatElement::Type::INT = new Type(4, L"Int", 0, eType_Int);

VertexFormatElement::Type::Type(int elementSize, std::wstring elementName, int elementGlType, EType elementType)
    : size(elementSize),
      name(elementName),
      glType(elementGlType),
      type(elementType)
{
}

int VertexFormatElement::Type::getSize() const
{
    return size;
}

VertexFormatElement::Usage::Usage(std::wstring usageName, EUsage usageId) : name(usageName), usage(usageId)
{
}

int VertexFormatElement::Usage::getId() const
{
    return static_cast<int>(usage);
}

VertexFormatElement::VertexFormatElement(const VertexFormatElement& source)
    : type(source.type),
      usage(source.usage),
      index(source.index),
      count(source.count)
{
}

VertexFormatElement::VertexFormatElement(int elementIndex, const Type* elementType, const Usage* elementUsage,
                                         int elementCount)
    : type(elementType),
      usage(elementUsage),
      index(elementIndex),
      count(elementCount)
{
    if (!supportsUsage(elementIndex, elementUsage))
    {
        usage = Usage::UV;
    }
}

bool VertexFormatElement::isPosition() const
{
    return usage == Usage::POSITION;
}
const VertexFormatElement::Usage* VertexFormatElement::getUsage() const
{
    return usage;
}
int VertexFormatElement::getIndex() const
{
    return index;
}
int VertexFormatElement::getByteSize() const
{
    return type->getSize() * count;
}
bool VertexFormatElement::supportsUsage(int elementIndex, const Usage* elementUsage) const
{
    return elementIndex == 0 || elementUsage == Usage::UV;
}
