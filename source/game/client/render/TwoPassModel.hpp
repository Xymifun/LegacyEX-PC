#pragma once

#include <boost/shared_ptr.hpp>

class Entity;

class TwoPassModel
{
public:
    virtual void renderSecondPass(boost::shared_ptr<Entity> entity, float limbSwing, float limbSwingAmount,
                                  float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                  bool useDisplayList) = 0;
};
