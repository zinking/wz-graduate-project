#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <GL/glut.h>

#include "pVec.h"
using namespace PAPI;

int displaytype;

void CheckGLError(){
	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		printf ( "OpenGL Error: %s\n", errString);
	}
}

struct vector2D 
{
	float x;
	float y;
	vector2D(){}
	vector2D(int x,int y):x(float(x)),y(float(y)){}
};

struct vector3D 
{
	float x;
	float y;
	float z;
};

struct vector5D {
	float x;
	float y;
	float z;
	float p; 
	float q; 
	vector5D( float xx, float yy, float zz):x(xx),
		y(yy),z(zz){}
	vector5D(){}

	pVec topVec( ){
		return pVec( x,y,z );

	}
	void setVec( pVec v){
		x = v.x();
		y = v.y();
		z = v.z();
	}


	inline vector5D operator+(const vector5D& a) const
	{
		return vector5D(x+a.x, y+a.y, z+a.z);
	}

	inline vector5D operator*(const float s) const
	{
		return vector5D(x*s, y*s, z*s);
	}
	inline vector5D& operator=(const vector5D& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}
};

struct index2D 
{
	int k;
	int l;
};

struct normalVectorAndZvalue 
{
	index2D basicpoint; 
	vector3D n; 
	unsigned int visible; 
	float Z; 
	float height; 
};

struct color
{
	float r;
	float g;
	float b;
};

void processMemoryOver(void)
{
	printf("memory over\n");
	exit(0);
}

const float PI=3.141592853f;


class BezierPatch{
public:
	BezierPatch():offsetX(0),offsetY(0){}
	~BezierPatch(){}
	virtual void DrawSurface(){}
	virtual int  checkSurfaceSelected( vector2D input){ return -1; }
	virtual void UpdateBezierSurface(){}
	virtual void updateSelectedControlPointPosition( float x, float y){}
	virtual int  getTheNearestCtrlPointIndex( vector2D input){ return -1; }
	virtual void InitializeCtrlPointOffset( float dx, float dy){}
	virtual void ReInitialize(){}
	void ToggleWireframe( int flag ){
		displaytype = flag;
	}

	void InitLightParameter(){
		mat_shininess[0] = 30;
		ranParameter( mat_fspecular );
		ranParameter( mat_ambient );
		ranParameter( mat_diffuse );
		ranParameter( mat_bspecular );
		ranParameter( wire_color );
		sourceShapeFlag = (int)(PAPI::pRandf()* 10) % 3 ; 
	}
	void setShowCtrlPoints( int flag ){ showCtrlPointsFlag = flag; }
	void setSourceShapeFlag( int flag ){ sourceShapeFlag = flag ;}
private:
	inline void ranParameter( float* p ){
		p[0] = PAPI::pRandf();
		p[1] = PAPI::pRandf();
		p[2] = PAPI::pRandf();
		p[3] = 1.0f;
	}


public:
	float offsetX;
	float offsetY;
protected:
	float original_offsetX;
	float original_offsetY;
	//lighting settings
	GLfloat wire_color[4]; 
	GLfloat mat_fspecular[4];
	GLfloat mat_ambient[4];	
	GLfloat mat_diffuse[4];  
	GLfloat mat_bspecular[4]; 

	GLfloat mat_shininess[1];
	int showCtrlPointsFlag; 
	int sourceShapeFlag; 

	//int displaytype;
	
	int editpointSeletedFlag;
	int ctrlPointSelectedFlag; 
};

enum edgetype{
	LEFT = 0,
	RIGHT,
	TOP,
	BOTTOM
};



class TriangularBezierPatch;
class BezierSurfacePatch : public BezierPatch{
	friend class TriangularBezierPatch;

public:
	float size; 
	int n; //for two dimension array it corresponds to the row direction
	int m; 
	int usubdivi; 
	int vsubdivi; 
	vector5D **ctrlPoint; 
	vector5D **bezierPoint; 
	normalVectorAndZvalue *bezierSurface; 
	int fourcorner[5][2];

	index2D selectedCtrlPointIndex;
	BezierSurfacePatch* upgraded_mesh;

	BezierSurfacePatch(  int u = 3, int v= 3  ):m(u),n(v){
		size=1.0f; 
		usubdivi=25; 
		vsubdivi=25; 
		ctrlPointSelectedFlag=0; 
		editpointSeletedFlag = 0;
		showCtrlPointsFlag=0; 
		upgraded_mesh = NULL;
		InitLightParameter();
		makeCtrlPointsArray();
		processInitCtrlPoints();
		makeBezierPointsArray();
		getBezierPointsCoordinate();
		makeBezierSurfacesArray();
		getBezierSurfacesNormalVector();
	}
	
	virtual ~BezierSurfacePatch(){
		free( ctrlPoint );
		free(bezierPoint );
		free(bezierSurface);
		if ( upgraded_mesh != NULL ) delete upgraded_mesh;
	}

	virtual void UpdateBezierSurface(){
		//if( upgraded_mesh != NULL ) {
		//	//this->UpgradeBezierPatch();
		//	//return;
		//}
		getBezierPointsCoordinate();

	}

