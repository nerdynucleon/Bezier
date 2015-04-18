#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtx/string_cast.hpp"
#define FPS 60
#define EPS 0.00001
#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
  public:
    int w, h; // width and height
};

class patch {
	public:
		glm::vec4 points[16];
};

class vecPair {
	public:
		glm::vec4 v0;
		glm::vec3 v1;
		vecPair(glm::vec4 u0, glm::vec3 u1){
			v0 = u0;
			v1 = u1;
		}
		vecPair(){};
};

class UV{
	public:
		float u;
		float v;
		UV(float, float);
		UV();
};

UV::UV(){
	u = 0.0f; v = 0.0f;
}

UV::UV(float uin, float vin){
	u = uin;
	v = vin;
}

class tri{
	public:
		glm::vec4 pts[3];
		UV uv[3];
		glm::vec3 n[3];
		patch* p;
		tri(patch*, UV,UV,UV, glm::vec4,glm::vec4,glm::vec4, glm::vec3,glm::vec3,glm::vec3);
		tri(patch*,bool);
};

tri::tri(patch *pat, UV uv0, UV uv1, UV uv2, glm::vec4 pt0, glm::vec4 pt1, glm::vec4 pt2, glm::vec3 n0, glm::vec3 n1, glm::vec3 n2){
	uv[0] = uv0;
	uv[1] = uv1;
	uv[2] = uv2;
	pts[0] = pt0;
	pts[1] = pt1;
	pts[2] = pt2;
	n[0] = n0; n[1] = n1; n[2]= n2;
	p = pat;
}



//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;

bool flat = false;
int num_patches = 0;
vector<patch*> Patches;
float param = 0.10;
vector<tri*> toDo;
// Input parameters
bool adaptive = true;
char *filename = NULL;
bool bez = true;

//****************************************************
// Simple init function
//****************************************************
bool wireframe = true;

float maxDim(){
	float max = 0;
	for(int i = 0; i < Patches.size(); i++){
		patch *p = Patches[i];
		for(int j = 0; j < 16; j++){
			if(abs(p->points[j].x) > max) max = abs(p->points[j].x);
			if(abs(p->points[j].y) > max) max = abs(p->points[j].y);
			if(abs(p->points[j].z) > max) max = abs(p->points[j].z);
		}
	}
	return max;
}

void initScene(){
  // Nothing to do here for this simple example.
  glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
  glLoadIdentity();				        // make sure transformation is "zero'd"
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	float max = 1.0/maxDim();
	glScalef(max,max,max);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
glEnable(GL_DEPTH_TEST);	
glShadeModel(GL_SMOOTH);
	//glLightModeli(parameter, GL_TRUE);
	/* Set Up light */
	float diffuse0[]={0.5, 0.5, 0.5, 1.0};
	float ambient0[]={0.5, 0.5, 0.5, 1.0};
	float specular0[]={0.1, 0.1, 0.1, 1.0};
	float light0_pos[]={0.0, 0.0, -3,0, 1.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
	float ambient[] = {0.2, 0.2, 0.2, 1.0};
float diffuse[] = {0.4, 0.4, 0.4, 1.0};
float specular[] = {0.1, 0.1, 0.1, 1.0};
float shine = 1.0;
glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);
}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, viewport.w, 0, viewport.h);

}


//****************************************************
// Translation Matrix
//****************************************************
void rotateFunc(int key, int x, int y){
	glMatrixMode(GL_MODELVIEW);
	int modifier = glutGetModifiers();	
	switch(key){
	case GLUT_KEY_LEFT:
		if(modifier != GLUT_ACTIVE_SHIFT) glRotatef(1.0, 0.0, 0.0, 1.0); 
		else glTranslatef(0.1, 0.0, 0.0);
		break;
  case GLUT_KEY_UP:
		if(modifier != GLUT_ACTIVE_SHIFT) glRotatef(-1.0, 1.0, 0.0, 0.0);
		else glTranslatef(0.0, 0.0, -0.1);
		break;
	case GLUT_KEY_RIGHT:
		if(modifier != GLUT_ACTIVE_SHIFT) glRotatef(-1.0, 0.0, 0.0, 1.0);
		else glTranslatef(-0.1, 0.0, 0.0);
		break; 
	case GLUT_KEY_DOWN:
		if(modifier != GLUT_ACTIVE_SHIFT) glRotatef(1.0, 1.0, 0.0, 0.0);
		else glTranslatef(0.0, 0.0, 0.1);
		break; 
	}	
}

