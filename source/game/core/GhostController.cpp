#include "game/core/GhostController.hpp"
#include <algorithm>
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/phys__Vec2.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace
{
const long long GHOST_SAMPLE_INTERVAL_MS = 250;
const long long MAX_GHOST_RECORDING_MS = 300000;
const unsigned int GHOST_RECORD_RESERVED_SAMPLES = 1200;
const unsigned int GHOST_RECORD_STREAM_HEADER_SIZE = 12;
const int GHOST_HEADER_VERSION = 3;
const int GHOST_RECORD_VERSION = 4;
const std::wstring GHOST_TIME_KEY_PREFIX = L"GHOSTTIME_";
const std::wstring GHOST_SCORE_KEY_PREFIX = L"GHOSTSCORE_";
const std::wstring GHOST_HEADER_KEY = L"GHOSTDATA_HEADER";
}

GhostController::GhostRecordSample::GhostRecordSample(const GhostRecordSample& sample)
{
    _init(sample.x, sample.y, sample.z, sample.rotX, sample.rotY, sample.rotZ, sample.timestamp);
}
GhostController::GhostRecordSample::GhostRecordSample(short a, short b, short c, short d, short e, short f, long long t)
{
    _init(a, b, c, d, e, f, t);
}
GhostController::GhostRecordSample::GhostRecordSample(DataInputStream* stream)
{
    read(stream);
}
void GhostController::GhostRecordSample::_init(short a, short b, short c, short d, short e, short f, long long t)
{
    x = a;
    y = b;
    z = c;
    rotX = d;
    rotY = e;
    rotZ = f;
    timestamp = t;
}
void GhostController::GhostRecordSample::read(DataInputStream* stream)
{
    timestamp = stream->readLong();
    x = stream->readShort();
    y = stream->readShort();
    z = stream->readShort();
    rotX = stream->readShort();
    rotY = stream->readShort();
    rotZ = stream->readShort();
}
unsigned int GhostController::GhostRecordSample::getSampleSize()
{
    return 20;
}
void GhostController::GhostRecordSample::write(DataOutputStream* stream)
{
    stream->writeLong(timestamp);
    stream->writeShort(x);
    stream->writeShort(y);
    stream->writeShort(z);
    stream->writeShort(rotX);
    stream->writeShort(rotY);
    stream->writeShort(rotZ);
}

GhostController::GhostRecord::GhostRecord(Vec3* pos) : samples(), hasRecording(false), mElapsed(0)
{
    startPos = Vec3::newPermanent(pos);
    samples.reserve(GHOST_RECORD_RESERVED_SAMPLES);
}
GhostController::GhostRecord::~GhostRecord()
{
    delete startPos;
}
Vec3* GhostController::GhostRecord::GetPosAtIndex(unsigned int index)
{
    if (index < samples.size())
    {
        double x = startPos->x + (double)samples[index].x / 10.0;
        double y = startPos->y + (double)samples[index].y / 10.0;
        double z = startPos->z + (double)samples[index].z / 10.0;
        return Vec3::newTemp(x, y, z);
    }
    return NULL;
}

bool GhostController::GhostRecord::HasRecording() const
{
    return hasRecording;
}
void GhostController::GhostRecord::ResetHasRecording()
{
    hasRecording = false;
}
void GhostController::GhostRecord::AddSample(double a, double b, double c, float d, float e, float f)
{
    samples.push_back(GhostRecordSample(
        GhostController::Double1DPAsInt16(a - startPos->x), GhostController::Double1DPAsInt16(b - startPos->y),
        GhostController::Double1DPAsInt16(c - startPos->z), GhostController::Double1DPAsInt16(d),
        GhostController::Double1DPAsInt16(e), GhostController::Double1DPAsInt16(f), mElapsed));
    hasRecording = true;
}
int GhostController::GhostRecord::NumSamples() const
{
    return (int)samples.size();
}
long long GhostController::GhostRecord::GetTimestampAtIndex(unsigned int index)
{
    return index < samples.size() ? samples[index].timestamp : -1;
}
void GhostController::GhostRecord::AddSample(const GhostRecordSample& sample)
{
    samples.push_back(sample);
    hasRecording = true;
}
void GhostController::GhostRecord::WriteStartPos(DataOutputStream* stream)
{
    stream->writeFloat((float)startPos->x);
    stream->writeFloat((float)startPos->y);
    stream->writeFloat((float)startPos->z);
}
void GhostController::GhostRecord::WriteSampleAtIndex(unsigned int index, DataOutputStream* stream)
{
    if (index < samples.size())
        samples[index].write(stream);
}
Vec3* GhostController::GhostRecord::GetRotAtIndex(unsigned int index)
{
    if (index < samples.size())
    {
        return Vec3::newTemp((double)samples[index].rotX / 10.0, (double)samples[index].rotY / 10.0,
                             (double)samples[index].rotZ / 10.0);
    }
    return NULL;
}

