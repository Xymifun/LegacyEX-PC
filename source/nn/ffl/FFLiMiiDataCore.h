#ifndef FFLI_MIIDATA_CORE_H_
#define FFLI_MIIDATA_CORE_H_

#include <nn/ffl/FFLCreateID.h>
#include <nn/ffl/FFLStandard.h>

#include <nn/ffl/FFLiAuthorID.h>

// Note: Keep in mind that bitfields are not portable.
//       The Wii U compiler orders fields in bitfield structs such that the first field is at the MSB.
//       However, the 3DS compiler orders fields in bitfield structs such that the first field is at the LSB.
//       Therefore, if you would like to use these structures for 3DS, make sure to reverse the order of fields in every bitfield struct.

class FFLiMiiDataCore
{
public:
    void SetMiiVersion(u8 version) { m_MiiVersion = version; }

    u8 MiiVersion() const { return m_MiiVersion; }

    void SetCopyable(bool enable) { m_Copyable = enable; }

    bool Copyable() const { return m_Copyable; }

    void SetNgWord(bool enable) { m_NgWord = enable; }

    bool NgWord() const { return m_NgWord; }

    void SetRegionMove(u16 regionMove) { m_RegionMove = regionMove; }

    u16 RegionMove() const { return m_RegionMove; }

    void SetLocalOnly(bool localOnly) { m_LocalOnly = localOnly; }

    bool LocalOnly() const { return m_LocalOnly; }

    void SetFontRegion(u16 region) { m_FontRegion = region; }

    u16 FontRegion() const { return m_FontRegion; }

    void SetPageIndex(u16 index) { m_PageIndex = index; }

    u16 PageIndex() const { return m_PageIndex; }

    void SetSlotIndex(u16 index) { m_SlotIndex = index; }

    u16 SlotIndex() const { return m_SlotIndex; }

    void SetFlagBit24To27(u16 value) { _0_24_27 = value; }

    u16 FlagBit24To27() const { return _0_24_27; }

    void SetBirthPlatform(u16 platform) { m_BirthPlatform = platform; }

    u16 BirthPlatform() const { return m_BirthPlatform; }

    FFLiAuthorID& AuthorID() { return m_AuthorID; }

    const FFLiAuthorID& AuthorID() const { return m_AuthorID; }

    FFLCreateID& CreatorID() { return m_CreatorID; }

    const FFLCreateID& CreatorID() const { return m_CreatorID; }

    u8* Padding1() { return _padding1; }

    static u32 Padding1Size() { return sizeof(u8) * 2; }

    void SetGender(u16 gender) { m_Gender = gender; }

    u16 Gender() const { return m_Gender; }

    void SetBirthMonth(u16 month) { m_BirthMonth = month; }

    u16 BirthMonth() const { return m_BirthMonth; }

    void SetBirthDay(u16 day) { m_BirthDay = day; }

    u16 BirthDay() const { return m_BirthDay; }

    void SetFavoriteColor(u16 value) { m_FavoriteColor = value; }

    u16 FavoriteColor() const { return m_FavoriteColor; }

    void SetFavoriteMii(u8 favoriteMii) { m_FavoriteMii = favoriteMii; }

    bool FavoriteMii() const { return m_FavoriteMii; }

    u16* Name() { return m_Name; }

    const u16* Name() const { return m_Name; }

    void SetHeight(s32 height) { m_Height = height; }

    s32 Height() const { return m_Height; }

    void SetBuild(s32 build) { m_Build = build; }

    s32 Build() const { return m_Build; }

    void SetFaceType(u16 value) { m_FaceType = value; }

    u16 FaceType() const { return m_FaceType; }

    void SetFacelineColor(u16 value) { m_FacelineColor = value; }

    u16 FacelineColor() const { return m_FacelineColor; }

    void SetFaceLine(u16 value) { m_FaceLine = value; }

    u16 FaceLine() const { return m_FaceLine; }

    void SetFaceMake(u16 value) { m_FaceMakeup = value; }

    u16 FaceMake() const { return m_FaceMakeup; }

