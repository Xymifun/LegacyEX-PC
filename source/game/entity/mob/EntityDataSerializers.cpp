#include "game/entity/mob/EntityDataSerializers.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/block/Block.hpp"

EntityDataSerializer<unsigned char>* EntityDataSerializers::BYTE = new EntityDataSerializers::BYTE_EntityDataSerializer(
    EntityDataSerializerEnum::BYTE);
EntityDataSerializer<int>* EntityDataSerializers::INT = new EntityDataSerializers::INT_EntityDataSerializer(
    EntityDataSerializerEnum::INT);
EntityDataSerializer<float>* EntityDataSerializers::FLOAT = new EntityDataSerializers::FLOAT_EntityDataSerializer(
    EntityDataSerializerEnum::FLOAT);
EntityDataSerializer<std::wstring>* EntityDataSerializers::STRING =
    new EntityDataSerializers::STRING_EntityDataSerializer(EntityDataSerializerEnum::STRING);
EntityDataSerializer<not_null_ptr<ItemInstance> >* EntityDataSerializers::ITEM_INSTANCE =
    new EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer(EntityDataSerializerEnum::ITEM_INSTANCE);
EntityDataSerializer<const BlockState*>* EntityDataSerializers::BLOCK_STATE =
    new EntityDataSerializers::BLOCK_STATE_EntityDataSerializer(EntityDataSerializerEnum::BLOCK_STATE);
EntityDataSerializer<bool>* EntityDataSerializers::BOOLEAN = new EntityDataSerializers::BOOLEAN_EntityDataSerializer(
    EntityDataSerializerEnum::BOOLEAN);
EntityDataSerializer<const Rotations*>* EntityDataSerializers::ROTATIONS =
    new EntityDataSerializers::ROTATIONS_EntitySerializer(EntityDataSerializerEnum::ROTATIONS);
EntityDataSerializer<BlockPos>* EntityDataSerializers::BLOCK_POS =
    new EntityDataSerializers::BLOCK_POS_EntityDataSerializer(EntityDataSerializerEnum::BLOCK_POS);
EntityDataSerializer<BlockPos*>* EntityDataSerializers::OPTIONAL_BLOCK_POS =
    new EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer(EntityDataSerializerEnum::OPTIONAL_BLOCK_POS);
EntityDataSerializer<const Direction*>* EntityDataSerializers::DIRECTION =
    new EntityDataSerializers::DIRECTION_EntityDataSerializer(EntityDataSerializerEnum::DIRECTION);
EntityDataSerializer<CompoundTag*>* EntityDataSerializers::COMPOUND_TAG =
    new EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer(EntityDataSerializerEnum::COMPOUND_TAG);

EntityDataSerializers::BLOCK_POS_EntityDataSerializer::BLOCK_POS_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<BlockPos>(id)
{
}
void EntityDataSerializers::BLOCK_POS_EntityDataSerializer::write(DataOutputStream* output, BlockPos value) const
{
    output->writeBlockPos(value);
}
BlockPos EntityDataSerializers::BLOCK_POS_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readBlockPos();
}
EntityDataAccessor<BlockPos>* EntityDataSerializers::BLOCK_POS_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
BlockPos EntityDataSerializers::BLOCK_POS_EntityDataSerializer::copy(BlockPos value) const
{
    return value;
}

EntityDataSerializers::BLOCK_STATE_EntityDataSerializer::BLOCK_STATE_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<const BlockState*>(id)
{
}
void EntityDataSerializers::BLOCK_STATE_EntityDataSerializer::write(DataOutputStream* output,
                                                                    const BlockState* value) const
{
    output->writeVarInt(value == NULL ? 0 : Block::getIdWithData(value));
}
const BlockState* EntityDataSerializers::BLOCK_STATE_EntityDataSerializer::read(DataInputStream* input) const
{
    int id = input->readVarInt();
    return id == 0 ? NULL : Block::stateByIdWithData(id);
}
EntityDataAccessor<const BlockState*>* EntityDataSerializers::BLOCK_STATE_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
const BlockState* EntityDataSerializers::BLOCK_STATE_EntityDataSerializer::copy(const BlockState* value) const
{
    return value;
}

EntityDataSerializers::BOOLEAN_EntityDataSerializer::BOOLEAN_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<bool>(id)
{
}
void EntityDataSerializers::BOOLEAN_EntityDataSerializer::write(DataOutputStream* output, bool value) const
{
    output->writeBoolean(value);
}
bool EntityDataSerializers::BOOLEAN_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readBoolean();
}
EntityDataAccessor<bool>* EntityDataSerializers::BOOLEAN_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
bool EntityDataSerializers::BOOLEAN_EntityDataSerializer::copy(bool value) const
{
    return value;
}

EntityDataSerializers::BYTE_EntityDataSerializer::BYTE_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<unsigned char>(id)
{
}
void EntityDataSerializers::BYTE_EntityDataSerializer::write(DataOutputStream* output, unsigned char value) const
{
    output->writeByte(value);
}
unsigned char EntityDataSerializers::BYTE_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readByte();
}
EntityDataAccessor<unsigned char>* EntityDataSerializers::BYTE_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
unsigned char EntityDataSerializers::BYTE_EntityDataSerializer::copy(unsigned char value) const
{
    return value;
}

EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer::COMPOUND_TAG_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<CompoundTag*>(id)
{
}
void EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer::write(DataOutputStream* output, CompoundTag* value) const
{
    output->writeNbt(value);
}
CompoundTag* EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readNbt();
}
EntityDataAccessor<CompoundTag*>* EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
CompoundTag* EntityDataSerializers::COMPOUND_TAG_EntityDataSerializer::copy(CompoundTag* value) const
{
    return value->copy();
}

EntityDataSerializers::DIRECTION_EntityDataSerializer::DIRECTION_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<const Direction*>(id)
{
}
void EntityDataSerializers::DIRECTION_EntityDataSerializer::write(DataOutputStream* output,
                                                                  const Direction* value) const
{
    output->write(value->get3DDataValue());
}
const Direction* EntityDataSerializers::DIRECTION_EntityDataSerializer::read(DataInputStream* input) const
{
    return Direction::from3DDataValue(input->read());
}
EntityDataAccessor<const Direction*>* EntityDataSerializers::DIRECTION_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
const Direction* EntityDataSerializers::DIRECTION_EntityDataSerializer::copy(const Direction* value) const
{
    return value;
}

EntityDataSerializers::FLOAT_EntityDataSerializer::FLOAT_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<float>(id)
{
}
void EntityDataSerializers::FLOAT_EntityDataSerializer::write(DataOutputStream* output, float value) const
{
    output->writeFloat(value);
}
float EntityDataSerializers::FLOAT_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readFloat();
}
EntityDataAccessor<float>* EntityDataSerializers::FLOAT_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
float EntityDataSerializers::FLOAT_EntityDataSerializer::copy(float value) const
{
    return value;
}

EntityDataSerializers::INT_EntityDataSerializer::INT_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<int>(id)
{
}
void EntityDataSerializers::INT_EntityDataSerializer::write(DataOutputStream* output, int value) const
{
    output->writeVarInt(value);
}
int EntityDataSerializers::INT_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readVarInt();
}
EntityDataAccessor<int>* EntityDataSerializers::INT_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
int EntityDataSerializers::INT_EntityDataSerializer::copy(int value) const
{
    return value;
}

EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer::ITEM_INSTANCE_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<not_null_ptr<ItemInstance> >(id)
{
}
void EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer::write(DataOutputStream* output,
                                                                      not_null_ptr<ItemInstance> value) const
{
    output->writeItem(value);
}
not_null_ptr<ItemInstance> EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readItem();
}
EntityDataAccessor<not_null_ptr<ItemInstance> >* EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer::
    createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
not_null_ptr<ItemInstance> EntityDataSerializers::ITEM_INSTANCE_EntityDataSerializer::copy(
    not_null_ptr<ItemInstance> value) const
{
    return value->copy();
}

EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer::OPTIONAL_BLOCK_POS_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<BlockPos*>(id)
{
}
void EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer::write(DataOutputStream* output,
                                                                           BlockPos* value) const
{
    output->writeBoolean(value != NULL);
    if (value != NULL)
        output->writeBlockPos(*value);
}
BlockPos* EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readBoolean() ? input->readBlockPosPointer() : NULL;
}
EntityDataAccessor<BlockPos*>* EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
BlockPos* EntityDataSerializers::OPTIONAL_BLOCK_POS_EntityDataSerializer::copy(BlockPos* value) const
{
    return value == NULL ? NULL : new BlockPos(*value);
}

EntityDataSerializers::ROTATIONS_EntitySerializer::ROTATIONS_EntitySerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<const Rotations*>(id)
{
}
void EntityDataSerializers::ROTATIONS_EntitySerializer::write(DataOutputStream* output, const Rotations* value) const
{
    output->writeFloat(value->getX());
    output->writeFloat(value->getY());
    output->writeFloat(value->getZ());
}
const Rotations* EntityDataSerializers::ROTATIONS_EntitySerializer::read(DataInputStream* input) const
{
    float x = input->readFloat();
    float y = input->readFloat();
    float z = input->readFloat();
    return new Rotations(x, y, z);
}
EntityDataAccessor<const Rotations*>* EntityDataSerializers::ROTATIONS_EntitySerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
const Rotations* EntityDataSerializers::ROTATIONS_EntitySerializer::copy(const Rotations* value) const
{
    return value == NULL ? NULL : new Rotations(*value);
}

EntityDataSerializers::STRING_EntityDataSerializer::STRING_EntityDataSerializer(
    EntityDataSerializerEnum::EEntityDataSerializer id)
    : EntityDataSerializer<std::wstring>(id)
{
}
void EntityDataSerializers::STRING_EntityDataSerializer::write(DataOutputStream* output, std::wstring value) const
{
    output->writeUTF(value);
}
std::wstring EntityDataSerializers::STRING_EntityDataSerializer::read(DataInputStream* input) const
{
    return input->readUTF();
}
EntityDataAccessor<std::wstring>* EntityDataSerializers::STRING_EntityDataSerializer::createAccessor(int id)
{
    return getOrCreateStoredAccessor(id);
}
std::wstring EntityDataSerializers::STRING_EntityDataSerializer::copy(std::wstring value) const
{
    return value;
}
