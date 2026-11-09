#include <nn/ffl/FFLAdditionalInfo.h>

#include <nn/ffl/FFLiAdditionalInfo.h>
#include <nn/ffl/FFLiAuthorID.h>
#include <nn/ffl/FFLiColor.h>
#include <nn/ffl/FFLiMisc.h>
#include <nn/ffl/FFLiUtil.h>

#include <nn/ffl/detail/FFLiCharInfo.h>

#include <cstring>

namespace
{

void ReplaceInvalidNameChar(u16* s, s32 len)
{
    for (s32 i = 0; i < len; i++)
    {
        if (s[i] == L'\0')
            return;

        if (s[i] < L' ' || s[i] == 0x7F)
            s[i] = L'?';
    }
}

bool IsAvailableFontRegion(FFLFontRegion a, FFLFontRegion b)
{
    return a == b;
}

void ReplaceNonAsciiNameChar(u16* s, s32 len)
{
    for (s32 i = 0; i < len; i++)
    {
        if (s[i] == L'\0')
            return;

        if (s[i] >= 0x80)
            s[i] = L'?';
    }
}

}

void FFLiGetAdditionalInfo(FFLAdditionalInfo* pAdditionalInfo, const FFLiCharInfo* pCharInfo, BOOL checkFontRegion,
                           FFLFontRegion fontRegion)
{
    if (pCharInfo->ngWord && !FFLiIsHomeAuthorID(&pCharInfo->authorID))
    {
        FFLiCopyWcharT2U16(pAdditionalInfo->name, 10 + 1, L"???");
        FFLiCopyWcharT2U16(pAdditionalInfo->creatorName, 10 + 1, L"???");
    }
    else
    {
        std::memcpy(pAdditionalInfo->name, pCharInfo->name, sizeof(u16) * (10 + 1));
        pAdditionalInfo->name[10] = L'\0';

        std::memcpy(pAdditionalInfo->creatorName, pCharInfo->creatorName, sizeof(u16) * (10 + 1));
        pAdditionalInfo->creatorName[10] = L'\0';

        ReplaceInvalidNameChar(pAdditionalInfo->name, 10);
        ReplaceInvalidNameChar(pAdditionalInfo->creatorName, 10);

        if (checkFontRegion && !IsAvailableFontRegion(pCharInfo->fontRegion, fontRegion))
        {
            ReplaceNonAsciiNameChar(pAdditionalInfo->name, 10);
            ReplaceNonAsciiNameChar(pAdditionalInfo->creatorName, 10);
        }
    }

    std::memcpy(&pAdditionalInfo->creatorID, &pCharInfo->creatorID, sizeof(FFLCreateID));

    pAdditionalInfo->gender = pCharInfo->gender;

    if (FFLiVerifyBirthday(pCharInfo->birthMonth, pCharInfo->birthDay))
    {
        pAdditionalInfo->birthMonth = pCharInfo->birthMonth;
        pAdditionalInfo->birthDay = pCharInfo->birthDay;
    }
    else
    {
        pAdditionalInfo->birthMonth = 0;
        pAdditionalInfo->birthDay = 0;
    }

    pAdditionalInfo->favoriteColor = pCharInfo->favoriteColor;

    u8 height = pCharInfo->height;
    u8 build = pCharInfo->build;

    if (height > 127)
        height = 127;

    if (build > 127)
        build = 127;

    pAdditionalInfo->height = height;
    pAdditionalInfo->build = build;

    pAdditionalInfo->ngWord = bool(pCharInfo->ngWord);
    pAdditionalInfo->fontRegion = pCharInfo->fontRegion;
    pAdditionalInfo->faceType = pCharInfo->parts.faceType;
    pAdditionalInfo->hairDir = pCharInfo->parts.hairDir;
    pAdditionalInfo->hairType = pCharInfo->parts.hairType;
    pAdditionalInfo->facelineColor = FFLiGetFacelineColor(pCharInfo->parts.facelineColor);
}
