#include <windows.h>
#include <GL/gl.h>
#include "Particle.h"
#include "trackball.h"
#include "Parameter.h"

extern CTrackBall	*ptrCTrackBall;
pVec view_(0,0,1);
pVec up_(0,1,0);

FishParticle::FishParticle(void) : boundary(pVec(0,0,0), pVec(5,5,5)){
	pVec center = pVec( 2.5, 2.5, 2.5 );
	action_handle = P.GenActionLists(1);
	P.CurrentGroup( fishs_handle);
}

FishParticle::FishParticle( pVec dimension ) : boundary(pVec(0,0,0), dimension){
	pVec center = pVec( dimension.x()/2 , dimension.y()/2 , dimension.z()/2 );
}
void FishParticle::Initialize(  ){
	if ( fishs_handle != 0  ) P.DeleteParticleGroups( fishs_handle, 1 );
	
	fishs_handle   = P.GenParticleGroups(1,7000); 
	P.CurrentGroup( fishs_handle);
	P.Size(0.5);
	P.Source( 1000,boundary);
	particle_count = 1000;


	bool isBigfish;
	for ( int i=0; i < particle_count ; i ++ ){
		FishTextures.push_back( getFishTexture( isBigfish ) ) ;
		FishSizes.push_back( isBigfish );
	}
	
	ParticlePattern xx(".\\pic\\test.jpg"), ni(".\\pic\\ni.jpg"),arrow(".\\pic\\arrow.jpg"),star(".\\pic\\star.jpg");
	MoveAction*				move		= new MoveAction( pVec(4,0,0)*0.019, move_frames );
	//SwingAction*				s_move		= new SwingAction( pVec(4,0,0)*0.005, move_frames );
	//SnakeAction*				sn_move	= new SnakeAction( pVec(4,0,0)*0.0019,move_frames);
	TargetPatternAction*  taxx			= new TargetPatternAction( pattern_frames , fishs_handle, xx ,		&boundary);
	TargetPatternAction* 	tani			= new TargetPatternAction(  pattern_frames, fishs_handle, ni ,		&boundary);
	TargetPatternAction* 	taarrow		= new TargetPatternAction(  pattern_frames, fishs_handle, arrow ,	&boundary);
	TargetPatternAction* 	tastar		= new TargetPatternAction(  pattern_frames, fishs_handle, star ,		&boundary);
	
	paCon.push_back( taxx );
	paCon.push_back( move );
	paCon.push_back( tani );
	paCon.push_back( move );
	paCon.push_back( taarrow );
	paCon.push_back( move );
	paCon.push_back( tastar );
	paCon.push_back( move );

	index = 0;

}

void FishParticle::Add( ParticlePattern& p){

	TargetPatternAction* 	tag = new TargetPatternAction(  pattern_frames, fishs_handle, p ,&boundary);
	paCon.insert( paCon.begin() + index+1, tag );
}

void FishParticle::AddTrace( ParticlePattern& p){

	SnakeAction* 	tag = new SnakeAction( move_frames,p );
	paCon.insert( paCon.begin() + index + 1, tag );
}

void FishParticle::update(){
	ParticleAction* pa;
	pa = this->paCon[index];

	if( !pa->finished ) pa->Update( P );
	else{
		pa->Resume();
		index++;
	}

	if( index >= paCon.size() )  index = 0;

	P.CurrentGroup( fishs_handle );
	P.Move(true, false);

}

void FishParticle::Draw( bool pause ){
	if ( !pause ) update();
	RenderFishs(P, view_ , up_ , 0.2, true, false, true);
}

void FishParticle::RenderFishs(ParticleContext_t &P, const pVec &view, const pVec &up,
							float size_scale , bool draw_tex,
							bool const_size, bool const_color){
	int cnt = (int)P.GetGroupCount();
	if(cnt < 1)	return;

	pVec *ppos = new pVec[cnt];
	float *color = const_color ? NULL : new float[cnt * 4];
	pVec *size = const_size ? NULL : new pVec[cnt];
	P.GetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);

	pVec right = Cross(view, up);
	right.normalize();
	pVec nup = Cross(right, view);
	right *= size_scale;
	nup *= size_scale;

	pVec V0 = -(right + nup);
	pVec V1 = right - nup;
	pVec V2 = right + nup;
	pVec V3 = nup - right;


	glPushMatrix();
	GLfloat m[4][4];
	ptrCTrackBall->create_rotation_matrix( m );
	glMultMatrixf( &m[0][0] );

	glEnable (GL_BLEND);
	glDisable( GL_DEPTH_TEST );

	bool isBigfish = false;
	GLuint texture = 0;
	int bigfish_size = 1;


	for(int i = 0; i < cnt; i++){
		texture = FishTextures[i];
		bigfish_size = FishSizes[i] ? 3:1;
		glBindTexture( GL_TEXTURE_2D, texture );
		glPushMatrix();
		//glRotatef(disturbance_angle*pRandf,0,0,1);

		glBegin(GL_QUADS);

		pVec &p = ppos[i];
		if(!const_color)        glColor4fv((GLfloat *)&color[i*4]);

		pVec sV0 = V0;
		pVec sV1 = V1;
		pVec sV2 = V2;
		pVec sV3 = V3;

		if(!const_size)
		{
			sV0 *= size[i].x() * bigfish_size;
			sV1 *= size[i].x() * bigfish_size;
			sV2 *= size[i].x() * bigfish_size;
			sV3 *= size[i].x() * bigfish_size;
		}

		if(draw_tex) glTexCoord2f(0,0);
		pVec ver = p + sV2;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(1,0);
		ver = p + sV3;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(1,1);
		ver = p + sV0;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(0,1);
		ver = p + sV1;
		glVertex3fv((GLfloat *)&ver);
		glEnd();
		glPopMatrix();

	}


	glPopMatrix();

	glEnable( GL_DEPTH_TEST );


	delete [] ppos;
	if(color) delete [] color;
	if(size) delete [] size;
}

FishParticle::~FishParticle(void){}


