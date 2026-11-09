#include "game/core/CameraController.hpp"

#include <algorithm>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/generation/Player.hpp"

CameraController::CameraController(int cameraMode)
{
    mCameraMode = cameraMode;
    mPlayerCameraMode = 0;
    mPlayer = NULL;
    mFromPos = NULL;
    mFromRot = NULL;
    mToPos = NULL;
    mToRot = NULL;
    mCurveType = cameraMode != 3;
    mCurrentPos = Vec3::newPermanent(0.0, 0.0, 0.0);
    mCurrentRot = Vec3::newPermanent(0.0, 0.0, 0.0);
    mCurveIndex = 0;
    mCurrentNode = -1;
    mTime = 0.0f;
    mDuration = 0.0f;
}

CameraController::~CameraController()
{
    clear();
    delete mCurrentPos;
    delete mCurrentRot;
    mCurrentNode = -1;
    mCurveIndex = 0;
}

void CameraController::reset()
{
    mTime = 0.0f;
    mDuration = 0.0f;
    mCurrentNode = -1;
    mCurveIndex = 0;
    mFromPos = NULL;
    mFromRot = NULL;
    mToPos = NULL;
    mToRot = NULL;
}

void CameraController::clear()
{
    std::vector<SplineNode>::iterator it;
    for (it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        delete it->position;
        delete it->rotation;
    }
    mNodes.clear();
    reset();
}

void CameraController::attachToPlayer(boost::shared_ptr<Player> player, bool active)
{
    if (mPlayer != NULL)
    {
        mPlayer->SetThirdPersonView(mPlayerCameraMode);
        mPlayer->SetCameraController(NULL, true);
    }
    mPlayer = player.get();
    mPlayerCameraMode = player->GetThirdPersonView(true);
    player->SetCameraController(this, active);
}

int CameraController::getCameraMode()
{
    return mCameraMode;
}

Vec3* CameraController::CalculateBezierPoint(float t, Vec3* p0, Vec3* p1, Vec3* p2, Vec3* p3)
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    Vec3* point = Vec3::newTemp(p0)->multiply(uuu);
    point = point->add(Vec3::newTemp(p1)->multiply(3.0f * uu * t));
    point = point->add(Vec3::newTemp(p2)->multiply(3.0f * u * tt));
    point = point->add(Vec3::newTemp(p3)->multiply(ttt));
    return point;
}

void CameraController::CalculateCurveLengths()
{
    double total = 0.0;
    Vec3* previous = NULL;
    mCurveLengths.clear();
    int segment;
    for (segment = 0; segment < (int)mNodes.size() - 3; segment += 3)
    {
        total = 0.0;
        mCurveLengths.push_back(std::vector<double>());
        mCurveLengths.back().push_back(0.0);
        Vec3* p0 = mNodes[segment].position;
        Vec3* p1 = mNodes[segment + 1].position;
        Vec3* p2 = mNodes[segment + 2].position;
        Vec3* p3 = mNodes[segment + 3].position;
        if (segment == 0)
        {
            previous = CalculateBezierPoint(0.0f, p0, p1, p2, p3);
        }
        int sample;
        for (sample = 1; sample < 101; ++sample)
        {
            float t = (float)sample / 100.0f;
            Vec3* last = previous;
            previous = CalculateBezierPoint(t, p0, p1, p2, p3);
            total += last->vectorTo(previous)->length();
            mCurveLengths.back().push_back(total);
        }
    }
}

double CameraController::MapTimeToCurveDistance(std::vector<double>& curveLengths, double time)
{
    double target = time * curveLengths[100];
    int low = 0;
    int high = 100;
    int mid = 0;
    while (low < high)
    {
        mid = low + (high - low) / 2;
        if (curveLengths[mid] < target)
            low = mid + 1;
        else
            high = mid;
    }
    if (curveLengths[mid] > target)
        --mid;
    double lower = curveLengths[mid];
    if (lower == target)
        return (double)mid / 100.0;
    double fraction = (target - lower) / (curveLengths[mid + 1] - lower);
    return ((double)mid + fraction) / 100.0;
}

void CameraController::insertNode(int index, double x, double y, double z, double xRot, double yRot, double zRot,
                                  float time)
{
    Vec3* position = Vec3::newPermanent(x, y, z);
    if (index == 0)
    {
        SplineNode node(position, Vec3::newPermanent(xRot, yRot, zRot), time);
        mNodes.insert(mNodes.begin(), node);
        Vec3* control = Vec3::newPermanent(
            position->add(mPlayer->calculateViewVector((float)xRot, (float)yRot)->multiply(4.0f)));
        SplineNode controlNode(control, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f);
        std::vector<SplineNode>::iterator next = mNodes.begin();
        ++next;
        mNodes.insert(next, controlNode);
        SplineNode* nodes = &mNodes[0];
        if (mNodes.size() > 2)
        {
            double controlLength = nodes[2].position->vectorTo(nodes[0].position)->length() * 0.25;
            if (controlLength > 4.0)
                controlLength = 4.0;
            Vec3* nextPosition = nodes[2].position;
            Vec3* nextControl = Vec3::newPermanent(
                nextPosition->add(nodes[3].position->vectorTo(nextPosition)->normalize()->multiply(controlLength)));
            SplineNode nextControlNode(nextControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f);
            mNodes.insert(mNodes.begin() + 2, nextControlNode);
            CalculateCurveLengths();
            return;
        }
    }
    else
    {
        Vec3* direction = position->vectorTo(mNodes[index - 2].position);
        double controlLength = direction->length() * 0.25;
        if (controlLength > 4.0)
            controlLength = 4.0;
        Vec3* firstControl = Vec3::newPermanent(position->add(direction->normalize()->multiply(controlLength)));
        Vec3* secondControl = Vec3::newPermanent(
            position->add(firstControl->vectorTo(position)->normalize()->multiply(controlLength)));
        SplineNode firstControlNode(firstControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f);
        mNodes.insert(mNodes.begin() + index, firstControlNode);
        SplineNode node(position, Vec3::newPermanent(xRot, yRot, zRot), time);
        mNodes.insert(mNodes.begin() + index + 1, node);
        SplineNode secondControlNode(secondControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f);
        mNodes.insert(mNodes.begin() + index + 2, secondControlNode);
    }
    CalculateCurveLengths();
}

