#include <nn/ffl/FFLiCreateID.h>

static inline FFLiCreateID* GetCreateID(FFLCreateID* pCreateID)
{
    return reinterpret_cast<FFLiCreateID*>(pCreateID);
}

static inline const FFLiCreateID* GetCreateID(const FFLCreateID* pCreateID)
{
    return reinterpret_cast<const FFLiCreateID*>(pCreateID);
}

BOOL FFLiIsNullMiiID(const FFLCreateID* pCreateID)
{
    if (pCreateID == NULL)
        return false;

    const u32 count = FFL_CREATE_ID_SIZE / sizeof(u16);
    NN_STATIC_ASSERT(count == 5);

    BOOL ret = false;
    if (pCreateID->value16[0] == 0 && pCreateID->value16[1] == 0 && pCreateID->value16[2] == 0
        && pCreateID->value16[3] == 0 && pCreateID->value16[4] == 0)
    {
        ret = true;
    }
    return ret;
}

void FFLiGetTemporaryMiiID(FFLCreateID* pCreateID)
{
    static const FFLiCreateID temporaryMiiID = {FFLI_CREATE_ID_FLAG_TEMPORARY};

    *GetCreateID(pCreateID) = temporaryMiiID;
}

BOOL FFLiIsNormalMiiID(const FFLCreateID* pCreateID)
{
    u8 flags = GetCreateID(pCreateID)->flags;
    return (flags & FFLI_CREATE_ID_FLAG_NORMAL) != 0;
}

BOOL FFLiIsSpecialMiiID(const FFLCreateID* pCreateID)
{
    return !FFLiIsNormalMiiID(pCreateID);
}

BOOL FFLiIsNTRMiiID(const FFLCreateID* pCreateID)
{
    const FFLiCreateID* pID = GetCreateID(pCreateID);
    BOOL ret = false;

    if (!(pID->flags & FFLI_CREATE_ID_FLAG_UNKNOWN_0) && (pID->flags & FFLI_CREATE_ID_FLAG_UNKNOWN_2))
        ret = true;

    return ret;
}

BOOL FFLiIsTemporaryMiiID(const FFLCreateID* pCreateID)
{
    u8 flags = GetCreateID(pCreateID)->flags;
    return (flags & FFLI_CREATE_ID_FLAG_TEMPORARY) != 0;
}

BOOL FFLiIsValidMiiID(const FFLCreateID* pCreateID)
{
    return !FFLiIsNullMiiID(pCreateID) && !FFLiIsTemporaryMiiID(pCreateID);
}

BOOL FFLiIsSameMiiID(const FFLCreateID* a, const FFLCreateID* b)
{
    if (FFLiIsValidMiiID(a) && FFLiIsValidMiiID(b))
    {
        const u32 count = FFL_CREATE_ID_SIZE / sizeof(u16);
        NN_STATIC_ASSERT(count == 5);

        const u16* pA = a->value16;
        const u16* pB = b->value16;

        BOOL ret = false;
        if (pA[0] == pB[0] && pA[1] == pB[1] && pA[2] == pB[2] && pA[3] == pB[3] && pA[4] == pB[4])
        {
            ret = true;
        }
        return ret;
    }

    return false;
}
