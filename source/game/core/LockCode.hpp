#ifndef GAME_CORE_LOCKCODE_HPP
#define GAME_CORE_LOCKCODE_HPP

#include <string>

class CompoundTag;

class LockCode
{
public:
    LockCode(const std::wstring& key);
    ~LockCode();

    bool isEmpty();
    std::wstring getKey();
    void addTag(CompoundTag* tag);

    static LockCode* fromTag(CompoundTag* tag);

    static LockCode* NO_LOCK;

private:
    std::wstring mKey;
};

#endif