void CameraController::addNode(double x, double y, double z, double xRot, double yRot, double zRot, float time)
{
    if (mCurveType == 0)
    {
        SplineNode node(Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), time);
        mNodes.push_back(node);
    }
    else if (mCurveType == 1)
    {
        insertNode((int)mNodes.size(), x, y, z, xRot, yRot, zRot, time);
    }
}

Vec3* CameraController::getCurrentPos()
{
    return mCurrentPos;
}

Vec3* CameraController::getCurrentRot()
{
    return mCurrentRot;
}

void CameraController::update(float delta)
{
    if (mCurveType == 0)
    {
        if (mNodes.size() <= 1)
            return;
        if (mCurrentNode == -1)
        {
            mFromPos = mNodes[0].position;
            mFromRot = mNodes[0].rotation;
            mToPos = mNodes[1].position;
            mToRot = mNodes[1].rotation;
            mDuration = mNodes[1].time;
            mCurrentNode = 2;
        }
        float part = (mTime + delta) / mDuration;
        float blend = part * part * (3.0f - 2.0f * part);
        blend = std::min(1.0f, blend);
        mTime = mTime + delta;
        *mCurrentPos = *mFromPos->interpolateTo(mToPos, blend);
        *mCurrentRot = *mFromRot->interpolateTo(mToRot, blend);
        if (mTime <= mDuration)
            return;
        if (mCurrentNode < (int)mNodes.size())
        {
            mFromPos = mToPos;
            mFromRot = mToRot;
            mDuration = mNodes[mCurrentNode].time;
            mToPos = mNodes[mCurrentNode].position;
            mToRot = mNodes[mCurrentNode].rotation;
            mTime = 0.0f;
            ++mCurrentNode;
            return;
        }
        reset();
        if (mPlayer != NULL)
        {
            mPlayer->SetThirdPersonView(mPlayerCameraMode);
            mPlayer->SetCameraController(NULL, true);
        }
    }
    else if (mCurveType == 1 && mNodes.size() > 1)
    {
        if (mCurrentNode == -1)
            mCurrentNode = 0;
        double time = (float)(mTime + delta);
        double distance = time * 0.2f;
        double curveLength = mCurveLengths[mCurveIndex][100];
        while (curveLength < distance)
        {
            if (mCurrentNode >= (int)mNodes.size() - 4 || mCurveLengths.size() <= (unsigned int)mCurveIndex)
                break;
            distance -= curveLength;
            ++mCurveIndex;
            mCurrentNode += 3;
            time = (float)(distance / 0.2f);
            if ((unsigned int)mCurveIndex < mCurveLengths.size())
                curveLength = mCurveLengths[mCurveIndex][100];
        }
        mTime = (float)time;
        if (mCurrentNode < (int)mNodes.size() - 4)
        {
            Vec3* p0 = mNodes[mCurrentNode].position;
            Vec3* p1 = mNodes[mCurrentNode + 1].position;
            Vec3* p2 = mNodes[mCurrentNode + 2].position;
            Vec3* p3 = mNodes[mCurrentNode + 3].position;
            double part = distance / curveLength;
            double curvePosition = MapTimeToCurveDistance(mCurveLengths[mCurveIndex], part);
            mCurrentPos->set(CalculateBezierPoint((float)curvePosition, p0, p1, p2, p3));
            *mCurrentRot = *mNodes[mCurrentNode].rotation->interpolateTo(mNodes[mCurrentNode + 3].rotation, part);
            return;
        }
        reset();
        if (mPlayer != NULL)
        {
            mPlayer->SetThirdPersonView(mPlayerCameraMode);
            mPlayer->SetCameraController(NULL, true);
        }
    }
}

void CameraController::Deserialise(DataInputStream* stream)
{
    int count = stream->readInt();
    if (count < 1)
    {
        count = stream->readInt();
        int i;
        for (i = 0; i < count; ++i)
        {
            double x = stream->readDouble();
            double y = stream->readDouble();
            double z = stream->readDouble();
            double xRot = stream->readDouble();
            double yRot = stream->readDouble();
            double zRot = stream->readDouble();
            SplineNode node(Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), -1.0f);
            mNodes.push_back(node);
        }
        CalculateCurveLengths();
    }
}

void CameraController::Serialise(DataOutputStream* stream)
{
    stream->writeInt(0);
    stream->writeInt((int)mNodes.size());
    std::vector<SplineNode>::const_iterator it;
    for (it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        stream->writeDouble(it->position->x);
        stream->writeDouble(it->position->y);
        stream->writeDouble(it->position->z);
        stream->writeDouble(it->rotation->x);
        stream->writeDouble(it->rotation->y);
        stream->writeDouble(it->rotation->z);
    }
}