	virtual void DrawSurface(){
		
		glPushMatrix();
		glTranslatef( offsetX, offsetY, 0 );
		updateControlPointProjectionPosition();
		if ( upgraded_mesh != NULL){
			upgraded_mesh->drawBezierSurfaces();
		}
		else drawBezierSurfaces();
		drawCtrlPoints();
		glPopMatrix();
	}

	virtual int checkSurfaceSelected( vector2D input){

		int vertex_index[4] = {0,3,9,0};
		float product = 1;
		//check if the point is within the triangle
		showCtrlPointsFlag = 1;
		for( int i=0; i < 4;i++){
			pVec vside(	ctrlPoint[fourcorner[i+1][0]][fourcorner[i+1][1]].p - ctrlPoint[fourcorner[i][0]][fourcorner[i][1]].p,
						ctrlPoint[fourcorner[i+1][0]][fourcorner[i+1][1]].q - ctrlPoint[fourcorner[i][0]][fourcorner[i][1]].q, 1 ); 
			pVec v( input.x - ctrlPoint[fourcorner[i][0]][fourcorner[i][1]].p,
					input.y - ctrlPoint[fourcorner[i][0]][fourcorner[i][1]].q, 1);
			pVec result = Cross( v,vside);
			if ( result.z() > 0 ) {
				showCtrlPointsFlag = 0;
				break;
			}
		}


		return showCtrlPointsFlag;
	}

	void UpgradeBezierPatch(){
		if( upgraded_mesh != NULL ) delete upgraded_mesh;
		upgraded_mesh = new BezierSurfacePatch(  this->m+1, this->n+1  );
		
		upgraded_mesh->setSourceShapeFlag( this->sourceShapeFlag );
		//first upgrade colomun;
		for( int i=0; i <= n; i++ ){//iterate through original n+1 rows
			upgraded_mesh->ctrlPoint[i][0] = ctrlPoint[i][0];
			upgraded_mesh->ctrlPoint[i][m+1] = ctrlPoint[i][m];
		}

		for( int j=1; j <= m; j++ ){//iterate through original m-1 coloms
			float miu = (float)j/(float)(m+1);
			for ( int i=0; i <= n; i++ ){// iterate throught original n+1 rows
				upgraded_mesh->ctrlPoint[i][j] = ctrlPoint[i][j]*(1-miu)+ctrlPoint[i][j-1]*(miu);
			}
		}

		for ( int j=0; j <= m+1; j++ ){//iterate through m+1 +1 colomuns
			upgraded_mesh->ctrlPoint[n+1][j] = upgraded_mesh->ctrlPoint[n][j];
		}

		


		// it's never an easy task to upgrade the mesh inplace;
		vector5D* old = new vector5D[m+2];
		for ( int mj=0; mj <= m+1; mj++) old[mj] = upgraded_mesh->ctrlPoint[0][mj];

		for ( int j=0; j <= m+1; j++){// 
			for( int i = 1; i <= n; i++ ){//
				float miu = (float)i/(float)(n+1);
				vector5D oldvalue = upgraded_mesh->ctrlPoint[i][j];
				upgraded_mesh->ctrlPoint[i][j] =upgraded_mesh->ctrlPoint[i][j]*(1-miu)+old[j]*(miu);
				old[j] = oldvalue;
			}
		}
		delete[] old;

		upgraded_mesh->getBezierPointsCoordinate();
		//upgraded_mesh->showCtrlPointsFlag = 1;
	}

	// this function call makes patch lhs(this) and rhs acheive C1 continuity
	// so make sure they have reached C0 continuity
	void ConnectWithAnotherPatchWithSameDegree(BezierSurfacePatch* rhs, edgetype ltype, edgetype rtype){


		if( this->upgraded_mesh != NULL ) return upgraded_mesh->ConnectWithAnotherPatchWithSameDegree( rhs,ltype,rtype);
		if( rhs->upgraded_mesh != NULL ) return this->ConnectWithAnotherPatchWithSameDegree( rhs->upgraded_mesh,ltype, rtype );

		if ( rhs->m - this->m >1 || rhs->n -this->n > 1 ){
			printf( "incorrect connection patch degree\n");
			return;
		}
		else if ( rhs->m -this->m == 1 && rhs->n - this->n == 1){
			this->UpgradeBezierPatch();
			return this->upgraded_mesh->ConnectWithAnotherPatchWithSameDegree( rhs, ltype, rtype );
		}
		else if ( rhs->m -this->m == -1 && rhs->n - this->n == -1){
			rhs->UpgradeBezierPatch();
			return this->ConnectWithAnotherPatchWithSameDegree( rhs->upgraded_mesh, ltype, rtype );
		}

		if ( ltype == LEFT && rtype == RIGHT ){
			for ( int j=0; j < m+1; j++){
				vector5D mid = (this->ctrlPoint[n-1][j]+rhs->ctrlPoint[1][j])*0.5;
				//vector5D cmid = (this->ctrlPoint[n][j]+rhs->ctrlPoint[0][j])*0.5;//ch
				this->ctrlPoint[n][j] = mid;
				rhs->ctrlPoint[0][j] = mid;
			} 
		} 
		else if ( ltype == TOP && rtype == BOTTOM ){
			for( int i=0; i < n+1; i++ ){
				vector5D mid = (this->ctrlPoint[i][m-1]+rhs->ctrlPoint[i][1])*0.5;
				this->ctrlPoint[i][m] = mid;
				rhs->ctrlPoint[i][0] = mid;
			}
		}
		else{
			printf("InCorrect connection Type\n");
		}
		this->getBezierPointsCoordinate();
		rhs->getBezierPointsCoordinate();

	}