float GhostController::GhostRecord::GetVelocityAtIndex(unsigned int index)
{
    if (index != 0 && index < samples.size())
    {
        const float dx = (samples[index].x - samples[index - 1].x) / 10.0f;
        const float dy = (samples[index].y - samples[index - 1].y) / 10.0f;
        const float dz = (samples[index].z - samples[index - 1].z) / 10.0f;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    return 0.0f;
}

short GhostController::Double1DPAsInt16(double value)
{
    const double scaled = value * 10.0 + 0.5;
    int rounded = (int)scaled;
    if (scaled < (double)rounded)
        --rounded;
    return (short)rounded;
}

GhostController::GhostController(int gameMode, const std::wstring& levelName, bool timed)
{
    mSaveBuffer = NULL;
    mCurrentOperation = NO_OPERATION;
    mLoadBuffer = NULL;
    mTimed = true;
    mDone = false;
    mPlaybackIndex = 0;
    mSuspendedTime = 0;
    InitializeCriticalSection(&mQueueLock);
    for (int track = 0; track < 4; ++track)
    {
        mTrackHeaderSaved[track] = true;
        LoadGhostHeader(track);
    }
    std::memset(mRecording, 0, sizeof(mRecording));
    std::memset(mLoaded, 0, sizeof(mLoaded));
    Initialise(gameMode, levelName, timed);
}

GhostController::~GhostController()
{
    for (int track = 0; track < 4; ++track)
    {
        delete mRecording[track];
        mRecording[track] = NULL;
        delete mLoaded[track];
        mLoaded[track] = NULL;
    }
}

void GhostController::addLoadSaveOp(GhostLoadSaveOperation::EOpType type, int track, GhostRecord* record)
{
    EnterCriticalSection(&mQueueLock);
    mQueue.push_back(GhostLoadSaveOperation(type, track, record));
    LeaveCriticalSection(&mQueueLock);
}

void GhostController::LoadGhostRecord(int track)
{
    if (gProfile->IsSignedIn(track))
        addLoadSaveOp(GhostLoadSaveOperation::LOAD_RECORD, track, NULL);
}

void GhostController::Initialise(int gameMode, const std::wstring& levelName, bool timed)
{
    mGameMode = gameMode;
    mLevelName = levelName;
    mTimed = timed;
    mSavingDisabled = false;
    mAnyRecordingActive = false;
    for (int track = 0; track < 4; ++track)
    {
        mRecordingActive[track] = false;
        mRecordingPending[track] = false;
        mPlaybackActive[track] = false;
        delete mRecording[track];
        mRecording[track] = NULL;
        delete mLoaded[track];
        mLoaded[track] = NULL;
        LoadGhostRecord(track);
    }
}

void GhostController::LoadGhostHeader(int track)
{
    if (gProfile->IsSignedIn(track))
        addLoadSaveOp(GhostLoadSaveOperation::LOAD_HEADER, track, NULL);
}

void GhostController::reset(int gameMode, const std::wstring& levelName, bool timed)
{
    mAnyRecordingActive = false;
    mSuspendedTime = 0;
    if (mGameMode != gameMode)
    {
        Initialise(gameMode, levelName, timed);
        return;
    }
    for (int track = 0; track < 4; ++track)
    {
        mRecordingActive[track] = false;
        mRecordingPending[track] = false;
        mPlaybackActive[track] = false;
        if (mRecording[track] != NULL)
        {
            delete mRecording[track];
            mRecording[track] = NULL;
        }
    }
}

std::wstring GhostController::getGhostPlayerName(int track)
{
    return L"GHOST_CONTROLLER_PLAYER_" + _toString<int>(track);
}

void GhostController::createGhostPlayers()
{
    Level* level = Minecraft::GetInstance()->getLevel(0);
    for (int track = 0; track < 4; ++track)
    {
        mGhostPlayers[track] = boost::shared_ptr<GhostPlayer>(new GhostPlayer(level, getGhostPlayerName(track)));
        GhostRecord* record = mLoaded[track];
        if (record != NULL && record->HasRecording())
        {
            Vec3* start = record->GetPosAtIndex(0);
            mGhostPlayers[track]->mX = start->x;
            mGhostPlayers[track]->mY = start->y;
            mGhostPlayers[track]->mZ = start->z;
            mGhostPlayers[track]->mXOld = start->x;
            mGhostPlayers[track]->mYOld = start->y;
            mGhostPlayers[track]->mZOld = start->z;
        }
    }
}

void GhostController::startPlayback()
{
    for (int track = 0; track < 4; ++track)
        mPlaybackActive[track] = mLoaded[track] != NULL;
    mPlaybackIndex = 0;
    mPlaybackStartTime = System::processTimeInMilliSecs();
    createGhostPlayers();
}

boost::shared_ptr<GhostPlayer> GhostController::getGhostPlayer(int track)
{
    if (mLoaded[track] != NULL)
        return mGhostPlayers[track];
    return boost::shared_ptr<GhostPlayer>();
}

std::vector<boost::shared_ptr<GhostPlayer> > GhostController::getGhostPlayers()
{
    std::vector<boost::shared_ptr<GhostPlayer> > players;
    for (int track = 0; track < 4; ++track)
    {
        if (mGhostPlayers[track] != NULL && mLoaded[track] != NULL)
            players.push_back(mGhostPlayers[track]);
    }
    return players;
}

std::vector<boost::shared_ptr<Player> > GhostController::getPlayers()
{
    std::vector<boost::shared_ptr<Player> > players;
    for (int track = 0; track < 4; ++track)
    {
        if (mGhostPlayers[track] != NULL && mLoaded[track] != NULL)
            players.push_back(mGhostPlayers[track]);
    }
    return players;
}

void GhostController::stopPlayback()
{
    for (int track = 0; track < 4; ++track)
        mPlaybackActive[track] = false;
}

void GhostController::stopRecording(int track)
{
    mRecording[track]->ResetHasRecording();
    setRecordingActive(track, false);
    delete mRecording[track];
    mRecording[track] = NULL;
}

#ifdef MINECRAFT_PC
void GhostController::PcCancelRecording(int track)
{
    mRecordingPending[track] = false;
    if (mRecording[track] != NULL)
        stopRecording(track);
    else
        setRecordingActive(track, false);
}
#endif

void GhostController::UpdateGhostRecord(int track, long long elapsed)
{
    boost::shared_ptr<MultiplayerLocalPlayer>& player = Minecraft::GetInstance()->mLocalPlayers[track];
    if (player == NULL)
        return;

    player.get()->getHealth();
    mRecording[track]->mElapsed += std::max(elapsed - mSuspendedTime, 0LL);
    if (mRecording[track]->mElapsed > MAX_GHOST_RECORDING_MS)
    {
        stopRecording(track);
        return;
    }
    mRecording[track]->AddSample(player->mX, player->mY, player->mZ, player->getRotationVector()->x,
                                 player->getRotationVector()->y, player->mFallFlyYaw);
}

void GhostController::setRecordingActive(int track, bool active)
{
    mLastSampleTime = System::processTimeInMilliSecs();
    mRecordingActive[track] = active;
    mAnyRecordingActive = false;
    for (int i = 0; i < 4; ++i)
    {
        if (mRecordingActive[i])
        {
            mAnyRecordingActive = true;
            break;
        }
    }
    if (active)
    {
        mRecordingPending[track] = true;
        UpdateGhostRecord(track, 0);
    }
}

void GhostController::startRecording(int track, Vec3* start)
{
    if (!mAnyRecordingActive)
        mSuspendedTime = 0;
    delete mRecording[track];
    mRecording[track] = new GhostRecord(start);
    setRecordingActive(track, true);
}

unsigned short GhostController::GetBestScore(int track) const
{
    if (mTrackHeaders[track].find(mGameMode) == mTrackHeaders[track].end())
        return 0;
    return mTrackHeaders[track].at(mGameMode).score;
}

bool GhostController::HasFinished(GhostRecord* record)
{
    return record->NumSamples() - 2U <= mPlaybackIndex;
}

unsigned int GhostController::getPlaybackIndex(GhostRecord* record, float& fraction)
{
    if (mGhostPlayers[0] == NULL || record == NULL || !record->HasRecording())
        return mPlaybackIndex;

    const long long elapsed = System::processTimeInMilliSecs() - mPlaybackStartTime;
    unsigned int index = mPlaybackIndex;
    bool advanced = false;
    if (elapsed > 0)
    {
        for (unsigned int sample = index; sample < (unsigned int)record->NumSamples(); ++sample)
        {
            if (elapsed < record->GetTimestampAtIndex(sample))
            {
                index = sample - 1;
                mPlaybackIndex = index;
                advanced = true;
                break;
            }
        }
    }

    if (index + 1 < (unsigned int)record->NumSamples())
    {
        const long long start = record->GetTimestampAtIndex(index);
        const long long end = record->GetTimestampAtIndex(index + 1);
        const float progress = (float)(elapsed - start) / (float)(end - start);
        fraction = 1.0f - progress < 0.0f ? 1.0f : progress;
    }
    else
    {
        fraction = 1.0f;
    }

    if (advanced)
    {
        Vec3* position = record->GetPosAtIndex(index);
        if (position != NULL)
        {
            mGhostPlayers[0]->mX = position->x;
            mGhostPlayers[0]->mY = position->y;
            mGhostPlayers[0]->mZ = position->z;
        }
    }
    return index;
}

std::wstring GhostController::BuildDatakey(bool header)
{
    if (header)
        return GHOST_HEADER_KEY;
    std::wstring key(L"");
    key = mTimed ? GHOST_TIME_KEY_PREFIX : GHOST_SCORE_KEY_PREFIX;
    key.append(mLevelName);
    return key;
}

void GhostController::SaveGhostHeader(int track)
{
    if (gProfile->IsSignedIn(track) && !mSavingDisabled)
        addLoadSaveOp(GhostLoadSaveOperation::SAVE_HEADER, track, NULL);
}

void GhostController::SetTrackHeader(int track, int key, unsigned int time, unsigned short score, bool timeRecorded,
                                     bool scoreRecorded, bool save)
{
    bool changed = false;
    if (mTrackHeaders[track].find(key) == mTrackHeaders[track].end())
    {
        TrackHeader header;
        header.time = time;
        header.score = score;
        header.timeRecorded = timeRecorded;
        header.scoreRecorded = scoreRecorded;
        mTrackHeaders[track][key] = header;
        changed = true;
    }
    else
    {
        if (mTrackHeaders[track][key].time == 0 || time < mTrackHeaders[track][key].time)
        {
            mTrackHeaders[track][key].time = time;
            changed = true;
        }
        if (score > mTrackHeaders[track][key].score)
        {
            mTrackHeaders[track][key].score = score;
            changed = true;
        }
        if (timeRecorded && !mTrackHeaders[track][key].timeRecorded)
        {
            changed = mTrackHeaders[track][key].timeRecorded = true;
        }
        if (scoreRecorded && !mTrackHeaders[track][key].scoreRecorded)
        {
            changed = mTrackHeaders[track][key].scoreRecorded = true;
        }
    }
    if (save && (changed || !mTrackHeaderSaved[track]))
        SaveGhostHeader(track);
}

void GhostController::LoadGhostHeader(int track, void* data, unsigned int length)
{
    ByteArrayInputStream input(arrayWithLength<unsigned char>(static_cast<unsigned char*>(data), length));
    DataInputStream stream(&input);
    if (stream.readUnsignedShort() == GHOST_HEADER_VERSION)
    {
        const unsigned int count = stream.readUnsignedShort();
        for (unsigned int i = 0; i < count; ++i)
        {
            const int key = stream.readUnsignedShort();
            const unsigned int time = stream.readInt();
            const unsigned short score = stream.readUnsignedShort();
            const bool timeRecorded = stream.readBoolean();
            const bool scoreRecorded = stream.readBoolean();
            SetTrackHeader(track, key, time, score, timeRecorded, scoreRecorded, false);
        }
    }
    input.mBuffer.data = NULL;
    input.mBuffer.length = 0;
    input.mCount = 0;
    input.mMark = 0;
    input.mPosition = 0;
}

int GhostController::LoadGhostHeader_Callback(void* context, int track, C4JStorage::eGeneralData_Callback_Status status,
                                              int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (!controller->mDone && status == C4JStorage::eGeneralData_Callback_StatusValue)
    {
        controller->LoadGhostHeader(track, controller->mLoadBuffer, controller->mLoadBufferSize);
        if (controller->mLoadBuffer != NULL)
        {
            std::free(controller->mLoadBuffer);
            controller->mLoadBuffer = NULL;
        }
    }
    controller->mCurrentOperation = NO_OPERATION;
    return 0;
}

unsigned int GhostController::LoadGhostHeaderOp(int track)
{
    return gStorage->ReadFromGenericData(track, BuildDatakey(true), &mLoadBuffer, &mLoadBufferSize,
                                         LoadGhostHeader_Callback, this);
}

GhostController::TrackHeader GhostController::GetTrackHeader(int track, int key) const
{
    boost::unordered_map<int, TrackHeader>::const_iterator found = mTrackHeaders[track].find(key);
    if (found == mTrackHeaders[track].end())
        return TrackHeader();
    return found->second;
}

void GhostController::SaveGhostRecord(int track, GhostRecord* record)
{
    if (gProfile->IsSignedIn(track) && !mSavingDisabled)
        addLoadSaveOp(GhostLoadSaveOperation::SAVE_RECORD, track, record);
}

void GhostController::AddGhostRecord(int track, GhostRecord* record, bool loadedFromStorage)
{
    if (mLoaded[track] != NULL)
        return;
    mLoaded[track] = record;
    if (!loadedFromStorage)
    {
        SaveGhostRecord(track, record);
        SetTrackHeader(track, mGameMode, 0, 0, true, false, true);
    }
}

void GhostController::AddLoadedGhostRecord(int track, void* data, unsigned int length)
{
    arrayWithLength<unsigned char> buffer(static_cast<unsigned char*>(data), length);
    ByteArrayInputStream input(buffer);
    DataInputStream stream(&input);
    if (stream.readUnsignedShort() == GHOST_RECORD_VERSION)
    {
        const float startX = stream.readFloat();
        const float startY = stream.readFloat();
        const float startZ = stream.readFloat();
        GhostRecord* record = new GhostRecord(Vec3::newTemp(startX, startY, startZ));
        record->mElapsed = stream.readLong();
        unsigned int count = stream.readInt();
        for (unsigned int i = 0; i < count; ++i)
            record->AddSample(GhostRecordSample(&stream));
        if (record->HasRecording())
        {
            AddGhostRecord(track, record, true);
            mPlaybackActive[track] = true;
        }
    }
    input.mBuffer.data = NULL;
    input.mBuffer.length = 0;
    input.mCount = 0;
    input.mMark = 0;
    input.mPosition = 0;
}

int GhostController::LoadGhostRecord_Callback(void* context, int track, C4JStorage::eGeneralData_Callback_Status status,
                                              int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (!controller->mDone && status == C4JStorage::eGeneralData_Callback_StatusValue)
    {
        controller->AddLoadedGhostRecord(track, controller->mLoadBuffer, controller->mLoadBufferSize);
        if (controller->mLoadBuffer != NULL)
        {
            std::free(controller->mLoadBuffer);
            controller->mLoadBuffer = NULL;
        }
    }
    controller->mCurrentOperation = NO_OPERATION;
    return 0;
}

unsigned int GhostController::LoadGhostRecordOp(int track)
{
    return gStorage->ReadFromGenericData(track, BuildDatakey(false), &mLoadBuffer, &mLoadBufferSize,
                                         LoadGhostRecord_Callback, this);
}

int GhostController::SaveGhostHeader_Callback(void* context, int, C4JStorage::eGeneralData_Callback_Status, int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller->mSaveBuffer != NULL)
    {
        std::free(controller->mSaveBuffer);
        controller->mSaveBuffer = NULL;
    }
    controller->mCurrentOperation = NO_OPERATION;
    return 0;
}

