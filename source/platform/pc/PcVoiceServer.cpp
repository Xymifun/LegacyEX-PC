#include "platform/pc/PcVoice.hpp"

#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

#include <vector>

void PcVoiceServerTick()
{
    std::vector<PcVoicePosition> positions;
    PlayerList* all = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
    if (all != 0)
    {
        const std::vector<boost::shared_ptr<ServerPlayer> >& list = *all->getPlayers();
        for (size_t i = 0; i < list.size(); ++i)
        {
            ServerPlayer* p = list[i].get();
            INetworkPlayer* net = p->mConnection ? p->mConnection->getNetworkPlayer() : 0;
            if (net == 0)
                continue;
            PcVoicePosition pos;
            pos.smallId = net->GetSmallId();
            pos.dimension = p->mDimension;
            pos.x = p->mX;
            pos.y = p->mY;
            pos.z = p->mZ;
            pos.yaw = p->mYRot;
            positions.push_back(pos);
        }
    }
    PcVoiceSetPositions(positions);
}
