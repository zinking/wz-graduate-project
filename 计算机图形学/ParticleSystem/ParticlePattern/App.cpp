//#include <gl/glew.h>
#include <gl/glut.h>

#include "Particle.h"
#include "trackball.h"
#include "quat.h"

#include "texture.h"




pVec eye(2,2,5);
pVec view(2,2,0);
pVec up(0,1,0);
pVec dimension( 10,10, 5 );
FishParticle fish(dimension);


CTrackBall			*ptrCTrackBall;


// screen settings
const int  OBJECT_HEIGHT = 800;
const int  TEXT_HEIGHT =  80;
const int  WINDOW_WIDTH =  1024;
const int  WINDOW_HEIGHT =  768;
int startX = -6, startY = -4, Width = 15,Height = 12;
int Depth = -4;

//UI Control Variables
bool pause = false;
bool collectgesture = false;
PContainer gPoints;
enum CollectTarget{ COLLECT_GESTURE = 1, COLLECT_TRACE = 4 };
CollectTarget current_target;

// scaling of 3d scene
static int scaling = 0;
static int begin_x, begin_y;
static GLfloat scale_factor = 0.5;

static void InitTextures( void ){
	//10-17 Group fish textures
	char file_name[50];
	static GLuint texID;
	for ( int i=11; i <= 17; i++ ){
		sprintf( file_name, ".\\Nimo\\FISHGroup%d.jpg", i-10 );
		texID = (GLuint)i;
		MakeFishTexture( file_name  ,texID);
		if ( glGetError() != GL_NO_ERROR ){
			cout << "binding texture " << endl;
		}

	}

	//21-24 BigFish Textures
	for ( int i=21; i <= 24; i++ ){
		sprintf( file_name, ".\\Nimo\\FISH%d.jpg", i-20 );
		texID = (GLuint)i;
		MakeFishTexture( file_name  , texID );
		if ( glGetError() != GL_NO_ERROR ){
			cout << "binding texture " << endl;
		}
	}

	texID = 1;
	MakeFishTexture( ".\\Nimo\\MarineWorld.jpg",texID );
}

static void init(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);		
	glShadeModel(GL_SMOOTH) ;

	glEnable(GL_LIGHTING) ;
	glEnable(GL_LIGHT0) ;

	glEnable(GL_DEPTH_TEST) ;
	glEnable( GL_TEXTURE_2D );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	GLfloat mat_specular[] = {0.0 , 0.0 , 1.0 , 0.05} ;
	GLfloat mat_shininess[] = {100.0} ;
	GLfloat position[] = { startX + Width/2  ,  0.5  ,  1.0  , 0.0} ;//{0.5 , 0.5 , 1.0 , 0.0} ;
	
	glMaterialfv(GL_FRONT , GL_SPECULAR , mat_specular) ;
	glMaterialfv(GL_FRONT , GL_SHININESS , mat_shininess) ;
	glLightfv(GL_LIGHT0 , GL_POSITION , position) ;


	ptrCTrackBall = new CTrackBall();
	InitTextures();


	fish.Initialize(  );


}


void DrawBackGround( ){

	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glBindTexture( GL_TEXTURE_2D, 1 );
	glBegin( GL_QUADS );
	glVertex3f( startX+Width,  startY, Depth );						glTexCoord2f(0,0);
	glVertex3f( startX+Width, startY+Height, Depth );		glTexCoord2f(1,0);
	glVertex3f( startX,startY+Height, Depth );			glTexCoord2f(1,1);
	glVertex3f( startX ,  startY, Depth );			glTexCoord2f(0,1);
	glEnd();
	glDisable( GL_BLEND );

}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
	glEnable( GL_TEXTURE_2D );
	
	glColor3f(1.0 ,1.0 , 0.0) ;
	DrawBackGround();

	glPushMatrix();
	GLfloat m[4][4];
	ptrCTrackBall->create_rotation_matrix( m );
	glScalef( scale_factor, scale_factor, scale_factor );
	glMultMatrixf( &m[0][0] );

	

	
	fish.Draw( pause );
	
	glutSolidTeapot( 2);

	glPopMatrix();

	glDisable( GL_TEXTURE_2D );
	glutSwapBuffers() ;
	glutPostRedisplay();
}

void reshape(int w , int h)
{
	glViewport(0 , 0 , (GLsizei) w , (GLsizei) h) ;
	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	gluPerspective(60.0 , (GLfloat) w / (GLfloat) h , 2.0 , 10.0) ;
	//glOrtho( 0,w,0,h,2,10 );
	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;
	gluLookAt( eye.x(), eye.y(), eye.z(),
					 view.x(), view.y(), view.z(),
					 up.x(), up.y(), up.z() ) ;
}

void animate(void)
{

}
void keyboard(unsigned char key , int x , int y)
{	
	switch (key)
	{
	case 'a' :
		glutIdleFunc(animate) ;
		break;
	case 'p':
		pause = ! pause;
		break;
	case 'g':
		collectgesture = ! collectgesture;
		current_target = COLLECT_GESTURE;
		cout <<" gesture collect enabled " << endl;
		break;
	case 't':
		collectgesture = ! collectgesture;
		current_target = COLLECT_TRACE;
	case 'r' :
		glutPostRedisplay() ;
		break;
	case 27 :
		exit(0) ;
		break ;
	default :
		break ;

	}
	glutPostRedisplay() ;
}

static void mouse(int button, int state, int x, int y)
{
	begin_x = x;
	begin_y = y;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		scaling = 0;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		scaling = 1;
	}
	else if ( button == GLUT_LEFT_BUTTON && state == GLUT_UP ){
		
		if ( collectgesture ) {
			collectgesture = false;
			ParticlePattern mpattern ;
			
			switch ( current_target ){
				case COLLECT_GESTURE:
					mpattern.CollectMouseGesturePoint( gPoints );
					fish.Add( mpattern );
					break;
				case COLLECT_TRACE:
					mpattern.CollectMouseTracePoint( gPoints );
					fish.AddTrace( mpattern );
					break;
			}
			
			gPoints.clear();
			cout << "mPattern finished " << endl;
		}


	}
}

static void motion(int x, int y)
{
	if ( collectgesture ){
		gPoints.push_back( cvPoint2D32f( x, y ));
	}
	else{
		static int count = 0;
		if (scaling){
			scale_factor = scale_factor * (1.0 + (((float) (begin_y - y)) / OBJECT_HEIGHT));
		}
		else{
			ptrCTrackBall->motioni_to_trackball( begin_x, begin_y, x, y, OBJECT_HEIGHT, WINDOW_WIDTH, &count );
		}
		begin_x = x;
		begin_y = y;
	}
	glutPostRedisplay();
}

int main(int argc , char** argv)
{
	glutInit(&argc ,argv) ;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH) ;
	glutInitWindowPosition(100 , 100) ;
	glutInitWindowSize( WINDOW_WIDTH, WINDOW_HEIGHT );
	glutCreateWindow(argv[0]) ;


	//GLenum err = glewInit();
	/*if (GLEW_OK != err){
		cout << "Unable to Initialize glew" << endl;
	}*/

	init() ;
	glutDisplayFunc(display) ;
	glutReshapeFunc(reshape) ;
	glutKeyboardFunc(keyboard) ;

	glutMouseFunc( mouse );
	glutMotionFunc( motion );

	glutMainLoop() ;
	return 0 ;
}