#include "game/world/DimensionType.hpp"

#include "game/world/Dimension.hpp"
#include "game/world/NetherDimension.hpp"
#include "game/world/NormalDimension.hpp"
#include "game/world/TheEndDimension.hpp"

const DimensionType* DimensionType::OVERWORLD = new DimensionType(0, L"Overworld", L"");
const DimensionType* DimensionType::NETHER = new DimensionType(-1, L"Nether", L"_nether");
const DimensionType* DimensionType::THE_END = new DimensionType(1, L"The End", L"_end");

DimensionType::DimensionType(int id, const std::wstring& name, const std::wstring& fileSuffix)
{
    this->id = id;
    this->name = name;
    this->fileSuffix = fileSuffix;
}

int DimensionType::getId() const
{
    return id;
}

std::wstring DimensionType::getName() const
{
    return name;
}

std::wstring DimensionType::getFileSuffix() const
{
    return fileSuffix;
}

Dimension* DimensionType::create() const
{
    if (id == 1)
    {
        return new TheEndDimension();
    }

    if (id == -1)
    {
        return new NetherDimension();
    }

    return new NormalDimension();
}

const DimensionType* DimensionType::getById(int id)
{
    if (id == 1)
    {
        return THE_END;
    }

    if (id == -1)
    {
        return NETHER;
    }

    return OVERWORLD;
}

const DimensionType* DimensionType::getByName(const std::wstring& name)
{
    if (name.compare(OVERWORLD->getName()) != 0)
    {
        if (name.compare(NETHER->getName()) == 0)
        {
            return NETHER;
        }

        if (name.compare(THE_END->getName()) == 0)
        {
            return THE_END;
        }
    }

    return OVERWORLD;
}