    void SetHairType(u16 value) { m_HairType = value; }

    u16 HairType() const { return m_HairType; }

    void SetHairColor(u16 value) { m_HairColor = value; }

    u16 HairColor() const { return m_HairColor; }

    void SetHairDir(u16 value) { m_HairDir = value; }

    u16 HairDir() const { return m_HairDir; }

    void SetEyeType(u16 value) { m_EyeType = value; }

    u16 EyeType() const { return m_EyeType; }

    void SetEyeColor(u16 value) { m_EyeColor = value; }

    u16 EyeColor() const { return m_EyeColor; }

    void SetEyeScale(u16 value) { m_EyeScale = value; }

    u16 EyeScale() const { return m_EyeScale; }

    void SetEyeScaleY(u16 value) { m_EyeScaleY = value; }

    u16 EyeScaleY() const { return m_EyeScaleY; }

    void SetEyeRotate(u16 value) { m_EyeRotate = value; }

    u16 EyeRotate() const { return m_EyeRotate; }

    void SetEyeSpacing(u16 value) { m_EyeSpacingX = value; }

    u16 EyeSpacing() const { return m_EyeSpacingX; }

    void SetEyePosition(u16 value) { m_EyePositionY = value; }

    u16 EyePosition() const { return m_EyePositionY; }

    void SetEyebrowType(u16 value) { m_EyebrowType = value; }

    u16 EyebrowType() const { return m_EyebrowType; }

    void SetEyebrowColor(u16 value) { m_EyebrowColor = value; }

    u16 EyebrowColor() const { return m_EyebrowColor; }

    void SetEyebrowScale(u16 value) { m_EyebrowScale = value; }

    u16 EyebrowScale() const { return m_EyebrowScale; }

    void SetEyebrowScaleY(u16 value) { m_EyebrowScaleY = value; }

    u16 EyebrowScaleY() const { return m_EyebrowScaleY; }

    void SetEyebrowRotate(u16 value) { m_EyebrowRotate = value; }

    u16 EyebrowRotate() const { return m_EyebrowRotate; }

    void SetEyebrowSpacing(u16 value) { m_EyebrowSpacingX = value; }

    u16 EyebrowSpacing() const { return m_EyebrowSpacingX; }

    void SetEyebrowPosition(u16 value) { m_EyebrowPositionY = value; }

    u16 EyebrowPosition() const { return m_EyebrowPositionY; }

    void SetNoseType(u16 value) { m_NoseType = value; }

    u16 NoseType() const { return m_NoseType; }

    void SetNoseScale(u16 value) { m_NoseScale = value; }

    u16 NoseScale() const { return m_NoseScale; }

    void SetNosePosition(u16 value) { m_NosePositionY = value; }

    u16 NosePosition() const { return m_NosePositionY; }

    void SetMouthType(u16 value) { m_MouthType = value; }

    u16 MouthType() const { return m_MouthType; }

    void SetMouthColor(u16 value) { m_MouthColor = value; }

    u16 MouthColor() const { return m_MouthColor; }

    void SetMouthScale(u16 value) { m_MouthScale = value; }

    u16 MouthScale() const { return m_MouthScale; }

    void SetMouthScaleY(u16 value) { m_MouthScaleY = value; }

    u16 MouthScaleY() const { return m_MouthScaleY; }

    void SetMouthPosition(u16 value) { m_MouthPositionY = value; }

    u16 MouthPosition() const { return m_MouthPositionY; }

    void SetMustacheType(u16 value) { m_MustacheType = value; }

    u16 MustacheType() const { return m_MustacheType; }

    void SetBeardType(u16 value) { m_BeardType = value; }

    u16 BeardType() const { return m_BeardType; }

    void SetBeardColor(u16 value) { m_BeardColor = value; }

    u16 BeardColor() const { return m_BeardColor; }

    void SetMustacheScale(u16 value) { m_MustacheScale = value; }

    u16 MustacheScale() const { return m_MustacheScale; }

    void SetMustachePosition(u16 value) { m_MustachePositionY = value; }

    u16 MustachePosition() const { return m_MustachePositionY; }