unsigned int GhostController::SaveGhostHeaderOp(int track)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    stream.writeUnsignedShort(GHOST_HEADER_VERSION);
    stream.writeUnsignedShort((unsigned short)mTrackHeaders[track].size());
    for (boost::unordered_map<int, TrackHeader>::const_iterator it = mTrackHeaders[track].begin();
         it != mTrackHeaders[track].end(); ++it)
    {
        stream.writeUnsignedShort((unsigned short)it->first);
        stream.writeInt((int)it->second.time);
        stream.writeUnsignedShort(it->second.score);
        stream.writeBoolean(it->second.timeRecorded);
        stream.writeBoolean(it->second.scoreRecorded);
    }
    arrayWithLength<unsigned char> bytes = byteStream.toByteArray();
    mSaveBuffer = bytes.data;
    mSaveBufferSize = bytes.length;
    return gStorage->WriteToGenericData(track, BuildDatakey(true), mSaveBuffer, mSaveBufferSize,
                                        SaveGhostHeader_Callback, this);
}

int GhostController::SaveGhostRecord_Callback(void* context, int, C4JStorage::eGeneralData_Callback_Status, int)
{
    GhostController* controller = static_cast<GhostController*>(context);
    if (controller->mSaveBuffer != NULL)
    {
        std::free(controller->mSaveBuffer);
        controller->mSaveBuffer = NULL;
    }
    controller->mCurrentOperation = NO_OPERATION;
    return 0;
}