void transFunc(unsigned char key, int x, int y){
	glMatrixMode(GL_MODELVIEW);
	switch(key){
	case 43:
		glScalef(1.10,1.10,1.10);
		break;
	case 45:
		glScalef(0.9,0.9,0.9);
		break;
	case 119:
		if(wireframe){
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		} else {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		wireframe = !wireframe;
		break;
	case 115:
		if(flat){
			glShadeModel(GL_SMOOTH);
		} else {
			glShadeModel(GL_FLAT);	
		}
		flat = !flat;
		break;
	}
}



//****************************************************
// Tesselate Bezier
//****************************************************

glm::mat4 matx3 = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));

vecPair bzCInt(glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, float u){
	float up = 1.0-u;
	glm::vec4 A = v0*up + v1*u;
	glm::vec4 B = v1*up + v2*u;
	glm::vec4 C = v2*up + v3*u;
	glm::vec4 D = A*up + B*u;
	glm::vec4 E = B*up + C*u;
	
	return vecPair(D*up + E*u, glm::vec3(matx3 * (E -D)));
}

vecPair bzPInt(patch *p, float u, float v){
	vecPair vCurve[4];
	vecPair uCurve[4];
	for(int i = 0; i < 4; i++) vCurve[i] = bzCInt(p->points[i*4],p->points[i*4+1],p->points[i*4+2],p->points[i*4+3],u);
	for(int j = 0; j < 4; j++) uCurve[j] = bzCInt(p->points[j],p->points[j+4],p->points[j+8],p->points[j+12],v);
	vecPair p1 = bzCInt(vCurve[0].v0, vCurve[1].v0, vCurve[2].v0, vCurve[3].v0 ,v);
	vecPair p2 = bzCInt(uCurve[0].v0, uCurve[1].v0, uCurve[2].v0, uCurve[3].v0, u);
	return vecPair(p1.v0, glm::normalize(glm::cross(p2.v1, p1.v1)));
}

tri::tri(patch* pat, bool first)
{
	if(first){
		pts[0] = pat->points[0];
		pts[1] = pat->points[3];
		pts[2] = pat->points[15];
		uv[1] = UV(1.0, 0.0);
		uv[2] = UV(1.0, 1.0); 
		n[1] = bzPInt(pat, 1.0, 0.0).v1;
		n[2] = bzPInt(pat, 1.0, 1.0).v1;
	} else {
		pts[0] = pat->points[0];
		pts[1] = pat->points[12];
		pts[2] = pat->points[15];
		uv[1] =	UV(0.0, 1.0);
		uv[2] = UV(1.0, 1.0); 
		n[1] = bzPInt(pat, 0.0, 1.0).v1;
		n[2] = bzPInt(pat, 1.0, 1.0).v1;
	}
	uv[0] = UV(0.0, 0.0);
	p = pat;
	n[0] = bzPInt(pat, 0.0, 0.0).v1;
}