	void makeCtrlPointsArray(void)
	{
		int i;
		fourcorner[0][0] = 0;fourcorner[0][1] = 0;
		fourcorner[1][0] = 0;fourcorner[1][1] = m;
		fourcorner[2][0] = n;fourcorner[2][1] = m;
		fourcorner[3][0] = n;fourcorner[3][1] = 0;
		fourcorner[4][0] = 0;fourcorner[4][1] = 0;


		if((ctrlPoint=(struct vector5D **)malloc((n+1)*sizeof(struct vector5D *)))==NULL) processMemoryOver();
		for(i=0;i<n+1;i++)
		{
			if((ctrlPoint[i]=(struct vector5D *)malloc((m+1)*sizeof(struct vector5D)))==NULL) processMemoryOver();
		}
	}


	void makeBezierPointsArray(void)
	{
		int k;

		if((bezierPoint=(struct vector5D **)malloc(usubdivi*sizeof(struct vector5D *)))==NULL) processMemoryOver();
		for(k=0;k<usubdivi;k++)
		{
			if((bezierPoint[k]=(struct vector5D *)malloc(vsubdivi*sizeof(struct vector5D)))==NULL) processMemoryOver();
		}

		makeBezierSurfacesArray();
	}

	void makeBezierSurfacesArray(void)
	{
		if((bezierSurface=(struct normalVectorAndZvalue *)malloc((usubdivi-1)*(vsubdivi-1)*sizeof(struct normalVectorAndZvalue)))==NULL) 
			processMemoryOver();
	}

	virtual void updateSelectedControlPointPosition( float x, float y){
		if ( ctrlPointSelectedFlag ){
			//this->updateControlPointProjectionPosition();
			int i=selectedCtrlPointIndex.k;
			int j=selectedCtrlPointIndex.l;
			

			GLdouble		modelview[16];
			GLdouble		projection[16];
			GLint			viewport[4];
			glGetDoublev( GL_MODELVIEW_MATRIX,	modelview);
			glGetDoublev( GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewport);
			double targetx,targety,targetz;
			gluProject( ctrlPoint[i][j].x , ctrlPoint[i][j].y , ctrlPoint[i][j].z ,
						modelview,projection,viewport,
						&targetx,&targety,&targetz);

			float winX = x;   
			float winY = viewport[3] - y; 
			float winZ = targetz;

			gluUnProject(winX,winY,winZ,modelview,projection,viewport,&targetx,&targety,&targetz); 
			ctrlPoint[i][j].x =  targetx ;
			ctrlPoint[i][j].y =  targety;
			ctrlPoint[i][j].z = targetz;
			
		}
	}


	virtual void updateControlPointProjectionPosition(){
		int i,j;
		GLdouble		modelview[16];
		GLdouble		projection[16];
		GLint			viewport[4];
		glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev (GL_PROJECTION_MATRIX, projection);
		glGetIntegerv (GL_VIEWPORT, viewport); 

		for(i=0;i<n+1;i++){
			for(j=0;j<m+1;j++){

				double c1, c2, c3;
				int result = gluProject(	ctrlPoint[i][j].x , ctrlPoint[i][j].y , ctrlPoint[i][j].z ,	
					modelview, projection , viewport, &c1, &c2, &c3	) ;
				if ( result ){
					ctrlPoint[i][j].p = c1;
					ctrlPoint[i][j].q = c2;
				}
				else{
					printf("there must be sth wrong with the matrix\n");
				}
			}
		}
	}