unsigned int GhostController::SaveGhostRecordOp(int track, GhostRecord* record)
{
    ByteArrayOutputStream byteStream(GhostRecordSample::getSampleSize() * GHOST_RECORD_RESERVED_SAMPLES
                                     + GHOST_RECORD_STREAM_HEADER_SIZE);
    DataOutputStream stream(&byteStream);
    stream.writeUnsignedShort(GHOST_RECORD_VERSION);
    record->WriteStartPos(&stream);
    stream.writeLong(record->mElapsed);
    const unsigned int count = record->NumSamples();
    stream.writeInt(count);
    for (unsigned int i = 0; i < count; ++i)
        record->WriteSampleAtIndex(i, &stream);
    arrayWithLength<unsigned char> bytes = byteStream.toByteArray();
    mSaveBuffer = bytes.data;
    mSaveBufferSize = bytes.length;
    return gStorage->WriteToGenericData(track, BuildDatakey(false), mSaveBuffer, mSaveBufferSize,
                                        SaveGhostRecord_Callback, this);
}

void GhostController::ProcessLoadSaveQueue()
{
    OSFastMutex* lock = &mQueueLock;
    EnterCriticalSection(lock);
    if (mCurrentOperation == NO_OPERATION && !mQueue.empty())
    {
        const GhostLoadSaveOperation operation = mQueue.front();
        mQueue.pop_front();
        switch (operation.type)
        {
        case GhostLoadSaveOperation::LOAD_HEADER:
            mCurrentOperation = operation.type;
            if (LoadGhostHeaderOp(operation.track) == 0)
                mCurrentOperation = NO_OPERATION;
            break;
        case GhostLoadSaveOperation::LOAD_RECORD:
            mCurrentOperation = operation.type;
            if ((mTimed && GetTrackHeader(operation.track, mGameMode).timeRecorded)
                || (!mTimed && GetTrackHeader(operation.track, mGameMode).scoreRecorded))
            {
                if (LoadGhostRecordOp(operation.track) == 0)
                    mCurrentOperation = NO_OPERATION;
            }
            else
            {
                mPlaybackActive[operation.track] = false;
                mCurrentOperation = NO_OPERATION;
            }
            break;
        case GhostLoadSaveOperation::SAVE_HEADER:
            mCurrentOperation = operation.type;
            if (SaveGhostHeaderOp(operation.track) == 0)
                mCurrentOperation = NO_OPERATION;
            break;
        case GhostLoadSaveOperation::SAVE_RECORD:
            mCurrentOperation = operation.type;
            if (SaveGhostRecordOp(operation.track, operation.record) == 0)
                mCurrentOperation = NO_OPERATION;
            break;
        default:
            mCurrentOperation = NO_OPERATION;
            break;
        }
    }
    LeaveCriticalSection(lock);
}

