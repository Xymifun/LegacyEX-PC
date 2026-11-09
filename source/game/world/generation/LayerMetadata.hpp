#ifndef GAME_WORLD_GENERATION_LAYERMETADATA_HPP
#define GAME_WORLD_GENERATION_LAYERMETADATA_HPP

#include <string>

class LayerMetadata
{
public:
    enum eFilter
    {
        eFilter_AnyLayer,
        eFilter_OnlyBottomLayer,
        eFilter_OnlyMiddleLayers,
        eFilter_OnlyTopLayer,
        eFilter_NotBottomLayer,
        eFilter_NotMiddleLayers,
        eFilter_NotTopLayer,
        eFilter_Count
    };

    typedef bool (*FilterFunc)(const LayerMetadata& metadata);

    LayerMetadata();

    static bool AnyLayer(const LayerMetadata& metadata);
    static bool OnlyBottomLayer(const LayerMetadata& metadata);
    static bool OnlyTopLayer(const LayerMetadata& metadata);
    static bool OnlyMiddleLayers(const LayerMetadata& metadata);
    static bool NotBottomLayer(const LayerMetadata& metadata);
    static bool NotMiddleLayers(const LayerMetadata& metadata);
    static bool NotTopLayer(const LayerMetadata& metadata);

    static FilterFunc getFilter(eFilter eFilterType);
    static FilterFunc getFilter(const std::wstring& sFilterName);
    static eFilter fromStringFilter(const std::wstring& sFilterName);
    static std::wstring toStringFilter(eFilter eFilterType);

    unsigned int m_uGameVariant;
    int m_iLayerIndex;
    int m_iBottomLayerIndex;
};

#endif
