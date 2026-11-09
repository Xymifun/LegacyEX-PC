#include "game/client/render/ParticleCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand ParticleCommand::getId()
{
    return eGameCommand_Particle;
}

int ParticleCommand::getPermissionLevel()
{
    return 2;
}

void ParticleCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> ParticleCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