void uniform() {
	glColor3f(1,1,1);
	for(int i = 0; i < Patches.size(); i++){
		patch* current_patch = Patches[i];
		//Tesselate Patches Uniformly
		int num_div = (int) ((1 + EPS)/param);
		for(int iu = 0; iu < num_div; iu++){
			float u0 = iu * param;
			float u1 = u0 + param;
			for(int iv = 0; iv < num_div; iv++){
				float v0 = iv * param;
				float v1 = v0 + param;
				glBegin(GL_QUADS);
				vecPair p1 = bzPInt(current_patch, u0, v0);
				vecPair p2 = bzPInt(current_patch, u1, v0);
				vecPair p3 = bzPInt(current_patch, u1, v1);
				vecPair p4 = bzPInt(current_patch, u0, v1);
				glNormal3f(p1.v1.x, p1.v1.y, p1.v1.z);
				glVertex4f(p1.v0.x, p1.v0.y, p1.v0.z, p1.v0.w);
				glNormal3f(p2.v1.x, p2.v1.y, p2.v1.z);
				glVertex4f(p2.v0.x, p2.v0.y, p2.v0.z, p2.v0.w);
				glNormal3f(p3.v1.x, p3.v1.y, p3.v1.z);
				glVertex4f(p3.v0.x, p3.v0.y, p3.v0.z, p3.v0.w);
				glNormal3f(p4.v1.x, p4.v1.y, p4.v1.z);
				glVertex4f(p4.v0.x, p4.v0.y, p4.v0.z, p4.v0.w);
				glEnd();
			}
		}
	}
}

inline float length(glm::vec4 v){
	return sqrt(sqr(v.x) + sqr(v.y) + sqr(v.z));
}

