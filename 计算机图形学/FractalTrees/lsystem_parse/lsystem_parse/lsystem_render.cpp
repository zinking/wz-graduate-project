#include "lsystem.h"
#include "lsystem_impl.h"


void LSystemRotationNode::render(LSystemRenderContext& c) const {

	switch ( this->axis )
	{
	case XAxis:
		glRotated( this->angleDegrees,1,0,0);
		break;
	case YAxis:
		glRotated( this->angleDegrees,0,1,0);
		break;
	case ZAxis:
		glRotated( this->angleDegrees,0,0,1);
		break;
	}
	//printf(" rotate ");
	
}

void LSystemScaleNode::render(LSystemRenderContext& c) const {
  // TODO: implement this
  // hint: don't call glScale -- just modify the current context
	//
	switch ( this->scaleType )
	{
	case ScaleLength:
		c.lengthScale *= this->scaleFraction;
		break;
	case ScaleRadius:
		c.radiusScale *= this->scaleFraction;
		break;
	case ScaleBoth:
		c.lengthScale *= this->scaleFraction;
		c.radiusScale *= this->scaleFraction;
		break;
	}

	//printf(" scale ");
	
}

void LSystemSegmentNode::render(LSystemRenderContext& c) const {
 
  // TODO: recursive application of rules.
  
  // watch out, could be null:
  const LSystemNode* ruleNode = c.lookup(id);
  if ( ruleNode != NULL && c.curDepth < c.maxDepth ){
	  //printf(" Begin recursive: ");
	  LSystemRenderContext nc = LSystemRenderContext(c);
	  nc.curDepth ++;
	  ruleNode->render( nc );
	  //printf(" End recursive ");

  }
  else{
	  glColor3ub(128, 101, 0);

	  gluQuadricTexture( c.quadric, true );

	  gluCylinder(c.quadric,
		  c.radiusScale,
		  c.radiusScale * endScaleFraction,
		  c.lengthScale,
		  10, 1);
	
	  glTranslated(0, 0, c.lengthScale);
	  //printf(" trunk ");
  }
  

  
  

}

extern glShader* shader;
void LSystemLeafNode::render(LSystemRenderContext& context) const {
    // TODO
	//printf(" leaf ");
	shader->end();
			
	glEnable(GL_BLEND);
	glEnable( GL_TEXTURE_2D );
	//glBindTexture( GL_TEXTURE_2D, 3 );
	glColor4f(0.1,0.9,0.1,0.8);
	
	double scale =  0.1;
	glPushMatrix();
	glScaled( scale, scale, scale );

	
	glBegin(GL_POLYGON);
	
	glVertex2f(0.0,0.0);	glTexCoord2f(0.0,0.0);	
	glVertex2f(1.0,0.7);	glTexCoord2f(1.0,0.7);
	glVertex2f(1.3,1.8);	glTexCoord2f(1.3,1.8);
	glVertex2f(1.0,2.8);	glTexCoord2f(1.0,2.8);
	glVertex2f(0.0,4.0);	glTexCoord2f(0.0,4.0);
	glVertex2f(-1.0,2.8);	glTexCoord2f(-1.0,2.8);
	glVertex2f(-1.3,1.8);	glTexCoord2f(-1.3,1.8);
	glVertex2f(-1.0,0.7);	glTexCoord2f(-1.0,0.7);
	glEnd();
	glPopMatrix();
	//glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	shader->begin();

}


// don't have to do anything here, just included for clarity
void LSystemGroupNode::render(LSystemRenderContext& c) const {
  
  double lengthScale = c.lengthScale;
  double radiusScale = c.radiusScale;
  
  if (restoreState) {
    glPushMatrix();
	//printf("[ ");
  }

  for (unsigned int i=0; i<children.size(); ++i) {
    children[i]->render(c);
  }

  if (restoreState) {
    glPopMatrix();
	//printf("]");
    c.lengthScale = lengthScale;//restore original parameters
    c.radiusScale = radiusScale;//restore original parameters
  }

}

void LSystem::generatelist() const{

	glNewList(display_index, GL_COMPILE);

	if (!_quadric) {
		_quadric = gluNewQuadric();
	}

	LSystemRenderContext c(_rules, _quadric, _maxDepth);

	glPushMatrix();
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glRotated(90.0, 0.0, 0.0, 1.0);
	glTranslatef(0,0,-1);
	glScaled(0.5,0.5,0.5);

	shader->begin();


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, barktexture);
	shader->setUniform1i( "textureMap1", 0 );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normaltexture);
	shader->setUniform1i( "normalMap1", 1 );

	_root->render(c);
	shader->end();

	
	glPopMatrix();

	glEndList();


}


void LSystem::renderGL() const {
	glBindTexture( GL_TEXTURE_2D, 1 );
	/*shader->begin();
	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, barktexture);
	shader->setUniform1i( "textureMap1", 0 );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normaltexture);
	shader->setUniform1i( "normalMap1", 1 );*/

	glCallList(display_index);
	/*shader->end();*/

}
