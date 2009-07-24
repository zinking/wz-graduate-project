#pragma  once
#include <windows.h>
#include <pAPI.h>
#include "ParticlePattern.h"
#include <cmath>
#include "Parameter.h"


using namespace PAPI;

class ParticleAction{
public :
	bool finished ;
	ParticleAction(){ finished = false; }
	virtual ~ParticleAction(){}
	virtual void Update(ParticleContext_t& P){}
	virtual void Resume( ){}


};

class MoveAction:public ParticleAction{
public:
	MoveAction( pVec dir, int dur ): direction(dir), duration(dur){
	}
	virtual ~MoveAction(){}

	virtual void Update( ParticleContext_t& P ){
		static int frameNO = 0;
		if ( frameNO > duration ) {
			finished = true;
			frameNO = 0;
			return;
		}
		P.TargetVelocity( direction,0.1 );
		P.RandomAccel(PDSphere( pVec(0,0,0), disturbance ));
		CheckBoundary(P);
		frameNO++;
	}
	virtual void Resume(){
		finished = false;
	}

	void CheckBoundary( ParticleContext_t& P ){
		int cnt = (int)P.GetGroupCount();
		if(cnt < 1) return;

		float *ptr;
		size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

		cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
			size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
			up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
		if(cnt < 1) return;//cnt indicates the number of particle in particle context
		pVec* pos	  = (pVec*)(ptr + pos3Ofs);

		for(int i = 0; i < cnt; i++){
			if (  pos->x() > xEndBoundary ) pos->x() = xStartBondary;
			pos	= (pVec*)( (float*) pos  +  int(flstride))	;
		}

	}

private:
	pVec direction;
	int duration;
	

};

class SnakeAction:public ParticleAction{
public:
	SnakeAction( int dur,ParticlePattern p ) :duration(dur),ges(p){
	}
	virtual ~SnakeAction(){}

	virtual void Update( ParticleContext_t& P ){
		static int frameNO = 0;
		//if ( frameNO == 0 ) this->MoveAlong( P, ges, true );
		if ( frameNO > duration ) {
			finished = true;
			frameNO = 0;
			return;
		}
		MoveAlong( P,ges );
		P.RandomAccel(PDSphere( pVec(0,0,0), disturbance ));
		frameNO++;
	}
	virtual void Resume(){
		finished = false;
	}

private:
	pVec direction;
	int duration;
	ParticlePattern ges;

	void MoveAlong( ParticleContext_t& P, ParticlePattern& ges , bool stop = false ){
		static int gesture_index = 0;
		int cnt = (int)P.GetGroupCount();
		if(cnt < 1) return;

		float *ptr;
		size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

		cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
			size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
			up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
		if(cnt < 1) return;//cnt indicates the number of particle in particle context

		pVec* pvel	  = (pVec*)(ptr + vel3Ofs);
		if ( stop ){//first make all particles stop 
			for(int i = 0; i < cnt; i++){
				pvel->x() = 0; pvel->y() = 0; pvel->z() = 0;
				pvel	= (pVec*)( (float*) pvel  +  int(flstride) );
			}
		}
		else{// then make all particels move along the gestures
			pVec currentVelocity;
			if ( gesture_index >= ges.particleCount-1 ) {
				finished = true;
				gesture_index = 0;
			}
			else {
				currentVelocity = pVec( ges.particle_con[gesture_index].x ,
												   ges.particle_con[gesture_index].y ,
					0 ) * 0.02;
				gesture_index++;
				

			}//setting current phase velocity

			for(int i = 0; i < cnt; i++){
				//pvel = currentVelocity;
				pvel->x() = currentVelocity.x();
				pvel->y() = currentVelocity.y();
				pvel->z() = currentVelocity.z();
				pvel	= (pVec*)( (float*) pvel  +  int(flstride) );
			}
		}



	}

};

//class SwingAction:public ParticleAction{
//public:
//	SwingAction( pVec dir, int dur ): direction(dir), duration(dur){
//	}
//	virtual ~SwingAction(){}
//
//	virtual void Update( ParticleContext_t& P ){
//		static int frameNO = 0;
//		if ( frameNO == 0 ) this->MoveAlong( P, ges, true );
//		if ( frameNO > duration ) {
//			finished = true;
//			frameNO = 0;
//			return;
//		}
//		P.TargetVelocity( direction,0.1 );
//		//MoveAlong( P, NULL, true  );
//		frameNO++;
//	}
//
//	void MoveAlong( ParticleContext_t& P, ParticlePattern& ges , bool stop = false ){
//		static int gesture_index = 0;
//		int cnt = (int)P.GetGroupCount();
//		if(cnt < 1) return;
//
//		float *ptr;
//		size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;
//
//		cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
//			size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
//			up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
//		if(cnt < 1) return;//cnt indicates the number of particle in particle context
//
//		pVec* pvel	  = (pVec*)(ptr + vel3Ofs);
//		if ( stop ){//first make all particles stop 
//			for(int i = 0; i < cnt; i++){
//				pvel->x() = 0; pvel->y() = 0; pvel->z() = 0;
//				pvel	= (pVec*)( (float*) pvel  +  int(flstride) );
//			}
//		}
//		else{// then make all particels move along the gestures
//			pVec currentVelocity;
//			if ( gesture_index >= ges.particleCount ) {
//				finished = true;
//				gesture_index = 0;
//			}
//			else {
//				currentVelocity = pVec( ges.particle_con[gesture_index+1].x - ges.particle_con[gesture_index].x ,
//							ges.particle_con[gesture_index+1].y - ges.particle_con[gesture_index].y ,
//						0 ) * 0.01;
//				
//			}//setting current phase velocity
//
//			for(int i = 0; i < cnt; i++){
//				pvel = currentVelocity;
//				pvel	= (pVec*)( (float*) pvel  +  int(flstride) );
//			}
//		}
//		
//
//
//	}
//
//	virtual void Resume(){
//		finished = false;
//	}
//
//private:
//	pVec direction;
//	int duration;
//
//};