    void SetGlassType(u16 value) { m_GlassType = value; }

    u16 GlassType() const { return m_GlassType; }

    void SetGlassColor(u16 value) { m_GlassColor = value; }

    u16 GlassColor() const { return m_GlassColor; }

    void SetGlassScale(u16 value) { m_GlassScale = value; }

    u16 GlassScale() const { return m_GlassScale; }

    void SetGlassPosition(u16 value) { m_GlassPositionY = value; }

    u16 GlassPosition() const { return m_GlassPositionY; }

    void SetMoleType(u16 value) { m_MoleType = value; }

    u16 MoleType() const { return m_MoleType; }

    void SetMoleScale(u16 value) { m_MoleScale = value; }

    u16 MoleScale() const { return m_MoleScale; }

    void SetMolePositionX(u16 value) { m_MolePositionX = value; }

    u16 MolePositionX() const { return m_MolePositionX; }

    void SetMolePositionY(u16 value) { m_MolePositionY = value; }

    u16 MolePositionY() const { return m_MolePositionY; }

    void ClearPadding0Bit31() { _0_31 = 0; }

    void ClearPadding0Bit14To15() { _0_14_15 = 0; }

    void ClearPadding32Bit12To15() { _32_12_15 = 0; }

    void ClearPadding36Bit14To15() { _36_14_15 = 0; }

    void ClearPadding38Bit15() { _38_15 = 0; }

    void ClearPadding3ABit14To15() { _3A_14_15 = 0; }

    void ClearPadding3CBit14To15() { _3C_14_15 = 0; }

    void ClearPadding40Bit8To15() { _40_8_15 = 0; }

    void ClearPadding42Bit15() { _42_15 = 0; }

    void ClearPadding46Bit15() { _46_15 = 0; }

    void SetPlaceHolder(s32 value) { m_PlaceHolder = value; }

    void SwapEndian();

private:
    u32 _0_31 : 1;
    u32 m_BirthPlatform : 3;
    u32 _0_24_27 : 4;
    u32 m_SlotIndex : 4;
    u32 m_PageIndex : 4;
    u32 _0_14_15 : 2;
    u32 m_FontRegion : 2;
    u32 m_RegionMove : 2;
    u32 m_NgWord : 1;
    u32 m_Copyable : 1;
    u32 m_MiiVersion : 8;

    union
    {
        FFLiAuthorID m_AuthorID;

        u8 _4[8];
    };

    union
    {
        FFLCreateID m_CreatorID;

        u8 _c[0xA];
    };
    u8 _padding1[2];

    u16 m_PlaceHolder : 1;
    u16 m_FavoriteMii : 1;
    u16 m_FavoriteColor : 4;
    u16 m_BirthDay : 5;
    u16 m_BirthMonth : 4;
    u16 m_Gender : 1;

    u16 m_Name[10];

    u8 m_Height;
    u8 m_Build;

    u16 m_FaceMakeup : 4;
    u16 m_FaceLine : 4;
    u16 m_FacelineColor : 3;
    u16 m_FaceType : 4;
    u16 m_LocalOnly : 1;
    u16 _32_12_15 : 4;
    u16 m_HairDir : 1;
    u16 m_HairColor : 3;
    u16 m_HairType : 8;

    u16 m_EyeScaleY : 3;
    u16 m_EyeScale : 4;
    u16 m_EyeColor : 3;
    u16 m_EyeType : 6;
    u16 _36_14_15 : 2;
    u16 m_EyePositionY : 5;
    u16 m_EyeSpacingX : 4;
    u16 m_EyeRotate : 5;

    u16 _38_15 : 1;
    u16 m_EyebrowScaleY : 3;
    u16 m_EyebrowScale : 4;
    u16 m_EyebrowColor : 3;
    u16 m_EyebrowType : 5;
    u16 _3A_14_15 : 2;
    u16 m_EyebrowPositionY : 5;
    u16 m_EyebrowSpacingX : 4;
    u16 m_EyebrowRotate : 5;

