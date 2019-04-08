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

#include "bird.h"

#include "uiDraw.h"
#include "uiInteract.h"
#include "point.h"

#include <vector>
using namespace std;

Bird::Bird()
{
	point.setX(-200);
	point.setY(random(-200, 200));
	velocity.setDx(velocity.getDx() + random(3, 6));
	if (point.getY() > 0)
		velocity.setDy(velocity.getDy() + random(-4, 0));
	else if (point.getY() < 0)
		velocity.setDy(velocity.getDy() + random(1, 4));
	else
		velocity.setDy(velocity.getDy() + random(-4, 4));
	alive = true;
}

int Bird::hit()
{
	alive = false;
	return 1;
}

Point Bird::getPoint()
{
	return point;
}

Velocity Bird::getVelocity()
{
	return velocity;
}

bool Bird::isAlive()
{
	if (alive)
		return true;
	else
		return false;
}

void Bird::setAlive(bool alive)
{
	this->alive = alive;
}

void Bird::setPoint(Point point)
{
	this->point = point;
}

void Bird::setVelocity(Velocity velocity)
{
	this->velocity = velocity;
}

void Bird::kill()
{
	alive = false;
}

void Bird::advance()
{
	point.setX(point.getX() + velocity.getDx());
	point.setY(point.getY() + velocity.getDy());
}

void SacredBird::draw()
{
	drawSacredBird(getPoint(), 15.0);
	drawEevee(getPoint());
	//drawLucario(Point(0,0));
}

int SacredBird::hit()
{
	setAlive(false);
	return -20;
}

ToughBird::ToughBird()
{
	health = 3;
	velocity.setDx(velocity.getDx() + random(2, 4));
	if (point.getY() > 0)
		velocity.setDy(velocity.getDy() + random(-3, 0));
	else if (point.getY() < 0)
		velocity.setDy(velocity.getDy() + random(1, 3));
	else
		velocity.setDy(velocity.getDy() + random(-3, 3));
}

void ToughBird::draw()
{
	drawToughBird(getPoint(), 15.0, health);
	//drawLucario(getPoint());
}

void ToughBird::setHealth(int health)
{
	this->health = health;
}

int ToughBird::hit()
{
	if (health == 3 || health == 2)
	{
		setHealth(health - 1);
		setAlive(true);
		return 1;
	}
	else
	{
		setAlive(false);
		return 5;
	}
}


void StandardBird::draw()
{
	drawCircle(Bird::getPoint(), 15.0);
	//drawAngryBird(getPoint());
	drawLucario(getPoint());
}

