#ifndef GAME_CORE_ASSETTHEME_HPP
#define GAME_CORE_ASSETTHEME_HPP

#include <algorithm>
#include <utility>
#include <vector>

template <class T>
class AssetTheme
{
public:
    void addAsset(unsigned int minimumWidth, T asset);

    T getAssetForSize(unsigned int size) const;

private:
    static bool sortByMinimumWidth(const std::pair<unsigned int, T>& left, const std::pair<unsigned int, T>& right);

    std::vector<std::pair<unsigned int, T> > mAssets;
};

template <class T>
void AssetTheme<T>::addAsset(unsigned int minimumWidth, T asset)
{
    mAssets.push_back(std::pair<unsigned int, T>(minimumWidth, asset));
    std::sort(mAssets.begin(), mAssets.end(), sortByMinimumWidth);
}

template <class T>
T AssetTheme<T>::getAssetForSize(unsigned int size) const
{
    typename std::vector<std::pair<unsigned int, T> >::const_iterator asset;

    for (asset = mAssets.begin(); asset != mAssets.end(); ++asset)
    {
        if (asset->first <= size)
        {
            return asset->second;
        }
    }

    return mAssets.back().second;
}

template <class T>
bool AssetTheme<T>::sortByMinimumWidth(const std::pair<unsigned int, T>& left, const std::pair<unsigned int, T>& right)
{
    return left.first >= right.first;
}

#endif
