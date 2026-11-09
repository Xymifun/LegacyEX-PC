#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_EVENT_LISTENER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_EVENT_LISTENER_HPP

#include "nn/pia.h"

namespace nn
{
namespace pia
{
namespace session
{

class MeshEventListener
{
public:
    struct MeshEvent
    {
        unsigned int type;
        StationIndex stationIndex;
        unsigned long long stationId;
    };

    MeshEventListener();
    ~MeshEventListener();
    Result RemoveStationId(unsigned long long stationId);
    void RemoveStationIndex(StationIndex stationIndex);
    void NoticeEvent(const MeshEvent& event);
    void Trace(unsigned long long value) const;
};

}
}
}

#endif
