#include "game/entity/mob/EntitySkeletonSplitFix.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"

EntitySkeletonSplitFix::~EntitySkeletonSplitFix()
{
}

int EntitySkeletonSplitFix::getVersion()
{
    return 701;
}

CompoundTag* EntitySkeletonSplitFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"Skeleton"))
    {
        int skeletonType = tag->getInt(L"SkeletonType");

        if (skeletonType == 1)
        {
            tag->putString(L"id", L"WitherSkeleton");
        }
        else if (skeletonType == 2)
        {
            tag->putString(L"id", L"Stray");
        }

        delete tag->get(L"SkeletonType");
        tag->remove(L"SkeletonType");
    }

    return tag;
}