void GhostController::tick()
{
    if (mAnyRecordingActive && !gApp.IsAppPaused())
    {
        const long long now = System::processTimeInMilliSecs();
        if (now >= mLastSampleTime + GHOST_SAMPLE_INTERVAL_MS)
        {
            for (int track = 0; track < 4; ++track)
            {
                if (mRecordingActive[track])
                    UpdateGhostRecord(track, now - mLastSampleTime);
            }
            mLastSampleTime = now;
            mSuspendedTime = 0;
        }
    }
    ProcessLoadSaveQueue();
}

bool GhostController::canBeDeleted()
{
    return mCurrentOperation == NO_OPERATION;
}

void GhostController::setDone()
{
    mDone = true;
}

bool GhostController::finaliseRecording(int track, bool scoreMode)
{
    setRecordingActive(track, false);
    GhostRecord* recording = mRecording[track];
    GhostRecord* loaded;
    bool replaced = false;
    if (scoreMode && recording == NULL)
    {
        if (mLoaded[track] != NULL)
        {
            delete mLoaded[track];
            mLoaded[track] = NULL;
        }
        loaded = NULL;
    }
    else
    {
        loaded = mLoaded[track];
        if (loaded == NULL || (recording != NULL && recording->mElapsed < loaded->mElapsed) || scoreMode)
        {
            replaced = true;
            if (recording != NULL)
            {
                if (loaded != NULL)
                {
                    delete loaded;
                    mLoaded[track] = NULL;
                    recording = mRecording[track];
                }
                AddGhostRecord(track, recording, false);
                mRecording[track] = NULL;
                return true;
            }
        }
    }
    if (!mTrackHeaderSaved[track])
    {
        mLoaded[track] = NULL;
        AddGhostRecord(track, loaded, false);
    }
    if (mRecording[track] != NULL)
    {
        delete mRecording[track];
        mRecording[track] = NULL;
    }
    return replaced;
}

