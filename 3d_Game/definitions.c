
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <windows.h>
#include <windowsx.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#include <GL/GLAux.h>
#pragma comment(lib, "glut32")
#pragma comment(lib, "Opengl32")
#pragma comment(lib, "SOIL")
#include "C:\Hacking\opengl\VC-Desktop-Lesson06\SOIL.h"

#define DEFS

//Obj Defs
#define ROT_LEFT 10
#define ROT_RIGHT 20
#define ROT_UP 30
#define ROT_DOWN 40

#define SHFT_LEFT 1
#define SHFT_UP   2
#define SHFT_DOWN 3
#define SHFT_RT   4
#define SHFT_BACK 5
#define SHFT_FORWARD 6

#define DIR_LEFT 1
#define DIR_UP   2
#define DIR_DOWN 3
#define DIR_RT   4
#define DIR_CHECKPT 255

#define OBJ_CUBE 1
#define OBJ_PYRAMID 2
#define OBJ_SQ 3
#define OBJ_TEXT 4

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

//Basic Structs
typedef struct vertex {
	double x;
	double y;
	double z;
} vertex;
typedef struct physics {
	double xacc;
	double xvel;
	double yacc;
	double yvel;
	double zacc;
	double zvel;
} physics;
typedef struct camera {
	//Position
	double posx;
	double posy;
	double posz;
	//Rotation
	double rotx;
	double roty;
	double rotz;
	//Vector
	double vecx;
	double vecy;
	double vecz;
	//Global World Rotation
	double rotwrldx;
	double rotwrldy;
	double rotwrldz;
	//Global World Position
	double poswrldx;
	double poswrldy;
	double poswrldz;
	//Physics
	physics phys;
} camera;
typedef struct obj {
	void *obj;
	unsigned int type;
	bool screen;
} obj;           
typedef struct color {
	double r;
	double g;
	double b;
} color;
typedef struct tex {
	GLuint texture;
	unsigned int type;
} tex;

//Advanced Structs / Union
typedef struct square {
	//Essential
	vertex btl;
	vertex btr;
	vertex tpl;
	vertex tpr;
	//Fancy
	color clr;
	GLuint tex;
} square;
typedef struct cube  {
	square top;
	square bt;
	square front;
	square back;
	square left;
	square right;
	physics phys;
} cube;
typedef struct text {
	char *str;
	vertex pos;
	vertex rot;
} text;
typedef struct objlist {
	obj *obj;
	unsigned int bytes;
	unsigned int current;
	int i;
} objlist;

//Level Structs
typedef struct level{
	objlist objlst;
	tex *texlist;
	camera cam;
} level;
typedef struct game {
	level *levs;
	level *CurLevel;
} game;

//Colors
color red, green, blue, white, gray;

//Declarations
void ShiftCube(cube *input, int direction, double amount);
bool ChkCamColl();
void SlideSq(square *input, int axis, double amount);
void UpdateScreen(int axis, double amount);

//Vars
GLuint	base;				// Base Display List For The Font Set
GLfloat	cnt1;				// 1st Counter Used To Move Text & For Coloring
GLfloat	cnt2;				// 2nd Counter Used To Move Text & For Coloring
GLYPHMETRICSFLOAT gmf[256];	// Storage For Information About Our Outline Font Characters