void subdivide()
{
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	tri* t = toDo.back();
	toDo.pop_back();
	/* Check midpoints */
	float u0 = (t->uv[0].u + t->uv[1].u)/2;
	float u1 = (t->uv[1].u + t->uv[2].u)/2;
	float u2 = (t->uv[2].u + t->uv[0].u)/2;
	float v0 = (t->uv[0].v + t->uv[1].v)/2;
	float v1 = (t->uv[1].v + t->uv[2].v)/2;
	float v2 = (t->uv[2].v + t->uv[0].v)/2;
	UV buv0 = UV(u0, v0);
	UV buv1 = UV(u1, v1);
	UV buv2 = UV(u2, v2);
	vecPair p1 = bzPInt(t->p,u0,v0);
	vecPair p2 = bzPInt(t->p,u1,v1);
	vecPair p3 = bzPInt(t->p,u2,v2);
	glm::vec4 bez0 = p1.v0;
	glm::vec4 bez1 = p2.v0;
	glm::vec4 bez2 = p3.v0;
	glm::vec3 n0 = p1.v1;
	glm::vec3 n1 = p2.v1;
	glm::vec3 n2 = p3.v1;
	glm::vec4 mid0 = scale * (t->pts[0] + t->pts[1]); 
	glm::vec4 mid1 = scale * (t->pts[1] + t->pts[2]);
	glm::vec4 mid2 = scale * (t->pts[2] + t->pts[0]);
	float d0 = length(bez0 - mid0);
	float d1 = length(bez1 - mid1);
	float d2 = length(bez2 - mid2);
	/* Generate New Triangles */
	if(d0 >= param){
		if(d1 >= param){
			if(d2 >= param){
				/* 1 1 1 */
				toDo.push_back(new tri(t->p, t->uv[0],  buv2, buv0,
																		 t->pts[0], bez2, bez0,
																		 t->n[0],   n2,   n0   ));

				toDo.push_back(new tri(t->p, t->uv[1],  buv0, buv1,
																		 t->pts[1], bez0, bez1,
																		 t->n[1],   n0,   n1   ));

				toDo.push_back(new tri(t->p, t->uv[2],  buv1, buv2,
																		 t->pts[2], bez1, bez2,
																		 t->n[2],   n1,   n2   ));

				toDo.push_back(new tri(t->p, buv1, buv0, buv2,
																		 bez1, bez0, bez2,
																		 n1,   n0,   n2        ));
			} else {
				/* 1 1 0 */	
				toDo.push_back(new tri(t->p, t->uv[0],  t->uv[2], buv0,
																		 t->pts[0], t->pts[2], bez0,
																		 t->n[0],		t->n[2], n0  ));

				toDo.push_back(new tri(t->p, t->uv[1], buv0, buv1,
																		 t->pts[1], bez0, bez1,
																		 t->n[1], n0, n1));

				toDo.push_back(new tri(t->p, t->uv[2], buv1, buv0,
																		 t->pts[2], bez1, bez0,
																		 t->n[2], n1, n0));
			}
		} else {
			if(d2 >= param){
				/* 1 0 1 */	
				toDo.push_back(new tri(t->p, t->uv[0], buv2, buv0,
																		 t->pts[0], bez2, bez0,
																		 t->n[0], n2, n0));

				toDo.push_back(new tri(t->p, t->uv[1], buv0, t->uv[2],
																		 t->pts[1], bez0, t->pts[2],
																		 t->n[1], n0, t->n[2]));	
	
				toDo.push_back(new tri(t->p, buv0, buv2, t->uv[2],
																		 bez0, bez2, t->pts[2],
																		 n0, n2, t->n[2]));	
			} else {
				/* 1 0 0 */
				toDo.push_back(new tri(t->p, t->uv[1], buv0, t->uv[2],
																		 t->pts[1], bez0, t->pts[2],
																		 t->n[1], n0, t->n[2]));
	
				toDo.push_back(new tri(t->p, buv0, t->uv[0],  t->uv[2],
																		 bez0, t->pts[0],  t->pts[2],
																		 n0, t->n[0], t->n[2]));
			}
		}
	} else {
		if(d1 >= param){
			if(d2 >= param){
				/* 0 1 1 */
				toDo.push_back(new tri(t->p, t->uv[1], t->uv[0], buv2,
																		 t->pts[1], t->pts[0], bez2,
																		 t->n[1], t->n[0], n2));

				toDo.push_back(new tri(t->p, t->uv[1], buv2, buv1,
																		 t->pts[1], bez2, bez1,
																		 t->n[1], n2, n1));

				toDo.push_back(new tri(t->p, t->uv[2], buv1, buv2,
																		 t->pts[2], bez1, bez2,
																		 t->n[2], n1, n2));
			} else {
				/* 0 1 0 */
				toDo.push_back(new tri(t->p, t->uv[1], t->uv[0], buv1,
																		 t->pts[1], t->pts[0], bez1,
																		 t->n[1], t->n[0], n1));

				toDo.push_back(new tri(t->p, t->uv[0], t->uv[2], buv1,
																		 t->pts[0], t->pts[2], bez1,
																		 t->n[0], t->n[2], n1));
			}
		} else {
			if(d2 >= param){
				/* 0 0 1*/
				toDo.push_back(new tri(t->p, t->uv[1], t->uv[0], buv2,
																		 t->pts[1], t->pts[0], bez2,
																		 t->n[1], t->n[0], n2));

				toDo.push_back(new tri(t->p, t->uv[2], t->uv[1], buv2,
																		 t->pts[2], t->pts[1], bez2,
																		 t->n[2], t->n[1], n2));
			} else {
				/* 0 0 0 Base Case: Draw Triangle*/	
				glColor3f(1,1,1);
				glBegin(GL_TRIANGLES);
				glNormal3f(t->n[0].x, t->n[0].y, t->n[0].z);
				glVertex4f(t->pts[0].x, t->pts[0].y, t->pts[0].z, t->pts[0].w);
				glNormal3f(t->n[1].x, t->n[1].y, t->n[1].z);
				glVertex4f(t->pts[1].x, t->pts[1].y, t->pts[1].z, t->pts[1].w);
				glNormal3f(t->n[2].x, t->n[2].y, t->n[2].z);
				glVertex4f(t->pts[2].x, t->pts[2].y, t->pts[2].z, t->pts[2].w);
				glEnd();
				
			}
		}
	}
	delete t;
}