    u16 _3C_14_15 : 2;
    u16 m_NosePositionY : 5;
    u16 m_NoseScale : 4;
    u16 m_NoseType : 5;
    u16 m_MouthScaleY : 3;
    u16 m_MouthScale : 4;
    u16 m_MouthColor : 3;
    u16 m_MouthType : 6;

    u16 _40_8_15 : 8;
    u16 m_MustacheType : 3;
    u16 m_MouthPositionY : 5;
    u16 _42_15 : 1;
    u16 m_MustachePositionY : 5;
    u16 m_MustacheScale : 4;
    u16 m_BeardColor : 3;
    u16 m_BeardType : 3;

    u16 m_GlassPositionY : 5;
    u16 m_GlassScale : 4;
    u16 m_GlassColor : 3;
    u16 m_GlassType : 4;
    u16 _46_15 : 1;
    u16 m_MolePositionY : 5;
    u16 m_MolePositionX : 5;
    u16 m_MoleScale : 4;
    u16 m_MoleType : 1;
};
NN_STATIC_ASSERT_IS_POD(FFLiMiiDataCore);
NN_STATIC_ASSERT(sizeof(FFLiMiiDataCore) == 0x48);

class FFLiMiiDataOfficial : public FFLiMiiDataCore
{
public:
    u16* CreatorName() { return m_CreatorName; }

    const u16* CreatorName() const { return m_CreatorName; }

    void SwapEndian();

private:
    u16 m_CreatorName[10]; // Wstring
};
NN_STATIC_ASSERT_IS_POD(FFLiMiiDataOfficial);
NN_STATIC_ASSERT(sizeof(FFLiMiiDataOfficial) == 0x5C);

#define FFL_STOREDATA_SIZE (0x60)

class FFLiStoreData : public FFLiMiiDataOfficial
{
public:
    void SetCRC();
    void SwapEndian();

private:
    u16 _5c;
    u16 m_Crc;
};
NN_STATIC_ASSERT_IS_POD(FFLiStoreData);
NN_STATIC_ASSERT(sizeof(FFLiStoreData) == FFL_STOREDATA_SIZE);

struct FFLiStoreDataCFL : FFLStoreData
{
};
NN_STATIC_ASSERT_IS_POD(FFLiStoreDataCFL);
NN_STATIC_ASSERT(sizeof(FFLiStoreDataCFL) == FFL_STOREDATA_SIZE);

class FFLiMiiDataHidden : public FFLiMiiDataCore
{
public:
    void SwapEndian(); // Deleted in NSMBU

private:
    u32 _48[12 / sizeof(u32)]; // Actual type unknown
};
NN_STATIC_ASSERT_IS_POD(FFLiMiiDataHidden);
NN_STATIC_ASSERT(sizeof(FFLiMiiDataHidden) == 0x54);

class FFLiMiiDataCoreRFL
{
public:
    bool LocalOnly() const { return m_LocalOnly; }

    u16 Gender() const { return m_Gender; }

    u16 BirthMonth() const { return m_BirthMonth; }

    u16 BirthDay() const { return m_BirthDay; }

    u16 FavoriteColor() const { return m_FavoriteColor; }

    bool FavoriteMii() const { return m_FavoriteMii; }

    const u16* Name() const { return m_Name; }

    s32 Height() const { return m_Height; }

    s32 Build() const { return m_Build; }

    u16 FaceType() const { return m_FaceType; }

    u16 FacelineColor() const { return m_FacelineColor; }

    u16 FaceLineAndMake() const { return m_FaceLineAndMake; }

    u16 HairType() const { return m_HairType; }

    u16 HairColor() const { return m_HairColor; }

    u16 HairDir() const { return m_HairDir; }

    u16 EyebrowType() const { return m_EyebrowType; }

    u16 EyebrowRotate() const { return m_EyebrowRotate; }

    u16 EyebrowColor() const { return m_EyebrowColor; }

    u16 EyebrowScale() const { return m_EyebrowScale; }

    u16 EyebrowPosition() const { return m_EyebrowPositionY; }

    u16 EyebrowSpacing() const { return m_EyebrowSpacingX; }

