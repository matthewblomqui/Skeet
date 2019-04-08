/*************************************************************
 * File: rifle.cpp
 * Author: Br. Burton
 *
 * Description: Contains the function bodies for the rifle class.
 *
 * Please DO NOT share this code with other students from
 *  other sections or other semesters. They may not receive
 *  the same code that you are receiving.
 *************************************************************/

#include "rifle.h"
#include "point.h"
#include "uiDraw.h"

#include <cassert>

void Rifle::setAngle(float angle)
{
	this->angle = angle;
}

void Rifle::draw() const
{
	//assert(angle >= ANGLE_MIN);
	//assert(angle <= ANGLE_MAX);

	drawRect(point, RIFLE_WIDTH, RIFLE_HEIGHT, 90 - angle);
}

void Rifle::moveLeft()
{
	setAngle(angle -= RIFLE_MOVE_AMOUNT);

	if (isPlayer1)
	{
		if (angle < ANGLE_MIN)
		{
			setAngle(ANGLE_MIN);
		}
	}
	else
	{
		if (angle < ANGLE_MIN_PLAYER2)
		{
			setAngle(ANGLE_MIN_PLAYER2);
		}
	}
}


void Rifle::moveRight()
{
	setAngle(angle += RIFLE_MOVE_AMOUNT);

	if (isPlayer1)
	{
		if (angle > ANGLE_MAX)
		{
			setAngle(ANGLE_MAX);
		}
	}
	else
	{
		if (angle > ANGLE_MAX_PLAYER2)
		{
			setAngle(ANGLE_MAX_PLAYER2);
		}
	}
}
