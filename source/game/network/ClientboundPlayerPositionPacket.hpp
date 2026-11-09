#ifndef GAME_NETWORK_CLIENTBOUNDPLAYERPOSITIONPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPLAYERPOSITIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundPlayerPositionPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundPlayerPositionPacket>
{
public:
    class RelativeArgument
    {
    public:
        RelativeArgument(int bit);

        int getMask() const;
        bool isSet(int flags) const;

        static boost::unordered_set<const RelativeArgument*> unpack(int flags);
        static int pack(const boost::unordered_set<const RelativeArgument*>& relativeArguments);

        static const RelativeArgument X;
        static const RelativeArgument Y;
        static const RelativeArgument Z;
        static const RelativeArgument Y_ROT;
        static const RelativeArgument X_ROT;

    private:
        int bit;
    };

    ClientboundPlayerPositionPacket();
    ClientboundPlayerPositionPacket(double x, double y, double z, float yRot, float xRot,
                                    const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id);
    ClientboundPlayerPositionPacket(double x, double y, double z,
                                    const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id);
    virtual ~ClientboundPlayerPositionPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    double getX();
    double getY();
    double getZ();
    float getYRot();
    float getXRot();
    int getId();
    bool hasRot();
    const boost::unordered_set<const RelativeArgument*>& getRelativeArguments();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundPlayerPositionPacket());
    }

private:
    double x;
    double y;
    double z;
    float yRot;
    float xRot;
    bool hasRotation;
    boost::unordered_set<const RelativeArgument*> relativeArguments;
    int id;
};

#endif
