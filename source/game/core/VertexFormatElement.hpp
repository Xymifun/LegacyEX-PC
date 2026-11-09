#ifndef GAME_CORE_VERTEX_FORMAT_ELEMENT_HPP
#define GAME_CORE_VERTEX_FORMAT_ELEMENT_HPP

#include <string>

class VertexFormatElement
{
public:
    class Type
    {
    public:
        enum EType
        {
            eType_Float,
            eType_UnsignedByte,
            eType_Byte,
            eType_UnsignedShort,
            eType_Short,
            eType_UnsignedInt,
            eType_Int
        };
        static const Type* FLOAT;
        static const Type* UBYTE;
        static const Type* BYTE;
        static const Type* USHORT;
        static const Type* SHORT;
        static const Type* UINT;
        static const Type* INT;
        Type(int size, std::wstring name, int glType, EType type);
        int getSize() const;
        std::wstring getName() const;
        int getGlType() const;
        int getId() const;

    private:
        int size;
        std::wstring name;
        int glType;
        EType type;
    };

    class Usage
    {
    public:
        enum EUsage
        {
            eUsage_Position,
            eUsage_Normal,
            eUsage_Color,
            eUsage_Uv,
            eUsage_BoneMatrix,
            eUsage_BlendWeight,
            eUsage_Padding
        };
        static const Usage* POSITION;
        static const Usage* NORMAL;
        static const Usage* COLOR;
        static const Usage* UV;
        static const Usage* BONE_MATRIX;
        static const Usage* BLEND_WEIGHT;
        static const Usage* PADDING;
        Usage(std::wstring name, EUsage usage);
        int getId() const;
        std::wstring getName() const;

    private:
        std::wstring name;
        EUsage usage;
    };

    VertexFormatElement(const VertexFormatElement& source);
    VertexFormatElement(int elementIndex, const Type* elementType, const Usage* elementUsage, int elementCount);
    bool isPosition() const;
    const Usage* getUsage() const;
    int getIndex() const;
    int getByteSize() const;
    bool supportsUsage(int elementIndex, const Usage* elementUsage) const;
    std::wstring toString() const;
    const Type* getType() const;
    int getCount() const;
    bool equals(const VertexFormatElement* other) const;


private:
    const Type* type;
    const Usage* usage;
    int index;
    int count;
};

#endif
