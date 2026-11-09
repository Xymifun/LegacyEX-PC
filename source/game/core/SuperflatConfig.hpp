#ifndef GAME_CORE_SUPERFLATCONFIG_HPP
#define GAME_CORE_SUPERFLATCONFIG_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/DefaultPresets.hpp"

class ByteArrayTag;

class SuperflatConfig
{
public:
    SuperflatConfig();
    ~SuperflatConfig() {}
    bool hasParam(unsigned char parameter);
    SuperflatConfig* setDisplayname(const std::wstring& name);
    bool hasOption(unsigned char parameter, unsigned char option);
    void addParam(unsigned char parameter);
    void setOption(unsigned char parameter, unsigned char option, unsigned char value);
    void removeParam(unsigned char parameter);
    void updateLayers();
    void clear();
    std::wstring getDisplayname();
    void setVersion(unsigned char version);
    unsigned char getOption(unsigned char parameter, unsigned char option);
    unsigned int maybeOption(unsigned char parameter, unsigned char option, unsigned char fallback);
    arrayWithLength<unsigned char> toBlob();
    void fromBlob(arrayWithLength<unsigned char> data);
    ByteArrayTag* toTag();
    void fromTag(ByteArrayTag* tag);
    SuperflatConfig* makeDefaults();
    SuperflatConfig* makePreset(DefaultPresets::Preset preset);
    unsigned char getVersion();
    SuperflatConfig* makeDebug();

    std::wstring mDisplayname;
    unsigned char mVersion;
    std::vector<FlatLayerInfo> layers;
    boost::unordered_map<unsigned char, boost::unordered_map<unsigned char, unsigned char> > mOptions;
};

#endif
