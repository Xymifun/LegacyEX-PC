#ifndef GAME_UTIL_NBTUTILS_HPP
#define GAME_UTIL_NBTUTILS_HPP

#include <string>

class BlockPos;
class BlockState;
class CompoundTag;
class Tag;

class NbtUtils
{
public:
    static bool compareNbt(Tag* first, Tag* second, bool compareLists);
    static BlockPos readBlockPos(CompoundTag* tag);
    static CompoundTag* writeBlockPos(const BlockPos& pos);
    static void mapToBlock(std::wstring& name, int& data);
    static const BlockState* readBlockState(CompoundTag* tag);
};

#endif