	void processInitCtrlPoints(void)
	{
		int i,j;
		float k,l; 

		for(i=0;i<n+1;i++){
			for(j=0;j<m+1;j++){
				k=(float)i/(float)n-0.5f;
				l=(float)j/(float)m-0.5f;
				ctrlPoint[i][j].x=size*k; 
				ctrlPoint[i][j].y=size*l;
				if(sourceShapeFlag==0)ctrlPoint[i][j].z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)ctrlPoint[i][j].z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2)ctrlPoint[i][j].z=0.0f;

			}
		}
	}

	virtual void InitializeCtrlPointOffset( float dx, float dy){
		int i,j;
		float k,l; 
		this->original_offsetX = dx;
		this->original_offsetY = dy;

		for(i=0;i<n+1;i++){
			for(j=0;j<m+1;j++){
				k=(float)i/(float)n-0.5f;
				l=(float)j/(float)m-0.5f;
				ctrlPoint[i][j].x=size*k+dx; 
				ctrlPoint[i][j].y=size*l+dy;
				if(sourceShapeFlag==0)ctrlPoint[i][j].z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)ctrlPoint[i][j].z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2)ctrlPoint[i][j].z=0.0f;

			}
		}
		this->getBezierPointsCoordinate();
	}
	
	virtual void ReInitialize( ){
		int i,j;
		float k,l; 


		for(i=0;i<n+1;i++){
			for(j=0;j<m+1;j++){
				k=(float)i/(float)n-0.5f;
				l=(float)j/(float)m-0.5f;
				ctrlPoint[i][j].x=size*k+original_offsetX; 
				ctrlPoint[i][j].y=size*l+original_offsetY;
				if(sourceShapeFlag==0)ctrlPoint[i][j].z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)ctrlPoint[i][j].z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2)ctrlPoint[i][j].z=0.0f;

			}
		}
		this->getBezierPointsCoordinate();

	}


	void drawCtrlPoints(void)
	{
		if(showCtrlPointsFlag)
		{
			int i,j;
			glDisable( GL_LIGHTING );

			glColor3f( 0.0f, 0.0f, 0.0f);

			glPointSize(5.0f);
			glBegin(GL_POINTS);
			for(i=0;i<n+1;i++){
				for(j=0;j<m+1;j++){
					glVertex3f(ctrlPoint[i][j].x, ctrlPoint[i][j].y,ctrlPoint[i][j].z);
				}
			}
			glEnd();

			if(ctrlPointSelectedFlag)
			{
				i=selectedCtrlPointIndex.k; j=selectedCtrlPointIndex.l;
				glPointSize( 10.0f);
				glColor3f(0.0f,1.0f,0.2f);
				glBegin(GL_POINTS);
				glVertex3f(ctrlPoint[i][j].x,ctrlPoint[i][j].y, ctrlPoint[i][j].z);
				glEnd();
				glColor3f(0.0f,0.0f,0.0f);
			}

			for(i=0;i<n+1;i++)
			{
				glBegin(GL_LINE_STRIP);
				for(j=0;j<m+1;j++)
				{
					glVertex3f(ctrlPoint[i][j].x,ctrlPoint[i][j].y, ctrlPoint[i][j].z);
				}
				glEnd();
			}
			for(j=0;j<(m+1);j++)
			{
				glBegin(GL_LINE_STRIP);
				for(i=0;i<n+1;i++)
				{
					glVertex3f(ctrlPoint[i][j].x,ctrlPoint[i][j].y, ctrlPoint[i][j].z);
				}
				glEnd();
			}
			glEnable( GL_LIGHTING );
		}
	}




	void drawBezierSurfaces(void)
	{
		int k,l,f;
		
		

		if (  displaytype == 0  ){
			for(f=0;f<(usubdivi-1)*(vsubdivi-1);f++)
			{
				k=bezierSurface[f].basicpoint.k; l=bezierSurface[f].basicpoint.l;
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_fspecular);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
				glMaterialfv(GL_BACK, GL_SPECULAR, mat_fspecular);

				glBegin(GL_QUADS);
				glNormal3fv((float*) &(bezierSurface[f].n) );
				glVertex3f(bezierPoint[k][l].x,			bezierPoint[k][l].y ,		bezierPoint[k][l].z);
				glVertex3f(bezierPoint[k+1][l].x,		bezierPoint[k+1][l].y,		bezierPoint[k+1][l].z	);
				glVertex3f(bezierPoint[k+1][l+1].x,		bezierPoint[k+1][l+1].y,	bezierPoint[k+1][l+1].z);
				glVertex3f(bezierPoint[k][l+1].x,		bezierPoint[k][l+1].y,		bezierPoint[k][l+1].z);
				glEnd();
			}
		}
		else if( displaytype == 1)
		{
			glDisable(GL_LIGHTING);
			for(f=0;f<(usubdivi-1)*(vsubdivi-1);f++)
			{
				k=bezierSurface[f].basicpoint.k; l=bezierSurface[f].basicpoint.l;
				
				glColor3fv(wire_color);
				glBegin(GL_LINE_LOOP);
				glVertex3f(bezierPoint[k][l].x,		bezierPoint[k][l].y ,	bezierPoint[k][l].z);
				glVertex3f(bezierPoint[k+1][l].x,	bezierPoint[k+1][l].y,	bezierPoint[k+1][l].z	);
				glVertex3f(bezierPoint[k+1][l+1].x,	bezierPoint[k+1][l+1].y,bezierPoint[k+1][l+1].z);
				glVertex3f(bezierPoint[k][l+1].x,	bezierPoint[k][l+1].y,	bezierPoint[k][l+1].z);
				glEnd();
			}
			glEnable(GL_LIGHTING);
		}
	}




	void getBezierPointsCoordinate(void)
	{
		int i,j,k,l;
		float u,v; 
		float *bu,*bv; 

		//these two are allocated for weight values
		if((bu=(float *)malloc((n+2)*sizeof(float)))==NULL) processMemoryOver();
		if((bv=(float *)malloc((m+2)*sizeof(float)))==NULL) processMemoryOver();

		for(k=0;k<usubdivi;k++)
		{
			u=(float)k/(float)(usubdivi-1);

			for(i=0;i<(n+2);i++) bu[i]=0.0f;

			bu[1]=1.0f;
			for(i=1;i<n+1;i++)
			{
				for(j=i+1;j>0;j--)
				{
					bu[j]=(1-u)*bu[j]+u*bu[j-1];
				}
			}
			for(l=0;l<vsubdivi;l++)
			{
				v=(float)l/(float)(vsubdivi-1);

				for(j=0;j<(m+2);j++) bv[j]=0.0f;

				bv[1]=1.0f;
				for(j=1;j<m+1;j++)
				{
					for(i=j+1;i>0;i--)
					{
						bv[i]=(1-v)*bv[i]+v*bv[i-1];
					}
				}

				bezierPoint[k][l].x=0.0f; bezierPoint[k][l].y=0.0f; bezierPoint[k][l].z=0.0f;
				for(i=0;i<n+1;i++)
				{
					for(j=0;j<m+1;j++)
					{
						bezierPoint[k][l].x+=ctrlPoint[i][j].x*bu[i+1]*bv[j+1];
						bezierPoint[k][l].y+=ctrlPoint[i][j].y*bu[i+1]*bv[j+1];
						bezierPoint[k][l].z+=ctrlPoint[i][j].z*bu[i+1]*bv[j+1];
					}
				}
			}
		}

		free(bu); free(bv);
		getBezierSurfacesNormalVector();
	}





	void getBezierSurfacesNormalVector(void)
	{
		int k,l,f;
		

		f=0;
		for(k=0;k<usubdivi-1;k++)
		{
			for(l=0;l<vsubdivi-1;l++)
			{
				bezierSurface[f].basicpoint.k=k; bezierSurface[f].basicpoint.l=l;
				pVec v1 = pVec( bezierPoint[k+1][l].x -bezierPoint[k][l].x ,
								bezierPoint[k+1][l].y -bezierPoint[k][l].y ,
								bezierPoint[k+1][l].z -bezierPoint[k][l].z );
				pVec v2 = pVec( bezierPoint[k][l+1].x -bezierPoint[k][l].x ,
								bezierPoint[k][l+1].y -bezierPoint[k][l].y ,
								bezierPoint[k][l+1].z -bezierPoint[k][l].z );
				pVec result = Cross(v1, v2);
				result.normalize();
				bezierSurface[f].n.x = result.x();
				bezierSurface[f].n.y = result.y();
				bezierSurface[f].n.z = result.z();
								
				f++;
			}
		}
	}




