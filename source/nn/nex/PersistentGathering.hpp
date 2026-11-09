#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PERSISTENTGATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PERSISTENTGATHERING_HPP

#include "nn/nex/PersistentGatheringType.hpp"
#include "nn/nex/_DDL_PersistentGathering.hpp"

namespace nn
{
namespace nex
{

class PersistentGathering : public _DDL_PersistentGathering
{
public:
    PersistentGathering();
    virtual ~PersistentGathering() {}

    void Reset();
    void SetPersistentGatheringType(PersistentGatheringType::PersistentGatheringType type);
    unsigned int GetPersistentGatheringType() const;
    void SetAttribute(unsigned int uiIndex, unsigned int uiValue);
    unsigned int GetAttribute(unsigned int uiIndex) const;
};

}
}

#endif
