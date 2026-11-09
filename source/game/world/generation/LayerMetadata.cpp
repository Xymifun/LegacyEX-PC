#include "game/world/generation/LayerMetadata.hpp"

static const std::wstring FILTER_NAMES[LayerMetadata::eFilter_Count] = {
    L"AnyLayer",       L"OnlyBottomLayer", L"OnlyMiddleLayers", L"OnlyTopLayer",
    L"NotBottomLayer", L"NotMiddleLayers", L"NotTopLayer"};

static const LayerMetadata::FilterFunc FILTER_FUNCS[LayerMetadata::eFilter_Count] = {
    &LayerMetadata::AnyLayer,     &LayerMetadata::OnlyBottomLayer, &LayerMetadata::OnlyMiddleLayers,
    &LayerMetadata::OnlyTopLayer, &LayerMetadata::NotBottomLayer,  &LayerMetadata::NotMiddleLayers,
    &LayerMetadata::NotTopLayer};

LayerMetadata::LayerMetadata() : m_iLayerIndex(0), m_iBottomLayerIndex(0)
{
}

bool LayerMetadata::AnyLayer(const LayerMetadata& metadata)
{
    return true;
}

bool LayerMetadata::OnlyBottomLayer(const LayerMetadata& metadata)
{
    return metadata.m_iLayerIndex == metadata.m_iBottomLayerIndex;
}

bool LayerMetadata::OnlyTopLayer(const LayerMetadata& metadata)
{
    return metadata.m_iLayerIndex == 0;
}

bool LayerMetadata::OnlyMiddleLayers(const LayerMetadata& metadata)
{
    return !OnlyBottomLayer(metadata) && !OnlyTopLayer(metadata);
}

bool LayerMetadata::NotBottomLayer(const LayerMetadata& metadata)
{
    return !OnlyBottomLayer(metadata);
}

bool LayerMetadata::NotMiddleLayers(const LayerMetadata& metadata)
{
    return !OnlyMiddleLayers(metadata);
}

bool LayerMetadata::NotTopLayer(const LayerMetadata& metadata)
{
    return !OnlyTopLayer(metadata);
}

LayerMetadata::FilterFunc LayerMetadata::getFilter(eFilter eFilterType)
{
    return FILTER_FUNCS[eFilterType];
}

LayerMetadata::FilterFunc LayerMetadata::getFilter(const std::wstring& sFilterName)
{
    return getFilter(fromStringFilter(sFilterName));
}

LayerMetadata::eFilter LayerMetadata::fromStringFilter(const std::wstring& sFilterName)
{
    for (int i = 0; i < eFilter_Count; ++i)
    {
        if (sFilterName.compare(FILTER_NAMES[i]) == 0)
        {
            return (eFilter)i;
        }
    }

    return eFilter_AnyLayer;
}

std::wstring LayerMetadata::toStringFilter(eFilter eFilterType)
{
    return FILTER_NAMES[eFilterType];
}
