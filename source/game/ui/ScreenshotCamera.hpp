#ifndef GAME_UI_SCREENSHOTCAMERA_HPP
#define GAME_UI_SCREENSHOTCAMERA_HPP

#include <boost/shared_ptr.hpp>

class Entity;

class ScreenshotCamera
{
public:
    ScreenshotCamera();
    void Capture(boost::shared_ptr<Entity> entity);
    void Load(boost::shared_ptr<Entity> entity, int value);
};

#endif