void adaptiveTes() {
	for(int i = 0; i < Patches.size(); i++){
		tri* t1 = new tri(Patches[i], true);
		tri* t2 = new tri(Patches[i], false);
		toDo.push_back(t1);
		toDo.push_back(t2);
		while(!toDo.empty())
		{
			subdivide();
		}
	}
}



//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// clear the color buffer
  glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
	// Wireframe rendering
	// Draw here	
	if(!adaptive){
		uniform();
  } else {
		adaptiveTes();
	}
	glFlush();
  glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}


// Process Input Filename
void fileType(char *filename){
	while(filename[0] != '\0'){
		filename++;
	}
	if (filename[-4]=='.' && filename[-3]=='b' && filename[-2]=='e' && filename[-1]=='z') {
		bez = true;
	} else if (filename[-4] == '.' && filename[-3] == 'o' && filename[-2] == 'b' && filename[-1] == 'j'){
		bez = false;
	} else {
		perror("Invalid filetype. Must be bezier or obj.");
		exit(-1);
	}
}

// Get patches from open bezier file
void getPatches(std::fstream *file){
	int i = 0;
	std::string next_line;
	while(i < num_patches){
		patch* newpatch = new patch;
		int pt = 0;
		while(pt < 16){
			getline(*file, next_line);
			char * cstr = new char [next_line.length()+1];
			strcpy (cstr, next_line.c_str());
			char * p = strtok (cstr," ");
			for(int k = 0; k < 4; k++){
				float pos[3];
				for(int j = 0; j < 3; j++){
					if(p == NULL){
						perror("invalid bez file.\n"); exit(-1);
					}
					pos[j] = atof(p);
					p = strtok(NULL," ");
				}
				newpatch->points[pt] = glm::vec4(pos[0],pos[1],pos[2],1.0f);
				pt++;
			}
			delete[] cstr;
		}
		Patches.push_back(newpatch);
		i++;
		//Get empty line
		getline(*file,next_line);
		if(next_line.length()!=1){
			perror("invalid bez file space.\n"); exit(-1);
		}
	}
}

void parseFile(char *filename){
	//Open File
	std::fstream file;
	file.open(filename, std::fstream::in);
	if(!file.is_open()){
		perror("Failed to Open file.");
		exit(-1);
	}
	//Parse File
	std::string next_line;
	if(bez){
		// Bezier FileType
		std::getline(file, next_line);
		num_patches = stoi(next_line);
		getPatches(&file);
	} else {
		//Obj Filetype
		perror("obj filetype not yet implemented..\n");
	}
}

// Input format ./as3 filename subdivision flag
void parseInput(int argc, char *argv[]){
	if(argc < 3){
		perror("invalid input. Need more arguments: filename, subdivision, flag");
		exit(-1);
	}
	param = (float) atof(argv[2]);
	if(strcmp(argv[3], "-a") == 0){
		adaptive = true;
	} else if(strcmp(argv[3], "-u") == 0){
		adaptive = false;
	} else {
		perror("invalid input. select adaptive or uniform meshing");
		exit(-1);
	}
	filename = argv[1];
	fileType(filename);
	parseFile(filename);
}

void timer(int v) {
	glutPostRedisplay();
	glutTimerFunc(1000/FPS, timer, v);
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
  //This initializes glut
  glutInit(&argc, argv);
	//Parse the inputs
	parseInput(argc, argv);
  //This tells glut to use a double-buffered window with red, green, and blue channels 
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

  // Initalize theviewport size
  viewport.w = 400;
  viewport.h = 400;

  //The size and position of the window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow(argv[0]);
	glutSpecialFunc(rotateFunc);
  initScene();							// quick function to set up scene
	glutTimerFunc(100, timer, 0);
	glutKeyboardFunc(transFunc);
  glutDisplayFunc(myDisplay);				// function to run when its time to draw something
 // glutReshapeFunc(myReshape);				// function to run when the window gets resized
  glutMainLoop();							// infinite loop that will keep drawing and resizing
  // and whatever else

  return 0;
}