    u16 EyeType() const { return m_EyeType; }

    u16 EyeRotate() const { return m_EyeRotate; }

    u16 EyePosition() const { return m_EyePositionY; }

    u16 EyeColor() const { return m_EyeColor; }

    u16 EyeScale() const { return m_EyeScale; }

    u16 EyeSpacing() const { return m_EyeSpacingX; }

    u16 NoseType() const { return m_NoseType; }

    u16 NoseScale() const { return m_NoseScale; }

    u16 NosePosition() const { return m_NosePositionY; }

    u16 MouthType() const { return m_MouthType; }

    u16 MouthColor() const { return m_MouthColor; }

    u16 MouthScale() const { return m_MouthScale; }

    u16 MouthPosition() const { return m_MouthPositionY; }

    u16 MustacheType() const { return m_MustacheType; }

    u16 BeardType() const { return m_BeardType; }

    u16 BeardColor() const { return m_BeardColor; }

    u16 MustacheScale() const { return m_MustacheScale; }

    u16 MustachePosition() const { return m_MustachePositionY; }

    u16 GlassType() const { return m_GlassType; }

    u16 GlassColor() const { return m_GlassColor; }

    u16 GlassScale() const { return m_GlassScale; }

    u16 GlassPosition() const { return m_GlassPositionY; }

    u16 MoleType() const { return m_MoleType; }

    u16 MoleScale() const { return m_MoleScale; }

    u16 MolePositionY() const { return m_MolePositionY; }

    u16 MolePositionX() const { return m_MolePositionX; }

public:
    u16 : 1;
    u16 m_Gender : 1;
    u16 m_BirthMonth : 4;
    u16 m_BirthDay : 5;
    u16 m_FavoriteColor : 4;
    u16 m_FavoriteMii : 1;

    u16 m_Name[10];
    u8 m_Height;
    u8 m_Build;
    u8 m_CreatorID[4];
    u8 m_SystemID[4];

    u16 m_FaceType : 3;
    u16 m_FacelineColor : 3;
    u16 m_FaceLineAndMake : 4;
    u16 : 3;
    u16 m_LocalOnly : 1;
    u16 : 2;

    u16 m_HairType : 7;
    u16 m_HairColor : 3;
    u16 m_HairDir : 1;
    u16 : 5;

    u16 m_EyebrowType : 5;
    u16 m_EyebrowRotate : 5;
    u16 : 6;
    u16 m_EyebrowColor : 3;
    u16 m_EyebrowScale : 4;
    u16 m_EyebrowPositionY : 5;
    u16 m_EyebrowSpacingX : 4;

    u16 m_EyeType : 6;
    u16 m_EyeRotate : 5;
    u16 m_EyePositionY : 5;
    u16 m_EyeColor : 3;
    u16 m_EyeScale : 4;
    u16 m_EyeSpacingX : 4;
    u16 m_EyeUnknown : 5;

    u16 m_NoseType : 4;
    u16 m_NoseScale : 4;
    u16 m_NosePositionY : 5;
    u16 m_NoseUnknown : 3;

    u16 m_MouthType : 5;
    u16 m_MouthColor : 2;
    u16 m_MouthScale : 4;
    u16 m_MouthPositionY : 5;

    u16 m_GlassType : 4;
    u16 m_GlassColor : 3;
    u16 m_GlassScale : 4;
    u16 m_GlassPositionY : 5;

    u16 m_MustacheType : 2;
    u16 m_BeardType : 2;
    u16 m_BeardColor : 3;
    u16 m_MustacheScale : 4;
    u16 m_MustachePositionY : 5;

    u16 m_MoleType : 1;
    u16 m_MoleScale : 4;
    u16 m_MolePositionY : 5;
    u16 m_MolePositionX : 5;
    u16 : 1;
};
NN_STATIC_ASSERT_IS_POD(FFLiMiiDataCoreRFL);
NN_STATIC_ASSERT(sizeof(FFLiMiiDataCoreRFL) == 0x36);

#endif // FFLI_MIIDATA_CORE_H_
