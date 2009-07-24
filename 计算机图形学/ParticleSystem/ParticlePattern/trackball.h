/*
*
*	trackball.h
*
*	EXPLANATION:
*		calc trackball matrix
*
*	IMPLEMENTATION:
*		trackball.cpp
*
*	AUTHOR:
*		Daisuke Iwai
*
*/
//READ --------------------------------------------------
#pragma once

#include <math.h>
#include "quat.h"

#define SQR(x) ((x)*(x))

class CTrackBall
{
private:
	


	// 1/sqrt(2)
	double ROOT_2_INV;

	// relative size of trackball
	float R;

public:

	/*
	*	c'tor/d'tor
	*/
	CTrackBall();
	~CTrackBall();

	float project_to_sphere( float x, float y );
	void simulate_trackball( quat *q, float p1x, float p1y, float p2x, float p2y );
	void create_rotation_matrix(float m[4][4]);
	void motioni_to_trackball( int begin_x, int begin_y, int x, int y, int height, int width, int *count );
};
