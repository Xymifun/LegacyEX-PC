#ifndef GAME_NETWORK_NET_MINECRAFT_WORLD__CONTAINERLISTENER_HPP
#define GAME_NETWORK_NET_MINECRAFT_WORLD__CONTAINERLISTENER_HPP

class Container;

namespace net_minecraft_world
{

class ContainerListener
{
public:
    virtual ~ContainerListener() {}

    virtual void containerChanged(Container* container) = 0;
};

}

#endif
