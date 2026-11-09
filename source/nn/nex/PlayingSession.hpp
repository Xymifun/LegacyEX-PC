#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PLAYINGSESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PLAYINGSESSION_HPP

#include "nn/nex/_DDL_PlayingSession.hpp"

namespace nn
{
namespace nex
{

class MatchmakeSession;

class PlayingSession : public _DDL_PlayingSession
{
public:
    virtual ~PlayingSession() {}

    MatchmakeSession* GetMatchmakeSession() const;
};

}
}

#endif
