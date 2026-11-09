#pragma once

class LootTableContext;
class Random;

class LootItemCondition
{
public:
    virtual bool applies(Random* random, LootTableContext& context) = 0;
};