void debuginfo(){

		if ( ctrlPointSelectedFlag ){
			int k = selectedCtrlPointIndex.k;
			int l = selectedCtrlPointIndex.l;
			printf( " Selected index: %d %d ( %f,  %f) \n",  k, l , ctrlPoint[k][l].p , ctrlPoint[k][l].q  );
		}
		
		int i,j;
		for(i=0;i<n+1;i++){
			for(j=0;j<m+1;j++){
					printf( " ( %3.0f,  %3.0f ) " , ctrlPoint[i][j].p , ctrlPoint[i][j].q  );
			}
			printf("\n");
		}
}


int getTheNearestCtrlPointIndex( vector2D input)
{
		
		int i,j;
		const float threshold= 6.5f; 
		float diffx,diffy; 
		float diff; 
		float mindist; 
		index2D output; 


		mindist=threshold; 
		output.k=-1; 
		output.l=-1;

		ctrlPointSelectedFlag = 0;

		for(i=0;i<n+1;i++)
		{
			for(j=0;j<m+1;j++)
			{

				diffx=input.x-ctrlPoint[i][j].p; diffy=input.y-ctrlPoint[i][j].q;
				diff=diffx*diffx+diffy*diffy;
				if(diff<=mindist)
				{
					mindist=diff;
					output.k=i; output.l=j;
					ctrlPointSelectedFlag = 1;
					printf("Contol Point (%d,%d) seleted\n", i,j);
					break;
				}

			}
		}


		selectedCtrlPointIndex.k = output.k;
		selectedCtrlPointIndex.l  = output.l;

		//debuginfo();
		return ctrlPointSelectedFlag;
}



	void getTriangleAt( int x, int y,  pVec& p1, pVec& p2, pVec& p3){
		p1 =  ctrlPoint[x][y].topVec();
		p2 =  ctrlPoint[x+1][y].topVec();
		p3 = ctrlPoint[x][y-1].topVec();
	}
};//END OF CLASS



