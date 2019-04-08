/*************************************************************
 * File: rifle.h
 * Author: Br. Burton
 *
 * Description: Defines a Rifle.
 *
 * Please DO NOT share this code with other students from
 *  other sections or other semesters. They may not receive
 *  the same code that you are receiving.
 *************************************************************/

#ifndef RIFLE_H
#define RIFLE_H

#include "point.h"

#define RIFLE_WIDTH 10
#define RIFLE_HEIGHT 40

#define ANGLE_MAX 90
#define ANGLE_MIN 0
#define ANGLE_START 45

#define ANGLE_MAX_PLAYER2 0
#define ANGLE_MIN_PLAYER2 -90
#define ANGLE_START_PLAYER2 -45

#define RIFLE_MOVE_AMOUNT 3

class Rifle
{
private:
	Point point;
	bool isPlayer1;

	/**********************************************************
	 * angle - The angle of the rifles in degrees.
	 *  Assumes that straight right is 0 degrees and up is 90.
	 **********************************************************/
	float angle;


public:
	Rifle(const Point & point) : point(point) { angle = ANGLE_START; isPlayer1 = true; }

	Rifle(const Point & point, float angle) : point(point) { this->angle = angle; isPlayer1 = false; }

	/****************
	 * Basic Getters
	 ****************/
	float getAngle() const { return angle; }
	void setAngle(float angle);
	Point getPoint() const { return point; }

	/*****************
	 * Drawing
	 *****************/
	void draw() const;

	/*****************
	 * Movement
	 *****************/
	void moveLeft();
	void moveRight();

};

#endif
