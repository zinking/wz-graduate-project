#pragma  once


#include <pVec.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace PAPI;

GLenum PixFormat(int chan)
{
	return (chan == 1) ? GL_LUMINANCE : (chan == 2) ? 
GL_LUMINANCE_ALPHA :(chan == 3) ? GL_RGB : GL_RGBA;
}
void MakeFishTexture( char* fileName, GLuint& SpotTexID ){
	IplImage* img = cvLoadImage( fileName,-1 );
	//glGenTextures(1, &SpotTexID);
	glBindTexture(GL_TEXTURE_2D, SpotTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, img->width, img->height, 0, GL_BGR_EXT ,
		GL_UNSIGNED_BYTE, img->imageData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//GL_ASSERT(); 



	/*glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
		checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		checkImage);*/

	cvReleaseImage( &img);

}


//void MakePictureTexture( char* fileName ,GLuint&	SpotTexID,GLenum type)
//{
//	//uc4Image Im (fileName);
//	AUX_RGBImageRec* image = auxDIBImageLoad(fileName);
//
//	glGenTextures(1, &SpotTexID);
//	glBindTexture(GL_TEXTURE_2D, SpotTexID);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//GL_CLAMP);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//GL_CLAMP);
//	//GL_ASSERT(); 
//
//	glTexImage2D(GL_TEXTURE_2D, 0, 4, image->sizeX, image->sizeY, 0, type ,
//		GL_UNSIGNED_BYTE, image->data);
//
//
//
//	//gluBuild2DMipmaps(GL_TEXTURE_2D,  GL_RGBA, image->sizeX, image->sizeY,
//	//	GL_RGBA, GL_UNSIGNED_BYTE, image->data);
//
//}

#define SQRT2PI 2.506628274631000502415765284811045253006
#define ONEOVERSQRT2PI (1. / SQRT2PI)
template<class T>
inline T Sqr(const T x)
{
	return x*x;
}
inline double Gaussian2(const double x, const double y, const double sigma)
{
	double oneOverSigma = 1. / sigma;
	return exp(-0.5 * (Sqr(x) + Sqr(y)) * Sqr(oneOverSigma)) * ONEOVERSQRT2PI * oneOverSigma;
}

void MakeGaussianSpotTexture( 	GLuint&	SpotTexID )
{
	const int DIM = 32;
	const int DIM2 = 16;
	const float TEX_SCALE = 6.0;

	glGenTextures(1, &SpotTexID);
	glBindTexture(GL_TEXTURE_2D, SpotTexID);

	float *img = new float[DIM*DIM];


	for(int y=0; y<DIM; y++) {
		for(int x=0; x<DIM; x++) {
			// Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
			if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
				img[y*DIM+x] = 0;
			else {
				img[y*DIM+x] = TEX_SCALE * Gaussian2(x-DIM2, y-DIM2, (DIM*0.15));
			}
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_REPEAT, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//GL_ASSERT();

	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, DIM, DIM, 0, GL_ALPHA ,
		GL_FLOAT, img);
}

void MakeSphereTexture( 	GLuint&	SpotTexID )
{
	const int DIM = 128;
	const int DIM2 = 63;
	const float TEX_SCALE = 6.0;

	glGenTextures(1, (GLuint *)&SpotTexID);
	glBindTexture(GL_TEXTURE_2D, SpotTexID);

	float *img = new float[DIM*DIM];

	pVec light(1,1,3);
	light.normalize();

	for(int y=0; y<DIM; y++) {
		for(int x=0; x<DIM; x++) {
			// Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
			if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
				img[y*DIM+x] = 0;
			else {
				pVec p(x, y, 0);
				p -= pVec(DIM2, DIM2, 0);
				float len = p.length();
				float z = sqrt(DIM2*DIM2 - len*len);
				p.z() = z;
				if(len >= DIM2) {
					img[y*DIM+x] = 0.0;
					continue;
				}

				p.normalize();
				float v = dot(p, light);
				if(v < 0) v = 0;

				img[y*DIM+x] = v;
			}
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//GL_ASSERT();

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
}