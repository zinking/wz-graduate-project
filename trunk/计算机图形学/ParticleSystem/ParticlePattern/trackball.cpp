

#include "trackball.h"

static quat curr, last;
Quaternion quaternion;
#define  Radius R

CTrackBall::CTrackBall()
{
	R = 0.8;
	ROOT_2_INV = 0.70710678118654752440;
	simulate_trackball( &curr, 0.0, 0.0, 0.0, 0.0 );
}
CTrackBall::~CTrackBall() {}

float CTrackBall::project_to_sphere( float x, float y )
{
	float z;
	float d_sqr, d;

	d_sqr = SQR( x ) + SQR( y );
	d = sqrt( d_sqr );
	if( d < Radius )
	{
		z = sqrt( 2.0 * SQR( Radius ) - d_sqr );
	}
	else
	{
		z = SQR( Radius ) / d;
	}
	return z;
}

void CTrackBall::simulate_trackball( quat *q, float p1x, float p1y, float p2x, float p2y )
{
	if( p1x == p2x && p1y == p2y )
	{
		quaternion.quat_identity( q );
	}
	else
	{
		quat p1, p2, a, d;
		float p1z, p2z;
		float s, t;

		p1z = project_to_sphere( p1x, p1y );
		quaternion.quat_assign( &p1, 0.0, p1x, p1y, p1z );

		p2z = project_to_sphere( p2x, p2y );
		quaternion.quat_assign( &p2, 0.0, p2x, p2y, p2z );

		quaternion.quat_mul( &a, &p1, &p2 );

		a.w = 0.0;
		s = quaternion.quat_norm(&a);
		quaternion.quat_div_real(&a, &a, s);
		
		quaternion.quat_sub(&d, &p1, &p2);
		
		t = quaternion.quat_norm(&d) / (2.0 * Radius * ROOT_2_INV);
		if (t > 1.0) t = 1.0;
		
		quaternion.quat_assign(q, cos(asin(t)), a.x * t, a.y * t, a.z * t);
	}
}

void CTrackBall::create_rotation_matrix(float m[4][4])
{
	 const quat *q = &curr;//四元数-》旋转矩阵
	m[0][0] = 1.0 - 2.0 * (q->y * q->y + q->z * q->z);
	m[0][1] =       2.0 * (q->x * q->y - q->z * q->w);
	m[0][2] =       2.0 * (q->z * q->x + q->w * q->y);
	m[0][3] = 0.0;
	m[1][0] =       2.0 * (q->x * q->y + q->z * q->w);
	m[1][1] = 1.0 - 2.0 * (q->z * q->z + q->x * q->x);
	m[1][2] =       2.0 * (q->y * q->z - q->w * q->x);
	m[1][3] = 0.0;
	m[2][0] =       2.0 * (q->z * q->x - q->w * q->y);
	m[2][1] =       2.0 * (q->y * q->z + q->x * q->w);
	m[2][2] = 1.0 - 2.0 * (q->y * q->y + q->x * q->x);
	m[2][3] = 0.0;
	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = 0.0;
	m[3][3] = 1.0;
}

void CTrackBall::motioni_to_trackball( int begin_x, int begin_y, int x, int y, int height, int width, int *count )
{
	quat t;
		
	simulate_trackball(&last, (2.0 * begin_x - width) / width,
		(height - 2.0 * begin_y) / height,
		(2.0 * x - width) / width,
		(height - 2.0 * y) / height);

	quaternion.quat_mul(&t, &last, &curr);
	curr = t;
		
	if (++count[0] % 97 == 0)
	{
		float n = quaternion.quat_norm(&curr);
		quaternion.quat_div_real(&curr, &curr, n);
	}
}