class TargetPatternAction : public ParticleAction{
public:
	TargetPatternAction( int du, int& ph, ParticlePattern pt,pDomain* bd ):pattern(pt),duration(du),particle_handle(ph),
	boundary(bd),initialized(false){
	}
	virtual ~TargetPatternAction(){}
	
	virtual void Update(ParticleContext_t& P ){

		static int frameNO = 0;
		if ( frameNO > duration ) {
			Sleep( display_duration );
			finished = true;
			frameNO = 0;
			return;
		}
		frameNO++;

		moveTowardPattern( pattern , duration-frameNO + 10, P );
		P.RandomAccel(PDSphere( pVec(0,0,0), disturbance ));
		
	}

	virtual void Resume(){
		finished = false;
		//initialized = false;
	}

private:
	int duration;
	int& particle_handle;
	ParticlePattern pattern;
	pDomain* boundary;
	bool initialized;

	void Initialize( ParticleContext_t& P ){

		int particle_count = pattern.particle_con.size();

		P.CurrentGroup( particle_handle);

		
		 int cnt = (int)P.GetGroupCount();
		int needcount = particle_count - cnt;
		 if (  needcount > 0 ) P.Source( needcount , PDPoint(pVec(0,0,0) ) );
		 else{
			 P.KillOld( 0 );
			 P.Source(particle_count,*boundary/*PDPoint(pVec(0,0,0) )*/  );
		 }
		
		moveTowardPattern( pattern , duration, P );
	}

	void moveTowardPattern( ParticlePattern& pic, float time, ParticleContext_t& P  ){
		int cnt = (int)P.GetGroupCount();
		if(cnt < 1) return;

		float *ptr;
		size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

		cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
			size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
			up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
		if(cnt < 1) return;//cnt indicates the number of particle in particle context

		int ptCnt = pic.particleCount;
		int step = 1;// if ptCnt match particle count then well
		double ratio = ( (double )ptCnt / (double)cnt );
		if ( 1 != ratio ){
			ratio > 1 ? step = ((int)ratio +1): step = (( int ) 1/ratio + 1 );
		}
		

		pVec* spos = (pVec*)(ptr + pos3Ofs);
		pVec   dpos;
		pVec* pvel	  = (pVec*)(ptr + vel3Ofs);
		pVec direction;
		pVec nVelo;

		int sampleIndicator = 0;
		int startPoint = 0;

		if ( cnt >= ptCnt ){//如果粒子的个数较多，则过采样，即在一个位置点上放置多个粒子
			for(int i = 0; i < cnt; i++){
				if  ( startPoint + step >= ptCnt ) {
					startPoint = 0;
					spos = (pVec*)(ptr + pos3Ofs);
					pvel	  = (pVec*)(ptr + vel3Ofs);
				}
				if ( sampleIndicator + step >= ptCnt ) {
					sampleIndicator = startPoint++;
					spos = (pVec*)(ptr + pos3Ofs);
					pvel	  = (pVec*)(ptr + vel3Ofs);
				}

				dpos	= pVec( pic.particle_con[sampleIndicator].x * 5, 
									 pic.particle_con[sampleIndicator].y * 5 , 
									 zdepth*pRandf()  );//Z-POS need to be revised


				direction	= dpos-*spos;
				float len		= direction.length()/time;
				pVec udirection =  direction/ direction.length();
				nVelo = udirection * len;
				pvel->x() = nVelo.x();
				pvel->y() = nVelo.y();
				pvel->z() = nVelo.z();
				spos	= (pVec*)( (float*) spos + int(flstride )*step);
				pvel	= (pVec*)( (float*) pvel  +  int(flstride)*step);
				sampleIndicator++;
			}
		}
		else{//如果点的个数较多，则欠采样
			for(int i = 0; i < cnt; i++){
				if ( sampleIndicator >= ptCnt ) sampleIndicator = startPoint++;

				dpos	= pVec( pic.particle_con[sampleIndicator].x * 5, 
					pic.particle_con[sampleIndicator].y * 5 , 
					zdepth*pRandf()  );//Z-POS need to be revised


				direction	= dpos-*spos;
				float len		= direction.length()/time;
				pVec udirection =  direction/ direction.length();
				nVelo = udirection * len;
				pvel->x() = nVelo.x();
				pvel->y() = nVelo.y();
				pvel->z() = nVelo.z();
				spos	= (pVec*)( (float*) spos + int(flstride ));
				pvel	= (pVec*)( (float*) pvel  +  int(flstride));
				sampleIndicator += step;
			}
		}



	}

};