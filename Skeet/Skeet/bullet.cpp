/*************************************************************
* File: game.cpp
* Author: Br. Burton
*
* Description: Contains the implementations of the
*  method bodies for the game class.
*
* Please DO NOT share this code with other students from
*  other sections or other semesters. They may not receive
*  the same code that you are receiving.
*************************************************************/

#include "bullet.h"

#include "uiDraw.h"
#include "uiInteract.h"
#include "point.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include "math.h"

#include <vector>
using namespace std;

Bullet::Bullet()
{
	setPoint(Point(200, -200));
	//point = getPoint();
	//angle = 60.0;
	alive = true;
}

Bullet::Bullet(Point point)
{
	setPoint(point);
	//angle = -45.0;
	alive = true;
}

Point Bullet::getPoint()
{
	return point;
}

Velocity Bullet::getVelocity()
{
	return velocity;
}

bool Bullet::isAlive()
{
	return alive;
}

void Bullet::setPoint(Point point)
{
	this->point = point;
}

void Bullet::setVelocity(Velocity velocity)
{
	this->velocity = velocity;
}

void Bullet::kill()
{
	alive = false;
}

void Bullet::advance()
{
	point.setX(point.getX() + velocity.getDx());
	point.setY(point.getY() + velocity.getDy());
}

void Bullet::draw()
{
	drawPokeball(point, 5, 1.0, 1.0, 1.0);
	drawHalfCircle(point, 7, 1.0, 0.0, 0.0);
	drawPokeball(point, 3, 0.0, 0.0, 0.0);
	drawPokeball(point, 2, 1.0, 1.0, 1.0);
}

void Bullet::fire(/*Point point, */float angle)
{
	float dx = BULLET_SPEED * (cos(M_PI / 180.0 * angle));
	float dy = BULLET_SPEED * (sin(M_PI / 180.0 * angle));
	velocity.setDx(velocity.getDx() - dx);
	velocity.setDy(velocity.getDy() + dy);
}