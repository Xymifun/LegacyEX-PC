#include "nn/pia/lan/LanJoinSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

LanJoinSessionSetting::LanJoinSessionSetting() : session::JoinSessionSetting()
{
}
LanJoinSessionSetting::~LanJoinSessionSetting()
{
}
void LanJoinSessionSetting::Reset()
{
    session::JoinSessionSetting::Reset();
}
void LanJoinSessionSetting::SetSessionId(unsigned int)
{
}

}
}
}
