#ifndef GAME_CLIENT_RENDER_MODELRESOURCELOCATION_HPP
#define GAME_CLIENT_RENDER_MODELRESOURCELOCATION_HPP

#include <string>

#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ModelResourceLocation : public ResourceLocation
{
public:
    ModelResourceLocation(int dummy, arrayWithLength<std::wstring> parts);

    static ModelResourceLocation* Make(const std::wstring& location, const std::wstring& variant);
    static ModelResourceLocation* Make(const ResourceLocation& location, const std::wstring& variant);
    static ModelResourceLocation* Make(const std::wstring& nameSpace, const std::wstring& path,
                                       const std::wstring& variant);
    static ModelResourceLocation* Make(const std::wstring& location);

    std::wstring getVariant() const;

    virtual std::wstring toString() const;

    int hashCode();
    bool equals(ModelResourceLocation* other);

protected:
    static arrayWithLength<std::wstring> decompose(const std::wstring& name);

private:
    std::wstring variant;
};

#endif
