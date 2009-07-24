#pragma once

#include <pAPI.h>
#include "ParticlePattern.h"
#include "particleaction.h"






typedef vector<ParticleAction*> PAContainer;
typedef PAContainer::iterator PACItr;

using namespace PAPI;

class FishParticle{
public:
	FishParticle(void);
	FishParticle( pVec dimension );
	~FishParticle(void);

	void Initialize(  );
	void Draw( bool pause = false );
	void Add( ParticlePattern& p);
	void AddTrace( ParticlePattern& p );

private:
	void update( );
	void RenderFishs(ParticleContext_t &P, const pVec &view, const pVec &up,
		float size_scale = 1.0f, bool draw_tex=false,
		bool const_size=false, bool const_color=false);

	GLuint getFishTexture( bool& isbigfish ){
		int poll = pRandf() * 100;
		poll %= 100; poll++;
		GLuint texture = 0;

		if( poll > 0 &&  poll <= 2 ){// This is a big fish
			texture = (int)( pRandf()* 10 ) % 4 +21;
			isbigfish = true;
		}
		else{
			texture = (int)( pRandf()* 10 ) % 6 +11;
			isbigfish = false;
		}
		return texture;
	}


private:
	PAContainer paCon;
	vector<GLuint> FishTextures;
	vector<bool> FishSizes;
	int index;	

	int	fishs_handle;	// The handle of the particle group
	int	action_handle; 
	int particle_count;

	pVec view;
	pVec up;

	ParticleContext_t P;
	PDBox boundary;
	
};
