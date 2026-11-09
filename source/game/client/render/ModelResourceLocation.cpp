#include "game/client/render/ModelResourceLocation.hpp"
#include "game/core/StringUtils.hpp"
#include "game/world/block/AbstractProperty.hpp"

static const std::wstring DEFAULT_VARIANT = L"normal";

ModelResourceLocation::ModelResourceLocation(int dummy, arrayWithLength<std::wstring> parts)
    : ResourceLocation(parts.data[0], parts.data[1]),
      variant(parts.data[2].length() == 0 ? DEFAULT_VARIANT : toLower(parts.data[2]))
{
}

arrayWithLength<std::wstring> ModelResourceLocation::decompose(const std::wstring& name)
{
    arrayWithLength<std::wstring> parts(3, true);

    parts.data[0] = L"";
    parts.data[1] = name;
    parts.data[2] = L"";

    unsigned int index = name.find(L'#');

    std::wstring location = name;

    if (index < name.length())
    {
        parts.data[2] = name.substr(index + 1, name.length());

        if (index > 1)
        {
            location = name.substr(0, index);
        }
    }

    arrayWithLength<std::wstring> baseParts = ResourceLocation::decompose(location);

    parts.data[0] = baseParts.data[0];
    parts.data[1] = baseParts.data[1];

    delete[] baseParts.data;

    return parts;
}

ModelResourceLocation* ModelResourceLocation::Make(const std::wstring& location, const std::wstring& variant)
{
    std::wstring text = location + L'#';

    const std::wstring& modelVariant = variant.length() == 0 ? DEFAULT_VARIANT : variant;

    std::wstring name = text + modelVariant;

    return new ModelResourceLocation(0, decompose(name));
}

ModelResourceLocation* ModelResourceLocation::Make(const ResourceLocation& location, const std::wstring& variant)
{
    return Make(location.toString(), variant);
}

ModelResourceLocation* ModelResourceLocation::Make(const std::wstring& nameSpace, const std::wstring& path,
                                                   const std::wstring& variant)
{
    return Make(ResourceLocation(nameSpace, path), variant);
}

ModelResourceLocation* ModelResourceLocation::Make(const std::wstring& location)
{
    return Make(location, DEFAULT_VARIANT);
}

std::wstring ModelResourceLocation::toString() const
{
    return ResourceLocation::toString() + L'#' + variant;
}

int ModelResourceLocation::hashCode()
{
    return ResourceLocation::hashCode() * 31 + javaStringHashCode(variant);
}

bool ModelResourceLocation::equals(ModelResourceLocation* other)
{
    if (this == other)
    {
        return true;
    }
    return ResourceLocation::equals(other) && variant == other->variant;
}
