#pragma once
#include <cv.h>
#include <cxcore.h>

#include <vector>
#include <string>
#include <cmath>



using namespace std;

typedef vector<CvPoint2D32f> PContainer;
typedef PContainer::iterator PItr;
typedef unsigned char BYTE;


class ParticlePattern
{
public:
	ParticlePattern(void);
	ParticlePattern( string filename );
	~ParticlePattern(void);

	void ParsePicture(string filename);
	void CollectMouseGesturePoint(PContainer mpoints);
	void CollectMouseTracePoint(PContainer mpoints);
	
private:

	void ViewSampleResult();//for debug
	int sampleRate;
	IplImage* src;
	int width;
	int height;

public:
	int particleCount;
	PContainer particle_con;

};