int GhostController::RecordTime(int track, int key, unsigned int time)
{
    if (mSavingDisabled)
    {
        mRecordingPending[track] = false;
        setRecordingActive(track, false);
        return 0;
    }
    const bool recorded = finaliseRecording(track, false);
    int changed = 0;
    if (mTrackHeaders[track].find(key) == mTrackHeaders[track].end())
    {
        TrackHeader header;
        header.time = time;
        mTrackHeaders[track][key] = header;
        changed = 1;
        if (recorded && !mTrackHeaders[track][key].timeRecorded)
        {
            mTrackHeaders[track][key].timeRecorded = true;
            SaveGhostHeader(track);
        }
        else
        {
            SaveGhostHeader(track);
        }
    }
    else
    {
        if (mTrackHeaders[track][key].time == 0 || time < mTrackHeaders[track][key].time)
        {
            changed = 1;
            mTrackHeaders[track][key].time = time;
        }
        if (recorded && !mTrackHeaders[track][key].timeRecorded)
        {
            mTrackHeaders[track][key].timeRecorded = true;
            SaveGhostHeader(track);
        }
        else if (changed != 0 || !mTrackHeaderSaved[track])
        {
            SaveGhostHeader(track);
        }
    }
    mRecordingPending[track] = false;
    return changed;
}

