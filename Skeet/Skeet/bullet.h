#pragma once
#ifndef BULLET_H
#define BULLET_H

#include <vector>
#include "uiInteract.h"
#include "point.h"
#include "velocity.h"
#include "rifle.h"
#include "flyingObjects.h"
#include <cmath>

#define BULLET_SPEED 10.0
class Bullet : public FlyingObjects
{
private:
	Point point;
	Velocity velocity;
	float angle;
	bool alive;
public:
	Bullet();
	Bullet(Point point);
	Point getPoint();
	Velocity getVelocity();
	bool isAlive();
	void setPoint(Point point);
	void setVelocity(Velocity velocity);
	void kill();
	void advance();
	void draw();
	void fire(/*Point point, */float angle);
};

#define CLOSE_ENOUGH 15
#endif
