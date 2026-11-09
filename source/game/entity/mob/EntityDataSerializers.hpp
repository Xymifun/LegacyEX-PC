#ifndef GAME_ENTITY_MOB_ENTITYDATASERIALIZERS_HPP
#define GAME_ENTITY_MOB_ENTITYDATASERIALIZERS_HPP

#include <string>

#include "game/core/Rotations.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EntityDataAccessor.hpp"
#include "game/world/block/EntityDataSerializer.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/Direction.hpp"

class DataInputStream;
class DataOutputStream;

class EntityDataSerializers
{
public:
    class BLOCK_POS_EntityDataSerializer : public EntityDataSerializer<BlockPos>
    {
    public:
        BLOCK_POS_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, BlockPos value) const;
        virtual BlockPos read(DataInputStream* input) const;
        virtual EntityDataAccessor<BlockPos>* createAccessor(int id);
        virtual BlockPos copy(BlockPos value) const;
    };

    class BLOCK_STATE_EntityDataSerializer : public EntityDataSerializer<const BlockState*>
    {
    public:
        BLOCK_STATE_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, const BlockState* value) const;
        virtual const BlockState* read(DataInputStream* input) const;
        virtual EntityDataAccessor<const BlockState*>* createAccessor(int id);
        virtual const BlockState* copy(const BlockState* value) const;
    };

    class BOOLEAN_EntityDataSerializer : public EntityDataSerializer<bool>
    {
    public:
        BOOLEAN_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, bool value) const;
        virtual bool read(DataInputStream* input) const;
        virtual EntityDataAccessor<bool>* createAccessor(int id);
        virtual bool copy(bool value) const;
    };

    class BYTE_EntityDataSerializer : public EntityDataSerializer<unsigned char>
    {
    public:
        BYTE_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, unsigned char value) const;
        virtual unsigned char read(DataInputStream* input) const;
        virtual EntityDataAccessor<unsigned char>* createAccessor(int id);
        virtual unsigned char copy(unsigned char value) const;
    };

    class COMPOUND_TAG_EntityDataSerializer : public EntityDataSerializer<CompoundTag*>
    {
    public:
        COMPOUND_TAG_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, CompoundTag* value) const;
        virtual CompoundTag* read(DataInputStream* input) const;
        virtual EntityDataAccessor<CompoundTag*>* createAccessor(int id);
        virtual CompoundTag* copy(CompoundTag* value) const;
    };

    class DIRECTION_EntityDataSerializer : public EntityDataSerializer<const Direction*>
    {
    public:
        DIRECTION_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, const Direction* value) const;
        virtual const Direction* read(DataInputStream* input) const;
        virtual EntityDataAccessor<const Direction*>* createAccessor(int id);
        virtual const Direction* copy(const Direction* value) const;
    };

    class FLOAT_EntityDataSerializer : public EntityDataSerializer<float>
    {
    public:
        FLOAT_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, float value) const;
        virtual float read(DataInputStream* input) const;
        virtual EntityDataAccessor<float>* createAccessor(int id);
        virtual float copy(float value) const;
    };

    class INT_EntityDataSerializer : public EntityDataSerializer<int>
    {
    public:
        INT_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, int value) const;
        virtual int read(DataInputStream* input) const;
        virtual EntityDataAccessor<int>* createAccessor(int id);
        virtual int copy(int value) const;
    };

    class ITEM_INSTANCE_EntityDataSerializer : public EntityDataSerializer<not_null_ptr<ItemInstance> >
    {
    public:
        ITEM_INSTANCE_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, not_null_ptr<ItemInstance> value) const;
        virtual not_null_ptr<ItemInstance> read(DataInputStream* input) const;
        virtual EntityDataAccessor<not_null_ptr<ItemInstance> >* createAccessor(int id);
        virtual not_null_ptr<ItemInstance> copy(not_null_ptr<ItemInstance> value) const;
    };

    class OPTIONAL_BLOCK_POS_EntityDataSerializer : public EntityDataSerializer<BlockPos*>
    {
    public:
        OPTIONAL_BLOCK_POS_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, BlockPos* value) const;
        virtual BlockPos* read(DataInputStream* input) const;
        virtual EntityDataAccessor<BlockPos*>* createAccessor(int id);
        virtual BlockPos* copy(BlockPos* value) const;
    };

    class ROTATIONS_EntitySerializer : public EntityDataSerializer<const Rotations*>
    {
    public:
        ROTATIONS_EntitySerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, const Rotations* value) const;
        virtual const Rotations* read(DataInputStream* input) const;
        virtual EntityDataAccessor<const Rotations*>* createAccessor(int id);
        virtual const Rotations* copy(const Rotations* value) const;
    };

    class STRING_EntityDataSerializer : public EntityDataSerializer<std::wstring>
    {
    public:
        STRING_EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id);
        virtual void write(DataOutputStream* output, std::wstring value) const;
        virtual std::wstring read(DataInputStream* input) const;
        virtual EntityDataAccessor<std::wstring>* createAccessor(int id);
        virtual std::wstring copy(std::wstring value) const;
    };

    static EntityDataSerializer<unsigned char>* BYTE;
    static EntityDataSerializer<int>* INT;
    static EntityDataSerializer<float>* FLOAT;
    static EntityDataSerializer<std::wstring>* STRING;
    static EntityDataSerializer<not_null_ptr<ItemInstance> >* ITEM_INSTANCE;
    static EntityDataSerializer<const BlockState*>* BLOCK_STATE;
    static EntityDataSerializer<bool>* BOOLEAN;
    static EntityDataSerializer<const Rotations*>* ROTATIONS;
    static EntityDataSerializer<BlockPos>* BLOCK_POS;
    static EntityDataSerializer<BlockPos*>* OPTIONAL_BLOCK_POS;
    static EntityDataSerializer<const Direction*>* DIRECTION;
    static EntityDataSerializer<CompoundTag*>* COMPOUND_TAG;

    EntityDataSerializers();
};

#endif
