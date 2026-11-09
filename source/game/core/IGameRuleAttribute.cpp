#include "game/core/IGameRuleAttribute.hpp"

#include <cwchar>

const wchar_t* gRuleAttributeNames[] = {L"descriptionName",
                                        L"promptName",
                                        L"dataTag",
                                        L"enchantmentId",
                                        L"enchantmentLevel",
                                        L"itemId",
                                        L"quantity",
                                        L"auxValue",
                                        L"slot",
                                        L"name",
                                        L"food",
                                        L"health",
                                        L"blockId",
                                        L"useCoords",
                                        L"seed",
                                        L"flatworld",
                                        L"filename",
                                        L"rot",
                                        L"data",
                                        L"block",
                                        L"entity",
                                        L"facing",
                                        L"edgeBlock",
                                        L"fillBlock",
                                        L"skipAir",
                                        L"x",
                                        L"x0",
                                        L"x1",
                                        L"y",
                                        L"y0",
                                        L"y1",
                                        L"z",
                                        L"z0",
                                        L"z1",
                                        L"chunkX",
                                        L"chunkZ",
                                        L"yRot",
                                        L"xRot",
                                        L"spawnX",
                                        L"spawnY",
                                        L"spawnZ",
                                        L"orientation",
                                        L"dimension",
                                        L"topblockId",
                                        L"biomeId",
                                        L"feature",
                                        L"minCount",
                                        L"maxCount",
                                        L"weight",
                                        L"id",
                                        L"probability",
                                        L"method",
                                        L"hasBeenInCreative",
                                        L"cloudHeight",
                                        L"fogDistance",
                                        L"dayTime",
                                        L"target",
                                        L"speed",
                                        L"dir",
                                        L"type",
                                        L"pass",
                                        L"for",
                                        L"random",
                                        L"blockAux",
                                        L"size",
                                        L"scale",
                                        L"freq",
                                        L"func",
                                        L"upper",
                                        L"lower",
                                        L"dY",
                                        L"thickness",
                                        L"points",
                                        L"holeSize",
                                        L"variant",
                                        L"startHeight",
                                        L"pattern",
                                        L"colour",
                                        L"primary",
                                        L"laps",
                                        L"liftForceModifier",
                                        L"staticLift",
                                        L"targetHeight",
                                        L"speedBoost",
                                        L"boostDirection",
                                        L"condition_type",
                                        L"condition_value_0",
                                        L"condition_value_1",
                                        L"beam_length",
                                        L"beam_direction",
                                        L"string_IDS"};

IGameRuleAttribute::IGameRuleAttribute() : mOptional(false), mHasValue(false)
{
}

int IGameRuleAttribute::attributeIdFromName(const std::wstring& name)
{
    int remaining = 13;
    int id = 0;
    const wchar_t** names = gRuleAttributeNames;

    do
    {
        const wchar_t* candidate = names[0];
        size_t len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id;
        }

        candidate = names[1];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 1;
        }

        candidate = names[2];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 2;
        }

        candidate = names[3];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 3;
        }

        candidate = names[4];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 4;
        }

        candidate = names[5];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 5;
        }

        candidate = names[6];
        len = wcslen(candidate);
        if (name.compare(0, name.size(), candidate, len) == 0)
        {
            return id + 6;
        }

        remaining--;
        id += 7;
        names += 7;
    } while (remaining != 0);

    return -1;
}

void IGameRuleAttribute::setOptional(bool optional)
{
    mOptional = optional;
}

bool IGameRuleAttribute::isOptional()
{
    return mOptional;
}

bool IGameRuleAttribute::hasValue()
{
    return mHasValue;
}