class TriangularBezierPatch :public BezierPatch{
private:
		float size;
		int subdiv;
		int degree;
		pVec*		_CtrlPoints;
		vector2D*	_Ctrl2DPoints;
		int ctrlPointCount;
		int*		_ctrlPointIndex;//used to draw ctrl mesh
		int ctrlPointIndexCount;
		int selectedCtrlPointIndex;
		pVec*		_InternalPoints;
		int internalPointCount;
		int*		_internalPointIndex;//used to draw triangle mesh
		int internalPointIndexCount;
		pVec*		_normals;
		int normalCount;


public:
	TriangularBezierPatch(){
		size=1.0f; 
		degree = 4;

		ctrlPointSelectedFlag=0; 
		showCtrlPointsFlag=0; 


		selectedCtrlPointIndex = -1;

		InitLightParameter();
		processInitCtrlPoints();		
		getBezierPointsCoordinate();

	}
	virtual ~TriangularBezierPatch(){
		delete[] _CtrlPoints;
		delete[] _Ctrl2DPoints;
		delete[] _ctrlPointIndex;
		delete[] _normals;
		delete[] _InternalPoints;
		delete[] _internalPointIndex;

	}

	void generateIndex( int div, int len, int* patchIndex){
		//    3
		//   2 6
		//  1 5 8
		// 0 4 7 9
		int rowlen = div;
		int skip_position = div-1;
		int numofindex = 0;
		for ( int i=0; numofindex < len;){
			if( i == skip_position ){
				skip_position += rowlen-1;
				rowlen--;
				i++;
				continue;
				
			}

			patchIndex[ numofindex++ ]=i;
			patchIndex[ numofindex++ ]=i+1;
			patchIndex[ numofindex++ ]=i+rowlen;
			i++;
		}
	}
	void processInitCtrlPoints( )
	{
		ctrlPointCount = (degree+1 )*( degree  )/2;
		_CtrlPoints = new pVec[ctrlPointCount];


		int i,j,index = 0;
		float x,y,z,k,l;
		for( i=0;i< degree;i++){
			for( j=0;j< degree - i;j++){
				k=(float)i/(float)degree-0.5f;
				l=(float)j/(float)degree-0.5f;
				x=size*k; 
				y=size*l;
				if(sourceShapeFlag==0)		z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)	z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2) z=0.0f;
				_CtrlPoints[index] = pVec( x,y,z);
				index ++;

			}
		}

		

		 ctrlPointIndexCount = ( degree -1  ) * degree /2 * 3;
		_ctrlPointIndex = new int[ctrlPointIndexCount];
		_Ctrl2DPoints	= new vector2D[ ctrlPointIndexCount ];

		// The control points are ordered like this:
		//
		//    3
		//   2 6
		//  1 5 8
		// 0 4 7 9

		generateIndex( degree, ctrlPointIndexCount, _ctrlPointIndex );

		subdiv = 17;
		internalPointCount = (subdiv-1)*(subdiv-1);
		_InternalPoints = new pVec[internalPointCount];
		internalPointIndexCount = ((subdiv-1)*(subdiv-1)+1)*3;
		_internalPointIndex = new int[internalPointIndexCount];

		normalCount = ((subdiv-1)*(subdiv-1)+1);
		_normals = new pVec[normalCount];


		int Index = 0;
		int i0 = 0, i1 = 0 ;
		for( int n = 0; n < subdiv; n++ )
		{
			i0 = i1;
			i1 += subdiv-n;

			for( i = 0; i < subdiv-2-n; i++ )
			{
				_internalPointIndex[Index++] = i0+i;
				_internalPointIndex[Index++] = i0+i+1;
				_internalPointIndex[Index++] = i1+i;

				_internalPointIndex[Index++] = i0+i+1;
				_internalPointIndex[Index++] = i1+i+1;
				_internalPointIndex[Index++] = i1+i;
			}

			_internalPointIndex[Index++] = i0+i;
			_internalPointIndex[Index++] = i0+i+1;
			_internalPointIndex[Index++] = i1+i;

		}


		
	}



	void updateControlPointProjectionPosition(){
		int i;
		GLdouble		modelview[16];
		GLdouble		projection[16];
		GLint			viewport[4];
		glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev (GL_PROJECTION_MATRIX, projection);
		glGetIntegerv (GL_VIEWPORT, viewport); 

		for(i=0;i<ctrlPointCount;i++){			
			double c1, c2, c3;
			int result = gluProject( _CtrlPoints[i].x() , _CtrlPoints[i].y() , _CtrlPoints[i].z() ,	
				modelview, projection , viewport, &c1, &c2, &c3	) ;
			if ( result ){
				_Ctrl2DPoints[i].x = c1;
				_Ctrl2DPoints[i].y = c2;
			}
			else{
				printf("there must be sth wrong with the matrix\n");
			}	
		}

	}

	void getBezierPointsCoordinate(void)
	{
		int Index = 0;
		for( int n = 0; n < subdiv; n++ )
		{
			for( int i = 0; i < subdiv-n; i++ )
			{
				// The barycentric coordinates
				float lambda1 = float(i)/(subdiv-1);
				float lambda2 = float(n)/(subdiv-1);
				float lambda0 = 1-lambda1-lambda2;

				// Move up the pyramid as our wanted surface coordinate is at the top
				//    3
				//   2 6
				//  1 5 8
				// 0 4 7 9
				//
				//     002
				//   101 011
				// 200 110 020
				pVec b1_200, b1_110, b1_020, b1_101, b1_011, b1_002;
				
				b1_200 = _CtrlPoints[0]*lambda0 + _CtrlPoints[1]*lambda1 + _CtrlPoints[4]*lambda2;
				b1_110 = _CtrlPoints[1]*lambda0 + _CtrlPoints[2]*lambda1 + _CtrlPoints[5]*lambda2;
				b1_020 = _CtrlPoints[2]*lambda0 + _CtrlPoints[3]*lambda1 + _CtrlPoints[6]*lambda2;
				b1_101  = _CtrlPoints[4]*lambda0 + _CtrlPoints[5]*lambda1 + _CtrlPoints[7]*lambda2;
				b1_011 = _CtrlPoints[5]*lambda0 + _CtrlPoints[6]*lambda1 + _CtrlPoints[8]*lambda2;
				b1_002   = _CtrlPoints[7]*lambda0 + _CtrlPoints[8]*lambda1 + _CtrlPoints[9]*lambda2;

				//   001
				// 100 010
				pVec b2_100, b2_010, b2_001;
				b2_100		= b1_200*lambda0  + b1_110*lambda1	+ b1_101*lambda2 ;
				b2_010		= b1_110*lambda0  + b1_020*lambda1  + b1_011*lambda2 ;
				b2_001		= b1_101*lambda0  + b1_011*lambda1  + b1_002*lambda2 ;

				_InternalPoints[Index]  = b2_100*lambda0  + 	b2_010*lambda1  + 	b2_001*lambda2 ;

				Index++;
				if( Index > internalPointCount )		printf( "sth wrong \n");

			}
		}
		int normal_index = 0;

		for(int i=0;i<internalPointIndexCount;)
		{
			pVec v1 = _InternalPoints[_internalPointIndex[i]] - _InternalPoints[_internalPointIndex[i+1]];
			pVec v2 = _InternalPoints[_internalPointIndex[i+1]] - _InternalPoints[_internalPointIndex[i+2]];
			pVec result = Cross( v1, v2 );
			result.normalize();
			_normals[normal_index] =result;
			normal_index++;
			i+=3;
		}

		
	}

	virtual void ReInitialize( ){
		int i,j,index = 0;
		float x,y,z,k,l;
		for( i=0;i< degree;i++){
			for( j=0;j< degree - i;j++){
				k=(float)i/(float)degree-0.5f ;
				l=(float)j/(float)degree-0.5f ;
				x=size*k; 
				y=size*l;
				if(sourceShapeFlag==0)		z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)	z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2) z=0.0f;
				_CtrlPoints[index] = pVec( x,y,z);
				index ++;

			}
		}

		getBezierPointsCoordinate();

	}

	virtual void InitializeCtrlPointOffset( float dx, float dy){
		int i,j,index = 0;
		float x,y,z,k,l;
		for( i=0;i< degree;i++){
			for( j=0;j< degree - i;j++){
				k=(float)i/(float)degree-0.5f ;
				l=(float)j/(float)degree-0.5f ;
				x=size*k +dx; 
				y=size*l +dy;
				if(sourceShapeFlag==0)		z=size*(k*k+l*l-0.25f)*2.0f; 
				else if(sourceShapeFlag==1)	z=size*((float)sin(2.0f*PI*k)+(float)sin(2.0f*PI*l))/4.0f; 
				else if(sourceShapeFlag==2) z=0.0f;
				_CtrlPoints[index] = pVec( x,y,z);
				index ++;

			}
		}

		getBezierPointsCoordinate();
	}



	void DrawCtrlPoint(){
		if ( showCtrlPointsFlag ){
			glDisable(GL_LIGHTING);
			glColor3f( 0.0f, 0.0f, 0.0f);
			glPointSize(5.0f);
			glBegin(GL_POINTS);
			for( int i=0;i< ctrlPointCount ;i++){
				glVertex3fv( (float*)&_CtrlPoints[i] );
				
			}
			glEnd();

			for(int i=0;i<ctrlPointIndexCount;){
				glBegin(GL_LINE_LOOP);
				for( int  j=0;j<3;j++){
					glVertex3fv(  (float*)&_CtrlPoints[_ctrlPointIndex[i] ] );		
					i++;
				}
				glEnd();
			}
			if(ctrlPointSelectedFlag)
			{
				
				glPointSize( 10.0f);
				glColor3f(0.0f,1.0f,0.2f);
				glBegin(GL_POINTS);
					glVertex3fv( (float*)&_CtrlPoints[selectedCtrlPointIndex]);
				glEnd();
			}
			glEnable( GL_LIGHTING );

		}
		

		

	}

	void DrawTriangleMesh(){


		glColor3f( 0.0f, 0.0f, 1.0f);

		if( displaytype == 0 ){
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_fspecular);
			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
			glMaterialfv(GL_BACK, GL_SPECULAR, mat_fspecular);
			for(int i=0;i<internalPointIndexCount;)
			{
				glBegin( GL_TRIANGLES );
				glNormal3fv( (float*)&_normals[i/3] );
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i] ] );
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i+1] ] );
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i+2] ] );
				glEnd();
				i+=3;
			}
		}
		
		else if( displaytype == 1 ){
			glDisable( GL_LIGHTING );
			glColor3fv(wire_color);
			for(int i=0;i<internalPointIndexCount-3;)
			{
				glBegin( GL_LINE_LOOP);
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i] ] );
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i+1] ] );
				glVertex3fv( (float*)&_InternalPoints[_internalPointIndex[i+2] ] );
				glEnd();
				i+=3;
			}
			glEnable( GL_LIGHTING );

		}
		
	}

	virtual void DrawSurface(){
		glPushMatrix();
		glTranslatef( offsetX, offsetY, 0 );
		updateControlPointProjectionPosition();
		DrawCtrlPoint();
		DrawTriangleMesh();
		glPopMatrix();

	}

	virtual void UpdateBezierSurface(){
		getBezierPointsCoordinate();
	}
	virtual void updateSelectedControlPointPosition( float x, float y){
		if ( ctrlPointSelectedFlag ){
			pVec selectedPoint = _CtrlPoints[ selectedCtrlPointIndex ];

			GLdouble		modelview[16];
			GLdouble		projection[16];
			GLint			viewport[4];
			glGetDoublev( GL_MODELVIEW_MATRIX,	modelview);
			glGetDoublev( GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewport);
			double targetx,targety,targetz;
			gluProject( selectedPoint.x() , selectedPoint.y() , selectedPoint.z(),
				modelview,projection,viewport,
				&targetx,&targety,&targetz);

			float winX = x;   
			float winY = viewport[3] - y; 
			float winZ = targetz;

			gluUnProject(winX,winY,winZ,modelview,projection,viewport,&targetx,&targety,&targetz); 
			_CtrlPoints[ selectedCtrlPointIndex ] = pVec( targetx, targety, targetz );
		}
		
	}



	virtual int checkSurfaceSelected( vector2D input){
		
		int vertex_index[4] = {0,3,9,0};
		float product = 1;
		//check if the point is within the triangle
		showCtrlPointsFlag = 1;
		for( int i=0; i < 3;i++){
			pVec vside(	_Ctrl2DPoints[vertex_index[i+1]].x - _Ctrl2DPoints[vertex_index[i]].x, 
						_Ctrl2DPoints[vertex_index[i+1]].y - _Ctrl2DPoints[vertex_index[i]].y, 1 ); 
			pVec v( input.x - _Ctrl2DPoints[vertex_index[i]].x,
					input.y - _Ctrl2DPoints[vertex_index[i]].y, 1);
			pVec result = Cross( v,vside);
			if ( result.z() > 0 ) {
				showCtrlPointsFlag = 0;
				break;
			}
		}
		
		
		return showCtrlPointsFlag;
	}
	virtual int getTheNearestCtrlPointIndex( vector2D input){
		int i;
		const float threshold= 10.0f; 
		float diffx,diffy; 
		float diff; 
		float mindist; 

		mindist=threshold; 
		ctrlPointSelectedFlag = 0;


		for( i=0; i < ctrlPointCount; i++){
			
			diffx=input.x- _Ctrl2DPoints[i].x; diffy=input.y- _Ctrl2DPoints[i].y;
			diff=diffx*diffx+diffy*diffy;
			if(diff<=mindist)
			{
				mindist=diff;
				selectedCtrlPointIndex = i;
				ctrlPointSelectedFlag = 1;
				printf( "Triangle Patch Index %d selected \n", i );
			}
				
		}

		
		return ctrlPointSelectedFlag;
	}

	inline int getCtrlPointIndex(int y, int x ){
		return y*(degree+1) - (1+y)*y/2 + x;
	}
	//void getTriangleAt( int x, int y, pVec*& p1, pVec*& p2, pVec*& p3){
	//	//very specified condition
	//	p1 = _CtrlPoints[getCtrlPointIndex(x,y)];
	//	p2 = _CtrlPoints[getCtrlPointIndex(x+1,y)];
	//	p3 = _CtrlPoints[getCtrlPointIndex(x,y+1)];
	//}


	void ConnectWithQuadBezierSurface( BezierSurfacePatch& quadsurface ){
		//TOP BOTTON connection only
		pVec qp1,qp2,qp3 ;
		for ( int j=0; j<3; j++ ){
			
			
			quadsurface.getTriangleAt(j,3,qp1,qp2,qp3);
			pVec& tp1 = _CtrlPoints[getCtrlPointIndex(j,0)];
			pVec& tp2 = _CtrlPoints[getCtrlPointIndex(j,1)];
			pVec& tp3 = _CtrlPoints[getCtrlPointIndex(j+1,0)];
			pVec mid = (tp2 + qp3 ) * 0.5;
			tp1 = mid;
			quadsurface.ctrlPoint[j][3].setVec( mid );
			/*pVec dn = (tp3 - tp1);
			dn.normalize();
			tp3 = (dn+tp1)*2 - qp3;*/
		}
		
		pVec&lastpoint = _CtrlPoints[getCtrlPointIndex(3,0)];
		lastpoint = quadsurface.ctrlPoint[3][3].topVec();
		
		this->getBezierPointsCoordinate();
		quadsurface.getBezierPointsCoordinate();

	}
};