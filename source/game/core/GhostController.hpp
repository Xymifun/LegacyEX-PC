#ifndef GAME_CORE_GHOSTCONTROLLER_HPP
#define GAME_CORE_GHOSTCONTROLLER_HPP
#include <string>
#include <vector>
#include <deque>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/util/Vec3.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "game/core/CriticalSection.hpp"
class DataInputStream;
class DataOutputStream;
class Player;
class GhostPlayer;
class GhostController
{
    friend class ClientMasterGameMode;
    friend class MasterGameMode;
    friend class GameRenderer;
    friend class UIScene_HUD;

public:
    struct GhostRecordSample
    {
        short x;
        short y;
        short z;
        short rotX;
        short rotY;
        short rotZ;
        long long timestamp;
        GhostRecordSample();
        GhostRecordSample(const GhostRecordSample&);
        GhostRecordSample(short, short, short, short, short, short, long long);
        GhostRecordSample(DataInputStream*);
        void _init(short, short, short, short, short, short, long long);
        void read(DataInputStream*);
        static unsigned int getSampleSize();
        void write(DataOutputStream*);
    };
    struct GhostRecord
    {
        std::vector<GhostRecordSample> samples;
        bool hasRecording;
        Vec3* startPos;
        long long mElapsed;
        GhostRecord(Vec3*);
        ~GhostRecord();
        Vec3* GetPosAtIndex(unsigned int);
        bool HasRecording() const;
        void ResetHasRecording();
        void AddSample(double, double, double, float, float, float);
        int NumSamples() const;
        long long GetTimestampAtIndex(unsigned int);
        void AddSample(const GhostRecordSample&);
        void WriteStartPos(DataOutputStream*);
        void WriteSampleAtIndex(unsigned int, DataOutputStream*);
        Vec3* GetRotAtIndex(unsigned int);
        float GetVelocityAtIndex(unsigned int);
    };
    struct TrackHeader
    {
        unsigned int time;
        unsigned short score;
        bool timeRecorded;
        bool scoreRecorded;
        TrackHeader() : time(0), score(0), timeRecorded(false), scoreRecorded(false) {}
    };
    struct GhostLoadSaveOperation
    {
        enum EOpType
        {
            LOAD_HEADER,
            LOAD_RECORD,
            SAVE_HEADER,
            SAVE_RECORD
        };
        EOpType type;
        int track;
        GhostRecord* record;
        GhostLoadSaveOperation(EOpType opType, int opTrack, GhostRecord* opRecord)
            : type(opType),
              track(opTrack),
              record(opRecord)
        {
        }
    };
    GhostController(int, const std::wstring&, bool);
    ~GhostController();
    static short Double1DPAsInt16(double);
    void addLoadSaveOp(GhostLoadSaveOperation::EOpType, int, GhostRecord*);
    void LoadGhostRecord(int);
    void Initialise(int, const std::wstring&, bool);
    void LoadGhostHeader(int);
    void reset(int, const std::wstring&, bool);
    std::wstring getGhostPlayerName(int);
    void createGhostPlayers();
    void startPlayback();
    boost::shared_ptr<GhostPlayer> getGhostPlayer(int);
    std::vector<boost::shared_ptr<GhostPlayer> > getGhostPlayers();
    std::vector<boost::shared_ptr<Player> > getPlayers();
    void stopPlayback();
    void stopRecording(int);
    void UpdateGhostRecord(int, long long);
    void setRecordingActive(int, bool);
    void startRecording(int, Vec3*);
    unsigned short GetBestScore(int) const;
    bool HasFinished(GhostRecord*);
    unsigned int getPlaybackIndex(GhostRecord*, float&);
    std::wstring BuildDatakey(bool);
    void SaveGhostHeader(int);
    void SetTrackHeader(int, int, unsigned int, unsigned short, bool, bool, bool);
    void LoadGhostHeader(int, void*, unsigned int);
    static int LoadGhostHeader_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
    unsigned int LoadGhostHeaderOp(int);
    TrackHeader GetTrackHeader(int, int) const;
    void SaveGhostRecord(int, GhostRecord*);
    void AddGhostRecord(int, GhostRecord*, bool);
    void AddLoadedGhostRecord(int, void*, unsigned int);
    static int LoadGhostRecord_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
    unsigned int LoadGhostRecordOp(int);
    static int SaveGhostHeader_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
    unsigned int SaveGhostHeaderOp(int);
    static int SaveGhostRecord_Callback(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
    unsigned int SaveGhostRecordOp(int, GhostRecord*);
    void ProcessLoadSaveQueue();
    void tick();
    bool canBeDeleted();
    void setDone();
    bool finaliseRecording(int, bool);
    int RecordTime(int, int, unsigned int);
    int RecordScore(int, int, unsigned short, unsigned int);
    static void DisableSaving(void*, int, C4JStorage::EMessageResult);
#ifdef MINECRAFT_PC
    void PcCancelRecording(int track);
#endif

private:
    static const int NO_OPERATION = 6;

    boost::unordered_map<int, TrackHeader> mTrackHeaders[4];
    long long mLastSampleTime;
    long long mPlaybackStartTime;
    long long mSuspendedTime;
    bool mRecordingActive[4];
    bool mRecordingPending[4];
    bool mAnyRecordingActive;
    bool mPlaybackActive[4];
    unsigned int mPlaybackIndex;
    unsigned char* mSaveBuffer;
    unsigned int mSaveBufferSize;
    void* mLoadBuffer;
    unsigned int mLoadBufferSize;
    int mGameMode;
    std::wstring mLevelName;
    bool mTimed;
    GhostRecord* mRecording[4];
    GhostRecord* mLoaded[4];
    boost::shared_ptr<GhostPlayer> mGhostPlayers[4];
    bool mTrackHeaderSaved[4];
    bool mDone;
    bool mSavingDisabled;
    std::deque<GhostLoadSaveOperation> mQueue;
    OSFastMutex mQueueLock;
    int mCurrentOperation;
};
#endif
