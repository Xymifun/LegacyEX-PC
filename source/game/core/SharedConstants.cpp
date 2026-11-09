#include "game/core/SharedConstants.hpp"

#include "game/core/Calendar.hpp"

const std::wstring SharedConstants::VERSION_STRING = L"1.12";
const std::wstring SharedConstants::DATA_VERSION_TAG = L"DataVersion";

std::wstring SharedConstants::ALLOWED_CHARACTERS = L"";

const float SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR = 30.25f;

bool SharedConstants::IS_HALLOWEEN = false;
bool SharedConstants::IS_HALLOWEEN_SEASON = false;
bool SharedConstants::IS_CHRISTMAS = false;
bool SharedConstants::IS_CHRISTMAS_SEASON = false;

void SharedConstants::staticCtor()
{
    ALLOWED_CHARACTERS =
        L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~\x00a0\x00c7\x00fc\x00e9\x00e2\x00e4\x00e0\x00e5\x00e7\x00ea\x00eb\x00e8\x00ef\x00ee\x00ec\x00c4\x00c5\x00c9\x00e6\x00c6\x00f4\x00f6\x00f2\x00fb\x00f9\x00ff\x00d6\x00dc\x00f8\x00a3\x00d8\x00d7\x0192\x00e1\x00ed\x00f3\x00fa\x00f1\x00d1\x00aa\x00ba\x00bf\x00ae\x00ac\x00bd\x00bc\x00a1\x00ab\x00bb\x00e3";

    IS_HALLOWEEN = Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() == 31;

    IS_HALLOWEEN_SEASON = (Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() >= 20u)
                          || (Calendar::GetMonth() == 11 && Calendar::GetDayOfMonth() <= 3u);

    IS_CHRISTMAS = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 24u)
                   || (Calendar::GetMonth() == 1 && Calendar::GetDayOfMonth() <= 1u);

    IS_CHRISTMAS_SEASON = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 17u)
                          && (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() <= 23u);
}

bool SharedConstants::isAllowedChatCharacter(char character)
{
    return character >= ' ' && character <= '?';
}
