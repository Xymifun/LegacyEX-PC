#include <nn/ffl/FFLVec.h>

#include <nn/ffl/FFLiCoordinate.h>

FFLiCoordinate::FFLiCoordinate()
    : m_SwizzleY(1),
      m_SwizzleZ(2),
      m_SwizzleX(0),
      m_FlipY(false),
      m_FlipZ(false),
      m_FlipX(false),
      m_Scale(1.0f)
{
}

FFLiCoordinate::~FFLiCoordinate()
{
}

f32 FFLiCoordinate::FlipValue(FFLVec3*, f32 value)
{
    return -value;
};

template <typename ValueType, typename VectorType>
void FFLiCoordinate::TransformImpl(VectorType* pVec) const
{
    ValueType values[3] = {pVec->x, pVec->y, pVec->z};

    ValueType x = m_FlipX ? FlipValue(pVec, values[0]) : values[0];
    ValueType y = m_FlipY ? FlipValue(pVec, values[1]) : values[1];
    ValueType z = m_FlipZ ? FlipValue(pVec, values[2]) : values[2];

    values[m_SwizzleX] = x;
    values[m_SwizzleY] = y;
    values[m_SwizzleZ] = z;

    pVec->x = values[0];
    pVec->y = values[1];
    pVec->z = values[2];
}

void FFLiCoordinate::Transform(FFLVec3* pVec) const
{
    TransformImpl<f32>(pVec);
    pVec->x *= m_Scale;
    pVec->y *= m_Scale;
    pVec->z *= m_Scale;
}

void FFLiCoordinate::Transform(FFLVec4* pVec) const
{
    Transform((FFLVec3*)pVec);
}

f32 FFLiCoordinate::FlipValue(FFLVec4*, f32 value)
{
    return -value;
};

void FFLiCoordinate::TransformWithoutScale(FFLVec4* pVec) const
{
    TransformWithoutScale((FFLVec3*)pVec);
}

void FFLiCoordinate::TransformWithoutScale(FFLVec3* pVec) const
{
    TransformImpl<f32>(pVec);
}

u32 FFLiCoordinate::FlipValue(FFLiSnorm10_10_10_2* pVec, u32 value)
{
    if (value == 0x200)
        return 0x1FF;

    else if (value == 0x1FF)
        return 0x200;

    else
        return 0x400 - value & 0x3ff;
}

void FFLiCoordinate::TransformWithoutScale(FFLiSnorm10_10_10_2* pVec) const
{
    TransformImpl<u32>(pVec);
}

s8 FFLiCoordinate::FlipValue(FFLiSnorm8_8_8_8* pVec, s8 value)
{
    if (value == -128)
        value = 127;

    return -value;
}

void FFLiCoordinate::TransformWithoutScale(FFLiSnorm8_8_8_8* pVec) const
{
    TransformImpl<s8>(pVec);
}

bool FFLiCoordinate::IsDefault() const
{
    if (m_FlipY)
        return false;

    if (m_FlipZ)
        return false;

    if (m_FlipX)
        return false;

    if (m_SwizzleY != 1)
        return false;

    if (m_SwizzleZ != 2)
        return false;

    if (m_SwizzleX != 0)
        return false;

    if (m_Scale != 1.0f)
        return false;

    return true;
}
