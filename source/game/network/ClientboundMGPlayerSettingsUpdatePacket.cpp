#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

maybe<float> readMaybeFloat(DataInputStream* input)
{
    if (input->readBoolean())
        return maybe<float>(input->readFloat());
    return maybe<float>();
}

ClientboundMGPlayerSettingsUpdatePacket::ClientboundMGPlayerSettingsUpdatePacket() : mSettings()
{
}
ClientboundMGPlayerSettingsUpdatePacket::ClientboundMGPlayerSettingsUpdatePacket(const MiniGamePlayerSettings& settings)
{
    mSettings = settings;
}
ClientboundMGPlayerSettingsUpdatePacket::~ClientboundMGPlayerSettingsUpdatePacket()
{
}
void ClientboundMGPlayerSettingsUpdatePacket::read(DataInputStream* input)
{
    mSettings.mHostOptions.Set(eCustomGameModeHostOption_All, input->readVarInt());
    mSettings.mPlayerAbilities.read(input);
    mSettings.mLivesPerRound = input->readVarInt();
    mSettings.mLootSetId = static_cast<unsigned int>(input->readInt());
    mSettings.mRandomLootSet = input->readBoolean();
    mSettings.mTeamCount = input->readVarInt();
    mSettings.mTeamMaxSize = input->readVarInt();
    mSettings.mTeamBasedSpawn = input->readBoolean();
    mSettings.mCheckpointBasedSpawn = input->readBoolean();
    mSettings.mShowNametags = input->readBoolean();
    mSettings.mTakeEverythingEnabled = input->readBoolean();
    mSettings.mChestRefillEnabled = input->readBoolean();

    unsigned char tntFlags = input->readByte();
    if (tntFlags & 1)
        mSettings.mCanTntDestroyBlocks = maybe<bool>(tntFlags >> 1);
    else
        mSettings.mCanTntDestroyBlocks = maybe<bool>();

    if (input->readBoolean())
        mSettings.mMapSizeOverride = maybe<int>(input->readVarInt());
    else
        mSettings.mMapSizeOverride = maybe<int>();
    mSettings.mRandomMapSize = input->readBoolean();
    mSettings.mArenaSizeMultiplier = readMaybeFloat(input);

    bool hasLayers = input->readBoolean();
    mSettings.mLayerCount = hasLayers ? maybe<unsigned int>(input->readVarInt()) : maybe<unsigned int>();
    mSettings.mRoundCount = input->readInt();
    mSettings.mSmallInventoryEnabled = input->readBoolean();
    mSettings.mNaturalRegeneration = input->readBoolean();
    mSettings.mFoodBalance = input->readInt();
    mSettings.mSmallSkinsEnabled = input->readBoolean();
    mSettings.mRoundLength = input->readInt();
    mSettings.mMaxPlayers = input->readInt();
    mSettings.mCentralSpawnDisabled = input->readBoolean();
    mSettings.mTeamGame = input->readBoolean();
    mSettings.mSpectatorParticipation = input->readBoolean();
}
void ClientboundMGPlayerSettingsUpdatePacket::write(DataOutputStream* output)
{
    output->writeVarInt(mSettings.mHostOptions.Get(eCustomGameModeHostOption_All));
    mSettings.mPlayerAbilities.write(output);
    output->writeVarInt(mSettings.mLivesPerRound);
    output->writeInt(static_cast<int>(mSettings.mLootSetId));
    output->writeBoolean(mSettings.mRandomLootSet);
    output->writeVarInt(mSettings.mTeamCount);
    output->writeVarInt(mSettings.mTeamMaxSize);
    output->writeBoolean(mSettings.mTeamBasedSpawn);
    output->writeBoolean(mSettings.mCheckpointBasedSpawn);
    output->writeBoolean(mSettings.mShowNametags);
    output->writeBoolean(mSettings.mTakeEverythingEnabled);
    output->writeBoolean(mSettings.mChestRefillEnabled);

    int destroyFlag = (mSettings.mCanTntDestroyBlocks.isSet() && mSettings.mCanTntDestroyBlocks.get()) ? 2 : 0;
    output->writeByte(mSettings.mCanTntDestroyBlocks.isSet() | destroyFlag);

    output->writeBoolean(mSettings.mMapSizeOverride.isSet());
    if (mSettings.mMapSizeOverride.isSet())
        output->writeVarInt(mSettings.mMapSizeOverride.get());
    output->writeBoolean(mSettings.mRandomMapSize);

    output->writeBoolean(mSettings.mArenaSizeMultiplier.isSet());
    if (mSettings.mArenaSizeMultiplier.isSet())
        output->writeFloat(mSettings.mArenaSizeMultiplier.get());

    const bool hasLayers = mSettings.mLayerCount.isSet();
    const unsigned int layerCount = hasLayers ? mSettings.mLayerCount.get() : 0;
    output->writeBoolean(hasLayers);
    if (hasLayers)
        output->writeVarInt(layerCount);

    output->writeInt(mSettings.mRoundCount);
    output->writeBoolean(mSettings.mSmallInventoryEnabled);
    output->writeBoolean(mSettings.mNaturalRegeneration);
    output->writeInt(mSettings.mFoodBalance);
    output->writeBoolean(mSettings.mSmallSkinsEnabled);
    output->writeInt(mSettings.mRoundLength);
    output->writeInt(mSettings.mMaxPlayers);
    output->writeBoolean(mSettings.mCentralSpawnDisabled);
    output->writeBoolean(mSettings.mTeamGame);
    output->writeBoolean(mSettings.mSpectatorParticipation);
}
void ClientboundMGPlayerSettingsUpdatePacket::handle(PacketListener* listener)
{
    listener->handleMiniGamePlayerSettingsUpdatePacket(shared_from_this());
}
int ClientboundMGPlayerSettingsUpdatePacket::getEstimatedSize()
{
    return 0x68;
}
const MiniGamePlayerSettings& ClientboundMGPlayerSettingsUpdatePacket::getSettings() const
{
    return mSettings;
}
int ClientboundMGPlayerSettingsUpdatePacket::getPacketId()
{
    return 0xef;
}
boost::shared_ptr<Packet> ClientboundMGPlayerSettingsUpdatePacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundMGPlayerSettingsUpdatePacket());
}
