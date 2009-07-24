#include <GL/glui.h>
#include "BezierMangeMent.h"
#include <cmath>

using namespace std;


BezierPatch* bsp;
TriangularBezierPatch* tbp;
int main_window;
BezierManager manager;



#define ONE_SECOND 	 1	// one second
#define MAX_TEXTURES 	 1  	// max textures displayed


enum {
	BUTTON_LEFT = 0,
	BUTTON_RIGHT,
	BUTTON_LEFT_TRANSLATE,
};

enum CTRL_STATUS{
	DRAGGING = 0,
	EDIT,
	SELECT,
	ANIMATE
};


int mButton = -1;
int mOldY, mOldX;

double uOldY, uOldX;
CTRL_STATUS ctrlstatus = DRAGGING;


float eye[3] =	{	0.0f,	0.0f, 7.0f};
float rot[3] =	{	0.0f,	0.0f, 0.0f};

const int ESC = 27;

int wireframe = 0;

const int GL_WIN_WIDTH = 800;
const int GL_WIN_HEIGHT = 600;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;


int animationFlag=0;

float aspect_ratio = 0;


void glutResize(int width, int height)
{
	glViewport(0, 0, GL_WIN_WIDTH, GL_WIN_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspect_ratio = (float)GL_WIN_WIDTH/(float)GL_WIN_HEIGHT;
	gluPerspective(45.0, aspect_ratio, 1.0, 300.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ESC:
		exit(0);
	case 'W':
	case 'w':
		wireframe = !wireframe;
		break;
	case '1':
		manager.setDemoNumber(1);
		manager.Demo(1);
		break;
	case '2':
		manager.setDemoNumber(2);
		manager.Demo(1);
		break;
	case '3':
		//manager.setDemoNumber(3);
	case 'a':
	case 'A':
		manager.Demo(2);
		break;
	}
}


void clamp(float *v)
{
	int i;

	for (i = 0; i < 3; i ++)
		if (v[i] > 360 || v[i] < -360)
			v[i] = 0;
}






void glutMotion(int x, int y)
{
	vector2D p2(x,GL_WIN_HEIGHT - y); 

	if (mButton == BUTTON_LEFT)
	{
		if ( ctrlstatus == DRAGGING ){
			//in motion
			rot[0] -= (mOldY - y);
			rot[1] -= (mOldX - x);
			clamp (rot);
		}
		else if ( ctrlstatus == EDIT ){
			double uX,uY;

			glPushMatrix();
			glTranslatef (-eye[0], -eye[1], -eye[2]);
			glRotatef(rot[0], 1.0f, 0.0f, 0.0f);
			glRotatef(rot[1], 0.0f, 1.0f, 0.0f);
			glRotatef(rot[2], 0.0f, 0.0f, 1.0f);
			manager.UpdateControlPointPosition( x, y );
			glPopMatrix();
			manager.Demo(2);

			glutSwapBuffers();

		}
		

		
	}
	else if (mButton == BUTTON_RIGHT)
	{
     
		//in motion
		eye[2] -= (mOldY - y) * 0.05f; 
		clamp (rot);
	} 
	else if (mButton == BUTTON_LEFT_TRANSLATE)
	{
		eye[0] += (mOldX - x) * 0.01f;
		eye[1] -= (mOldY - y) * 0.01f;
		clamp (rot);
	}

	mOldX = x;
	mOldY = y;
}





void glutMouse(int button, int state, int x, int y)
{
	vector2D p2(x,GL_WIN_HEIGHT - y); 
	if(state == GLUT_DOWN)
	{		
		mOldX = x;
		mOldY = y;
		switch(button)
		{
		case GLUT_LEFT_BUTTON:	
			
			if (   manager.getPatchSelectedControlPoint(p2)  ) {
				ctrlstatus = EDIT;
				glutSwapBuffers();
			}
			else {
				manager.getCurrentPatch( p2 );
				
				
			}
			mButton = BUTTON_LEFT;
			break;
			
		case GLUT_RIGHT_BUTTON:
			mButton = BUTTON_RIGHT;
			break;
		}
	} else if (state == GLUT_UP)
		//mButton = -1;

		switch(button)
		{
			case GLUT_LEFT_BUTTON:
				if ( ctrlstatus == EDIT ){
					//bsp->UpdateBezierSurface();
					manager.UpdateCurrentPatch();
					mButton = BUTTON_LEFT;
					ctrlstatus = DRAGGING;
				}
				break;
		}

}

void glutMenu(int value)
{
	switch (value)
	{
	case 1:
		glutFullScreen();
		return;

	case 2:
		exit(0);
	}
}


void glutSpecial(int value, int x, int y)
{
	switch (value)
	{
	case GLUT_KEY_F1:
		glutFullScreen();
		return;
	}
}


void glutDisplay(void)
{
	if(glutGetWindow()!=main_window)glutSetWindow(main_window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glPushMatrix();
	glTranslatef (-eye[0], -eye[1], -eye[2]);
	glRotatef(rot[0], 1.0f, 0.0f, 0.0f);
	glRotatef(rot[1], 0.0f, 1.0f, 0.0f);
	glRotatef(rot[2], 0.0f, 0.0f, 1.0f);
	
	//bsp->DrawSurface();
	//bsp->DrawSurface();
	//glutSolidTeapot(1);
	manager.DrawAllPatches();
	//glutSolidTeapot(2);
	glPopMatrix();

	glutSwapBuffers();
	if ( animationFlag == 1 ) rot[1] += 0.5;
}

void InitLighting(){
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position1[] = { 0.0, 0.0, 7.0, 0.0 };
	GLfloat light_position2[] = { 0.0, 0.0, -7.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

	GLfloat mat_fspecular[] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat mat_emission[]  = { 0.0, 1.0, 1.0, 1.0 };
	GLfloat mat_ambient[]  = { 0.0, 1.0, 0.5, 1.0 };
	GLfloat mat_diffuse[]  = { 0.0, 1.0, 0.1, 1.0 };
	GLfloat mat_bspecular[] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat mat_shininess[]= { 50.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_fspecular);
	//glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_BACK, GL_SPECULAR, mat_fspecular);
	
	//glEnable(GL_DEPTH_TEST);
}


void InitializeOGL()
{
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	//glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);		// This Will Clear The Background Color To Black

	glEnable(GL_DEPTH_TEST);					// Enables Depth Testing
	glShadeModel(GL_SMOOTH);
	InitLighting();

	return;
}











GLUI *glui1,*glui2;

#define N_ID					200
#define M_ID					201
#define USUBDIVI_ID				300
#define VSUBDIVI_ID				301
#define OMEGADEG_ID				400
#define RADIUS_ID				401
#define SHOW_CTRLPOINTS_ID		500
#define SHOW_BEZIERWIRES_ID		501
#define SHOW_BEZIERSURFACES_ID	502
#define HIDDENLINEREMOVAL_ID	600
#define SOURSESHAPE_ID			700
#define ROTATION_ID				800
#define TRANSLATE_ID			801
#define ANIMATION_ID			900
#define DISPLAYTYPE_ID			901
#define DEMO_ID					902
#define ACTION_ID				903


GLUI_Checkbox *hiddenLineRemovalCheckbox;
GLUI_Translation *rotationTranslation;
GLUI_Translation *zoomTranslation;
float move[3];
int displaytypeFlag = -1;
int demonumber = -1;
int actionnumber = -1;



int		counter=0;
float	timer=0.0f;
float	fps=0.0f;

void myIdle(void)
{
	clock_t start, finish;

	if(glutGetWindow()!=main_window)glutSetWindow(main_window);

	counter++;
	start=clock(); 

	glutDisplay();

	finish=clock(); 
	timer+=(float)(finish-start)/CLOCKS_PER_SEC;
	fps=(float)counter/timer;

	glui2->sync_live();
}



void control_cb(int control)
{
	switch(control)
	{
	case TRANSLATE_ID:
		manager.UpdatePatchOffset( move[0]*0.1, move[1]*0.1);
		if(glutGetWindow()!=main_window) glutSetWindow(main_window);
		glutPostRedisplay();
		break;

	case DISPLAYTYPE_ID:
		break;
	case DEMO_ID:
		if ( demonumber == 0 ){
			manager.setDemoNumber(1);
			manager.Demo(1);

		}
		else if ( demonumber == 1 ){
			manager.setDemoNumber(2);
			manager.Demo(1);
		}
		break;
	case ACTION_ID:
		if ( actionnumber == 0 ){
			manager.Demo(1);
		}
		else if ( actionnumber == 1 ){
			manager.Demo(2);
		}
		break;
	

	
	case ANIMATION_ID:
		if(animationFlag==0)
		{
			counter=0; timer=0.0f; fps=0.0f;

			glui2->sync_live();
			ctrlstatus = DRAGGING;
			glutIdleFunc(glutDisplay);
			
		}
		else{
			glutIdleFunc(myIdle);
			
		}

		if(glutGetWindow()!=main_window) glutSetWindow(main_window);
		glutPostRedisplay();
		break;
	}
}


void myGlui()
{

	glui1=GLUI_Master.create_glui_subwindow(main_window,GLUI_SUBWINDOW_RIGHT);
	glui2=GLUI_Master.create_glui_subwindow(main_window,GLUI_SUBWINDOW_BOTTOM);



	glui1->add_statictext(""); // hspace

	glui1->add_statictext(""); // hspace

	GLUI_Panel *sourseShapePanel=glui1->add_panel("Diplay Type:");
	GLUI_RadioGroup *showWhatRadiobutton=glui1->add_radiogroup_to_panel
		(sourseShapePanel,&displaytype,DISPLAYTYPE_ID,control_cb);
	glui1->add_radiobutton_to_group(showWhatRadiobutton,"Surface");
	glui1->add_radiobutton_to_group(showWhatRadiobutton,"Wireframe");

	GLUI_Panel *demonumberpandel=glui1->add_panel("Available Demos:");
	GLUI_RadioGroup *showdemonumberRadioButton=glui1->add_radiogroup_to_panel
		(demonumberpandel,&demonumber,DEMO_ID,control_cb);
	glui1->add_radiobutton_to_group(showdemonumberRadioButton,"DEMO1");
	glui1->add_radiobutton_to_group(showdemonumberRadioButton,"DEMO2");

	GLUI_Panel *actionpanel=glui1->add_panel("Actions");
	GLUI_RadioGroup *actionnumberRadioButton=glui1->add_radiogroup_to_panel
		(actionpanel,&actionnumber,ACTION_ID,control_cb);
	glui1->add_radiobutton_to_group(actionnumberRadioButton,"Initialize:");
	glui1->add_radiobutton_to_group(actionnumberRadioButton,"Connect:");


	zoomTranslation=glui2->add_translation("MOVE",GLUI_TRANSLATION_XY,move,TRANSLATE_ID,control_cb);
	
	glui2->add_column(false);

	GLUI_RadioGroup *animationRadiobutton=glui2->add_radiogroup
		(&animationFlag,ANIMATION_ID,control_cb);
	glui2->add_radiobutton_to_group(animationRadiobutton,"Manual");
	glui2->add_radiobutton_to_group(animationRadiobutton,"Automatic");
	glui2->add_column(false);
	glui2->add_statictext("Benchmark [fps]");
	GLUI_EditText *fpsEdittext=glui2->add_edittext
		("",GLUI_EDITTEXT_FLOAT,&fps);
	glui2->add_column(false);
	glui2->add_statictext("1.Click to Select Patch");
	glui2->add_statictext("2.Then Drag the Control Point to change the patch");
	glui2->add_statictext("3.But drag Control Point on the connection edge make no sense");
	glui2->add_statictext("4.Select Patch then use Arrow to Move");
	glui2->add_statictext("HOT KEYS: 1, 2, A");
	
}












int main(int argc, char** argv)
{

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );
	glutInitWindowSize( GL_WIN_WIDTH, GL_WIN_HEIGHT );
	glutInit( &argc, argv );

	main_window = glutCreateWindow("BezierSurface Demo");

	glutReshapeFunc(glutResize);       // called every time  the screen is resized
	glutDisplayFunc(glutDisplay);      // called when window needs to be redisplayed
	glutIdleFunc(glutDisplay);         // called whenever the application is idle
	glutKeyboardFunc(glutKeyboard);    // called when the application receives a input from the keyboard
	glutMouseFunc(glutMouse);          // called when the application receives a input from the mouse
	glutMotionFunc(glutMotion);        // called when the mouse moves over the screen with one of this button pressed
	glutSpecialFunc(glutSpecial);      // called when a special key is pressed like SHIFT

	InitializeOGL();

	glutCreateMenu(glutMenu);
	glutAddMenuEntry("Full Screen", 1);
	glutAddMenuEntry("Exit", 2);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
	myGlui();

	glutMainLoop();

	return 0;
}


