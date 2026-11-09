#include "game/core/SuperflatConfig.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"

SuperflatConfig::SuperflatConfig() : mDisplayname(), mVersion(0), layers(), mOptions()
{
    clear();
}

bool SuperflatConfig::hasParam(unsigned char parameter)
{
    unsigned char key = parameter;
    return mOptions.find(key) != mOptions.end();
}

SuperflatConfig* SuperflatConfig::setDisplayname(const std::wstring& name)
{
    mDisplayname = name;
    return this;
}

bool SuperflatConfig::hasOption(unsigned char parameter, unsigned char option)
{
    return hasParam(parameter) && mOptions[parameter].find(option) != mOptions[parameter].end();
}

void SuperflatConfig::addParam(unsigned char parameter)
{
    mOptions.insert(std::make_pair(parameter, boost::unordered_map<unsigned char, unsigned char>()));
}

void SuperflatConfig::setOption(unsigned char parameter, unsigned char option, unsigned char value)
{
    if (hasOption(parameter, option))
    {
        unsigned char key = parameter;
        unsigned char subkey = option;
        mOptions[key][subkey] = value;
    }
    else
    {
        if (!hasParam(parameter))
        {
            addParam(parameter);
        }
        unsigned char key = parameter;
        boost::unordered_map<unsigned char, unsigned char>& options = mOptions[key];
        options.insert(boost::unordered_map<unsigned char, unsigned char>::value_type(option, value));
    }
}

void SuperflatConfig::removeParam(unsigned char parameter)
{
    boost::unordered_map<unsigned char, boost::unordered_map<unsigned char, unsigned char> >::iterator it =
        mOptions.find(parameter);
    if (it != mOptions.end())
    {
        mOptions.erase(it);
    }
}

void SuperflatConfig::updateLayers()
{
    unsigned int start = 0;
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(); it != layers.end(); ++it)
    {
        it->setStart(start);
        start += it->getHeight();
    }
}

void SuperflatConfig::clear()
{
    mVersion = 255;
    layers.clear();
    mOptions.clear();
}

std::wstring SuperflatConfig::getDisplayname()
{
    return std::wstring(mDisplayname);
}

void SuperflatConfig::setVersion(unsigned char version)
{
    mVersion = version;
}

unsigned char SuperflatConfig::getOption(unsigned char parameter, unsigned char option)
{
    if (hasOption(parameter, option))
    {
        unsigned char key = parameter;
        unsigned char subkey = option;
        return mOptions[key][subkey];
    }
    return 0;
}

unsigned int SuperflatConfig::maybeOption(unsigned char parameter, unsigned char option, unsigned char fallback)
{
    if (hasOption(parameter, option))
    {
        return getOption(parameter, option);
    }
    return fallback;
}

arrayWithLength<unsigned char> SuperflatConfig::toBlob()
{
    ByteArrayOutputStream output;
    DataOutputStream stream(&output);
    stream.writeByte(mVersion);
    stream.writeUTF(mDisplayname);
    stream.writeByte((unsigned char)layers.size());
    for (std::vector<FlatLayerInfo>::iterator layer = layers.begin(); layer != layers.end(); ++layer)
    {
        stream.writeUnsignedShort((unsigned short)layer->getId());
        stream.writeByte((unsigned char)layer->getData());
        stream.writeByte((unsigned char)layer->getHeight());
    }
    stream.writeByte((unsigned char)mOptions.size());
    for (boost::unordered_map<unsigned char, boost::unordered_map<unsigned char, unsigned char> >::iterator parameter =
             mOptions.begin();
         parameter != mOptions.end(); ++parameter)
    {
        stream.writeByte(parameter->first);
        stream.writeByte((unsigned char)parameter->second.size());
        for (boost::unordered_map<unsigned char, unsigned char>::iterator option = parameter->second.begin();
             option != parameter->second.end(); ++option)
        {
            stream.writeByte(option->first);
            stream.writeByte(option->second);
        }
    }
    return output.toByteArray();
}

void SuperflatConfig::fromBlob(arrayWithLength<unsigned char> data)
{
    clear();
    ByteArrayInputStream input(data);
    DataInputStream stream(&input);
    mVersion = stream.readByte();
    mDisplayname = stream.readUTF();
    unsigned char layerCount = stream.readByte();
    for (unsigned int i = 0; i < layerCount; ++i)
    {
        int blockId;
        int dataValue;
        int height;
        if (mVersion == 5)
        {
            blockId = stream.readUnsignedShort();
            dataValue = stream.readByte();
            height = stream.readByte();
        }
        else
        {
            blockId = stream.readByte();
            dataValue = stream.readByte();
            height = stream.readByte();
        }
        layers.push_back(FlatLayerInfo(height, blockId, dataValue));
    }
    updateLayers();
    unsigned char parameterCount = stream.readByte();
    for (unsigned int i = 0; i < parameterCount; ++i)
    {
        unsigned char parameter = stream.readByte();
        addParam(parameter);
        unsigned char optionCount = stream.readByte();
        for (unsigned int j = 0; j < optionCount; ++j)
        {
            setOption(parameter, stream.readByte(), stream.readByte());
        }
    }
    input.reset();
}

ByteArrayTag* SuperflatConfig::toTag()
{
    return new ByteArrayTag(toBlob(), true);
}

void SuperflatConfig::fromTag(ByteArrayTag* tag)
{
    fromBlob(tag->getAsByteArray());
}

SuperflatConfig* SuperflatConfig::makeDefaults()
{
    clear();
    FlatGeneratorInfo info(this, false);
    info.addLayer(FlatLayerInfo(1, Blocks::grass));
    info.addLayer(FlatLayerInfo(2, Blocks::dirt));
    info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    info.setBiome();
    info.setVillage(true);
    return this;
}

SuperflatConfig* SuperflatConfig::makePreset(DefaultPresets::Preset preset)
{
    clear();
    DefaultPresets::makePreset(this, preset);
    return this;
}
