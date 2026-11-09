#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include <boost/unordered_map.hpp>

#include "game/core/DataInputStream.hpp"
#include "game/core/Pig.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

static boost::unordered_map<unsigned long long, std::vector<PositionOverrideStruct> > gRiderPositionOverrides;

void RiderPositionOverrides::ClearOverrides()
{
    gRiderPositionOverrides.clear();
}

void RiderPositionOverrides::ReadOverrides(DataInputStream* stream)
{
    std::wstring entityName = stream->readUTF();
    int count = stream->readInt();

    for (int i = 0; i < count; ++i)
    {
        bool isTamed = stream->readBoolean();
        bool isSaddled = stream->readBoolean();
        double x = stream->readFloat();
        double y = stream->readFloat();
        double z = stream->readFloat();
        unsigned long long type = EntityIO::byString(entityName);
        PositionOverrideStruct overridePosition = {isTamed, isSaddled, x, y, z};
        gRiderPositionOverrides[type].push_back(overridePosition);
    }
}

bool RiderPositionOverrides::OverridePosition(const boost::shared_ptr<Entity>& vehicle,
                                              const boost::shared_ptr<Entity>& passenger)
{
    if (gRiderPositionOverrides.find(vehicle->GetType()) == gRiderPositionOverrides.end())
    {
        return false;
    }

    unsigned char isTamed = 0;
    unsigned char isSaddled = 0;
    if (eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL, vehicle->GetType()))
    {
        isTamed = boost::static_pointer_cast<TamableAnimal>(vehicle)->isTame();
    }
    else if (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, vehicle->GetType()))
    {
        boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(vehicle);
        isSaddled = horse->isSaddled();
        isTamed = horse->isTamed();
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PIG, vehicle->GetType()))
    {
        isSaddled = boost::static_pointer_cast<Pig>(vehicle)->hasSaddle();
    }

    for (unsigned int i = 0; i < gRiderPositionOverrides.at(vehicle->GetType()).size(); ++i)
    {
        PositionOverrideStruct overridePosition = gRiderPositionOverrides.at(vehicle->GetType()).at(i);
        if (overridePosition.isTamed != isTamed || overridePosition.isSaddled != isSaddled)
        {
            continue;
        }

        Vec3* riderOffset = Vec3::newTemp(overridePosition.x, overridePosition.y + passenger->getRidingHeight(),
                                          overridePosition.z);
        riderOffset->yRot(-(vehicle->mYRot * Mth::DEG_TO_RAD));

        Vec3* vehiclePosition = Vec3::newTemp(vehicle->mX, vehicle->mBoundingBox->mMinY, vehicle->mZ);

        float cosine;
        float sine;
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, vehicle->GetType()))
        {
            float yBodyRot = boost::static_pointer_cast<LivingEntity>(vehicle)->mYBodyRot;
            float bodyRotation = yBodyRot * Mth::DEG_TO_RAD;
            cosine = Mth::cos(bodyRotation);
            sine = Mth::sin(bodyRotation);
        }
        else
        {
            cosine = 1.0f;
            sine = cosine;
        }

        passenger->setPos(riderOffset->x * sine + vehiclePosition->x, vehiclePosition->y + riderOffset->y,
                          riderOffset->z * cosine + vehiclePosition->z);
        return true;
    }

    return false;
}
