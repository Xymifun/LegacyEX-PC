#ifndef GAME_WORLD_DIMENSIONTYPE_HPP
#define GAME_WORLD_DIMENSIONTYPE_HPP

#include <string>

class Dimension;

class DimensionType
{
public:
    static const DimensionType* OVERWORLD;
    static const DimensionType* NETHER;
    static const DimensionType* THE_END;

    DimensionType(int id, const std::wstring& name, const std::wstring& fileSuffix);

    int getId() const;
    std::wstring getName() const;
    std::wstring getFileSuffix() const;

    Dimension* create() const;

    static const DimensionType* getById(int id);
    static const DimensionType* getByName(const std::wstring& name);

private:
    int id;
    std::wstring name;
    std::wstring fileSuffix;
};

#endif
