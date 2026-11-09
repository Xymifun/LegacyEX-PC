#ifndef GAME_CORE_CAMERACONTROLLER_HPP
#define GAME_CORE_CAMERACONTROLLER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"

class DataInputStream;
class DataOutputStream;
class Player;

class CameraController
{
public:
    struct SplineNode
    {
        Vec3* position;
        Vec3* rotation;
        float time;

        SplineNode(Vec3* nodePosition, Vec3* nodeRotation, float nodeTime)
            : position(nodePosition),
              rotation(nodeRotation),
              time(nodeTime)
        {
        }
    };

    CameraController(int cameraMode);
    ~CameraController();

    void reset();
    void clear();
    void attachToPlayer(boost::shared_ptr<Player> player, bool active);
    int getCameraMode();
    static Vec3* CalculateBezierPoint(float t, Vec3* p0, Vec3* p1, Vec3* p2, Vec3* p3);
    void CalculateCurveLengths();
    static double MapTimeToCurveDistance(std::vector<double>& curveLengths, double time);
    void insertNode(int index, double x, double y, double z, double xRot, double yRot, double zRot, float time);
    void addNode(double x, double y, double z, double xRot, double yRot, double zRot, float time);
    Vec3* getCurrentPos();
    Vec3* getCurrentRot();
    void update(float delta);
    void Deserialise(DataInputStream* stream);
    void Serialise(DataOutputStream* stream);

    int getCurveType();
    std::vector<SplineNode>* getNodesAndControls();
    std::vector<SplineNode>* getNodes();
    Vec3* getPointForPos(Vec3* position);
    void deletePointByPos(Vec3* position);
    void addNodeAfter(Vec3* position);
    void addNodeBefore(Vec3* position);
    std::vector<Vec3*> GetDrawingPoints();
    static Vec3* CalculateBezierPointTangent(float t, Vec3* p0, Vec3* p1, Vec3* p2, Vec3* p3);
    std::vector<Vec3*> GetDrawingNormals();
    bool LoadCameraPath(std::wstring path);
    bool SaveCameraPath(std::wstring path);

private:
    Player* mPlayer;
    std::vector<SplineNode> mNodes;
    std::vector<std::vector<double> > mCurveLengths;
    Vec3* mFromPos;
    Vec3* mFromRot;
    Vec3* mToPos;
    Vec3* mToRot;
    Vec3* mCurrentPos;
    Vec3* mCurrentRot;
    float mTime;
    float mDuration;
    int mCurveType;
    int mCameraMode;
    int mPlayerCameraMode;
    int mCurrentNode;
    int mCurveIndex;
};

#endif
