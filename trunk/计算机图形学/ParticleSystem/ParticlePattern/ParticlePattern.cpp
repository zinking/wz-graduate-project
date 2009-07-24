#include "ParticlePattern.h"
#include "Parameter.h"

#include <highgui.h>
#include <iostream>
#include <algorithm>

using namespace std;


const int pcount = 1000;

ParticlePattern::ParticlePattern(void){
	particleCount = pcount;
}

ParticlePattern::ParticlePattern(string filename ){
	ParsePicture( filename );
}

void ParticlePattern::ParsePicture(string filename){
	src = cvLoadImage( filename.c_str() );

	int total_pixel = 0;
	for( int i=0; i < src->width ; i++ ){
		for ( int j=0; j < src->height; j++ ){
			BYTE color = cvGet2D( src,j,i ).val[0];
			if ( color <= 50 ) total_pixel ++;
		}
	}

	sampleRate = (int) sqrt( (float)total_pixel/(float)particleCount );
	if ( sampleRate <= 0 ) sampleRate = 1;

	particle_con.clear();

	for( int i=0; i < src->width ; i+= sampleRate ){
		for ( int j=0; j < src->height; j+= sampleRate ){
			BYTE color = cvGet2D( src,j,i ).val[0];
			if ( color <= 50 ) {
				float pj = (float ) j / ( float ) src->height;
				float pi = (float ) i / ( float ) src->width;
				particle_con.push_back( cvPoint2D32f( pi,pj ));
			}
		}
	}

	this->particleCount = particle_con.size();
	width = src->width;
	height = src->height;
	//reverse( particle_con.begin(),particle_con.end() );
	
	

	ViewSampleResult();

}


void ParticlePattern::CollectMouseGesturePoint(PContainer mpoints){
	float miny=9999.0,minx=9999.0,maxx=0,maxy=0;
	for ( int i = 0 ; i < mpoints.size(); i++ ){
		CvPoint2D32f pt = mpoints[i];
		pt.x > maxx ? maxx = pt.x :NULL;
		pt.x < minx ? minx = pt.x:NULL;
		pt.y > maxy ? maxy =pt.y:NULL;
		pt.y < miny ? miny = pt.y :NULL;
	}
	int interpolate_count = 0;
	interpolate_count = 1000 / mpoints.size();
	int original_count = mpoints.size();

	for ( int i = 0 ; i < original_count-1; i++ ){
		for ( int j=0;j<interpolate_count;j++){
			float miu = (float)j / interpolate_count;
			CvPoint2D32f interPoint = cvPoint2D32f( mpoints[i].x * miu + mpoints[i+1].x*(1-miu) ,
				mpoints[i].y * miu + mpoints[i+1].y*(1-miu) );
			mpoints.push_back( interPoint);
		}
	}

	particle_con.clear();
	width  =  ( maxx - minx);
	height = ( maxy - miny );
	for ( int i = 0 ; i < mpoints.size(); i++ ){
		CvPoint2D32f pt = mpoints[i];
		pt.x -= minx;
		pt.x /= width;
		pt.y -= miny;
		pt.y /= height;
		particle_con.push_back( pt );
	}
	this->particleCount = mpoints.size();
	cout << " mouse gesture  collected " << endl;
	ViewSampleResult();

}

void ParticlePattern::CollectMouseTracePoint(PContainer mpoints){

	system("cls");
	int interpolate_count = 0;
	interpolate_count = move_frames / mpoints.size();
	int original_count = mpoints.size();

	for ( int i = 0 ; i < original_count-1; i++ ){
		for ( int j=0;j<interpolate_count;j++){
			float miu = (float)j / interpolate_count;
			CvPoint2D32f interPoint = cvPoint2D32f( mpoints[i].x * miu + mpoints[i+1].x*(1-miu) ,
				mpoints[i].y * miu + mpoints[i+1].y*(1-miu) );
			mpoints.push_back( interPoint);
		}
	}

	particle_con.clear();

	for ( int i = 0 ; i < mpoints.size()-1; i++ ){
		CvPoint2D32f pt1 = mpoints[i];
		CvPoint2D32f pt2 = mpoints[i+1];
		float length = sqrt( (pt2.x - pt1.x )*(pt2.x - pt1.x ) +(pt2.y - pt1.y )*(pt2.y - pt1.y ) );
		if ( 0 == length ) {
			length =1;
		}
		CvPoint2D32f velo = cvPoint2D32f( (pt2.x - pt1.x )/length , (pt2.y - pt1.y )/length );
		//cout << "velocity:" << velo.x << " " << velo.y << endl;
		//if ( velo.x >= 1 || velo.y >= 1 ) {
		//	cout << "velocity:" << velo.x << " " << velo.y << endl;
		//}

		particle_con.push_back( velo );
	}
	this->particleCount = mpoints.size();
	cout << "Traces Collected " << endl;
	//^%ViewSampleResult();

}


void ParticlePattern::ViewSampleResult(){
	IplImage* preview = cvCreateImage(  cvSize(width,height),IPL_DEPTH_8U, 3 );
	for ( int i=0; i< particle_con.size(); i++ ){
		CvPoint pt = cvPoint( particle_con[i].x * height, particle_con[i].y * width );
		cvCircle( preview, pt, 2,cvScalar(0,0,0.2));
	}

	//cout << particleCount <<"particle points sampled" << endl;

	cvNamedWindow( "Preview");
	cvShowImage("Preview",preview);
	cvReleaseImage( &preview );
};

ParticlePattern::~ParticlePattern(void){
}
