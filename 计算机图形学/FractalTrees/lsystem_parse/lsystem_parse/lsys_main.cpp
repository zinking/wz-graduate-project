//--------------------------------------------------------------------------------------
//
// Lsystem Plant Demo
//
// Author: wangzhen
// Email: zinking3@gmail.com
//
//--------------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>

using namespace std;

#include <GL/glew.h>
#include <GL/glut.h>
#include "nvGlutManipulators.h"
#include "lsystem.h"
#include "lsystem_impl.h"

////////////////////////////////////////////////////////////////////////////////
//
// Globals
//
////////////////////////////////////////////////////////////////////////////////

LSystem*	g_lsystem = NULL;
glShader*	shader = NULL;

int			g_mousex = 0, g_mousey = 0;
int			g_width = 0, g_height = 0;

nv::GlutExamine manipulator;

enum UIOption {
    OPTION_DISPLAY_WIREFRAME,
    OPTION_LERP_LAYERS,
    OPTION_ANIMATE,
    OPTION_USE_PROGRAM,
    OPTION_COUNT
};
bool options[OPTION_COUNT];
map<char,UIOption> optionKeyMap;

////////////////////////////////////////////////////////////////////////////////
//
// Functions
//
////////////////////////////////////////////////////////////////////////////////
void parseLSystem(const std::string& filename) {

	char straxiom[256], rule[256];

	std::vector<std::string> rules;

	std::ifstream istr(filename.c_str());
	if (!istr.is_open()) { 
		return;
	}

	istr.getline(straxiom, 256);
	while (istr.getline(rule, 256)) {
		rules.push_back(rule);
	}
	if( g_lsystem != NULL ) delete g_lsystem;

	g_lsystem = new LSystem(straxiom, rules);
	g_lsystem->setMaxDepth( 1 );
	//g_lsystem->setMaxDepth( g_lsystem->maxDepth() + 1 );
	
	shader = g_lsystem->getShader();
	g_lsystem->generatelist();

}



//
//
//////////////////////////////////////////////////////////////////////
void init_opengl() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.8, 0.8, 0.8, 1.0);

    glewInit();

	glEnable(GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	try {
		parseLSystem("oaky.lsys");
	} catch (std::runtime_error& e) {
		//std::cerr << "error: " << e.what() << "\n";
		return;
	}

	
			
	



}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    manipulator.applyTransform();


	glClearColor(0.8,0.8,0.8,0);
	//glClearColor(0.0,0.0,0.0,0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	glPushMatrix();
		glLoadIdentity();
		manipulator.applyTransform();
		/*glColor3f(1.0, 0.0, 0.5);
		glutWireTorus(0.2, 1.0, 20, 30);
		glColor3f(0.3, 0.5, 1.0);
		glutWireTeapot(0.5f);*/

		g_lsystem->renderGL();
	glPopMatrix();




	glutSwapBuffers();
}

//
//
//////////////////////////////////////////////////////////////////////
void idle() {
    if ( options[OPTION_ANIMATE])
        manipulator.idle();
    
    glutPostRedisplay();
}

//
//
//////////////////////////////////////////////////////////////////////
void key(unsigned char k, int x, int y) {
    //k = tolower(k);

    if (optionKeyMap.find(k) != optionKeyMap.end())
        options[optionKeyMap[k]] = !options[optionKeyMap[k]];
	
    switch(k) {
        case 27:
        case 'q':
            exit(0);
            break;
		case 'g':
			g_lsystem->setMaxDepth( g_lsystem->maxDepth() + 1 );
			g_lsystem->generatelist();
			break;
		case 'd':
			g_lsystem->setMaxDepth( g_lsystem->maxDepth() - 1 );
			g_lsystem->generatelist();
			break;
		case 'a':
			parseLSystem("bush.lsys");
			g_lsystem->setMaxDepth( 1 );
			g_lsystem->generatelist();
			break;
		case 'o':
			parseLSystem("oaky.lsys");
			g_lsystem->setMaxDepth( 1 );
			g_lsystem->generatelist();
			break;
		case 'f':
			parseLSystem("fractal.lsys");
			g_lsystem->setMaxDepth( 1 );
			g_lsystem->generatelist();
			break;


		case 'r':
			g_lsystem->setMaxDepth(0);
			g_lsystem->generatelist();
			break;

		
    }
    
	glutPostRedisplay();
}

//
//
//////////////////////////////////////////////////////////////////////
void resize(int w, int h) {
	g_width = w;
	g_height = h;
    glViewport(0, 0, w, h);

	//pFilterBox->setWindowViewPort(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    manipulator.reshape(w, h);
}

//
//
//////////////////////////////////////////////////////////////////////
void mouse(int button, int state, int x, int y) {
    manipulator.mouse(button, state, x, y);
	glutPostRedisplay();
}

void passiveMotion(int x, int y) {
	g_mousex = x;
	g_mousey = g_height - y;
	glutPostRedisplay();
}
//
//
//////////////////////////////////////////////////////////////////////
void motion(int x, int y) {
    manipulator.motion(x, y);
}

void main_menu(int i)
{
  key((unsigned char) i, 0, 0);
}
void init_menus()
{
    int object_menu_id = glutCreateMenu(main_menu);
    {
        glutCreateMenu(main_menu);
        glutAddMenuEntry("' ' - Toggle continuous Animation", ' ');
        glutAddMenuEntry("'g' - Make the plant grow one step forward", 'g');
        glutAddMenuEntry("'d' - Make the plant grow one step backward", 'd');
        glutAddMenuEntry("'a' - Render the bush like plant model ", 'a');
        glutAddMenuEntry("'o' - Render the oak Tree like plant model", 'o');
        glutAddMenuEntry("'r' - reset plant model to zero depth", 'r');
		glutAddMenuEntry("'w' - Toggle displaying wireframe", 'w');
        glutAddMenuEntry("Quit (esc)", '\033');
        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }
}
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("Lsystem Plant");

	init_opengl();

    manipulator.setDollyActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_SHIFT);
    manipulator.setPanActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_CTRL);
	
    manipulator.setDollyPosition( -2.0f);

	glutDisplayFunc(display);
	glutPassiveMotionFunc(passiveMotion);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);
    init_menus();

    //configure the options
    optionKeyMap['w'] = OPTION_DISPLAY_WIREFRAME;
    options[OPTION_DISPLAY_WIREFRAME] = true;

    optionKeyMap[' '] = OPTION_ANIMATE;
    options[OPTION_ANIMATE] = true;

    //print the help info
    printf( "  Commands:\n");
    printf( "  q / [ESC] - Quit the application\n");
    printf( "  [Ctrl ]+MOUSE   - Move the plant around\n");
    printf( "  [Shift]+MOUSE   - scale the plant around\n");
    printf( "  Mouse           - Rotate the plant around\n");
	printf( "  Mouse Right     - To See command Menus\n");


	glutMainLoop();


	return 0;
}