int GhostController::RecordScore(int track, int key, unsigned short score, unsigned int time)
{
    if (mSavingDisabled)
    {
        mRecordingPending[track] = false;
        setRecordingActive(track, false);
        return 0;
    }
    int changed = 0;
    bool saveHeader = true;
    if (mTrackHeaders[track].find(key) == mTrackHeaders[track].end() && score != 0)
    {
        TrackHeader header;
        header.score = score;
        mTrackHeaders[track][key] = header;
        changed = 1;
        mTrackHeaders[track][key].score = score;
        mTrackHeaders[track][key].scoreRecorded = finaliseRecording(track, true);
    }
    else
    {
        if (score > mTrackHeaders[track][key].score && score != 0)
        {
            changed = 1;
            mTrackHeaders[track][key].score = score;
            mTrackHeaders[track][key].scoreRecorded = finaliseRecording(track, true);
        }
        else
        {
            if (score == mTrackHeaders[track][key].score && score != 0)
            {
                GhostRecord* loaded = mLoaded[track];
                if (loaded == NULL || (long long)time < loaded->mElapsed)
                {
                    bool recorded = finaliseRecording(track, true);
                    changed = recorded;
                    mTrackHeaders[track][key].scoreRecorded |= recorded;
                }
            }
            else
            {
                setRecordingActive(track, false);
            }
            if (changed == 0 && mTrackHeaderSaved[track])
                saveHeader = false;
        }
    }
    if (saveHeader)
        SaveGhostHeader(track);
    mRecordingPending[track] = false;
    return changed;
}
