#ifndef DEFS
#include "definitions.c"
#endif

//Globals
HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
bool TitleScreen = true;
char msg[1000];
int level_timer;
int global_x = 640, global_y = 480;
int mouse_x = 0, mouse_y = 0, i = 0;
int main_clock;
double global_decrease = 0.001;
double global_alpha = 1.0;
double tele_clock = 0;

double ly = 0.0;
double angle = 0.0;
double angled = 0.0;
double sensitivity = 0.02;
double walkbias = 0.0;
double walkbiasangle = 0.0;
int iter = 0;

vertex clearance = {.x = 0.0, .y = 0.0, .z = 0.0};

double friction = .5;
double gravity = 1;

cube hitbox;
double globtranx = 1.5, globtranz = -7.0;

bool bounce = false;

const double piover180 = 0.0174532925;
square *temp;
POINT global_pos;



//Standalone Utilities
double dabs(double in) {
	if (in >= 0)
		return in;
	else
		return (in / -1);
}
bool isposorneg(double num) {
	if (num >= 0)
		return true;
	else
		return false;
}
cube sqstocube(square a, square b) {
	cube out;
	out.top = a;
	out.bt = b;

	out.front.tpl = a.btl;
	out.front.tpr = a.btr;
	out.front.btl = b.btl;
	out.front.btr = b.btr;

	out.back.tpl = a.tpl;
	out.back.tpr = a.tpr;
	out.back.btl = b.tpl;
	out.back.btr = b.tpr;

	out.left.tpl = a.tpl;
	out.left.tpr = a.btl;
	out.left.btl = b.tpl;
	out.left.btr = b.btl;

	out.right.tpl = a.tpr;
	out.right.tpr = a.btr;
	out.right.btl = b.tpr;
	out.right.btr = b.btr;

	return out;
}
cube pttosqs(vertex a, double r) {
	cube out;
	out.top.tpl = (vertex){ .x = a.x + r, .z = a.z + r, .y = a.y + r };
	out.top.tpr = (vertex){ .x = a.x - r, .z = a.z + r, .y = a.y + r };
	out.top.btl = (vertex){ .x = a.x + r, .z = a.z - r, .y = a.y + r };
	out.top.btr = (vertex){ .x = a.x - r, .z = a.z - r, .y = a.y + r };

	out.bt.tpl  = (vertex){ .x = a.x + r, .z = a.z + r, .y = a.y - r };
	out.bt.tpr  = (vertex){ .x = a.x - r, .z = a.z + r, .y = a.y - r };
	out.bt.btl  = (vertex){ .x = a.x + r, .z = a.z - r, .y = a.y - r };
	out.bt.btr  = (vertex){ .x = a.x - r, .z = a.z - r, .y = a.y - r };
	return out;
}

//Colors
void DefineColors() {
	red.r = 1.0;
	red.g = 0.0;
	red.b = 0.0;
	green.r = 0.0;
	green.g = 1.0;
	green.b = 0.0;
	blue.r = 0.0;
	blue.g = 0.0;
	blue.b = 1.0;
	white.r = 1.0;
	white.g = 1.0;
	white.b = 1.0;
	gray.r = 100;
	gray.g = 100;
	gray.b = 100;
	return;
}

//Fonts
GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID

	base = glGenLists(256);								// Storage For 256 Characters

	font = CreateFont(-12,							// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		FW_BOLD,						// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		ANSI_CHARSET,					// Character Set Identifier
		OUT_TT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		ANTIALIASED_QUALITY,			// Output Quality
		FF_DONTCARE | DEFAULT_PITCH,		// Family And Pitch
		L"Comic Sans MS");				// Font Name

	SelectObject(hDC, font);							// Selects The Font We Created

	wglUseFontOutlines(hDC,							// Select The Current DC
		0,								// Starting Character
		255,							// Number Of Display Lists To Build
		base,							// Starting Display Lists
		0.0f,							// Deviation From The True Outlines
		0.2f,							// Font Thickness In The Z Direction
		WGL_FONT_POLYGONS,				// Use Polygons, Not Lines
		gmf);							// Address Of Buffer To Recieve Data
}
GLvoid KillFont(GLvoid)									// Delete The Font
{
	glDeleteLists(base, 256);								// Delete All 256 Characters
}
GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	float		length = 0;								// Used To Find The Length Of The Text
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	for (unsigned int loop = 0; loop<(strlen(text)); loop++)	// Loop To Find Text Length
	{
		length += gmf[text[loop]].gmfCellIncX;			// Increase Length By Each Characters Width
	}

	glTranslatef(-length / 2, 0.0f, 0.0f);					// Center Our Text On The Screen

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base);									// Sets The Base Character to 0
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

//Globals
game *CG;
game g1;
static cube cl1, cl2, ground;
static text t1;
objlist screen = {.i = 0};
//Object List Functions
void add_obj(void *structure, unsigned int type, objlist *l) {
	//Reallocate if neccessary
	if (l->current + sizeof(obj) >= l->bytes) {
		l->bytes += sizeof(obj) * 10;
		if (!(l->current && l->bytes)) {
			l->obj = (obj *)calloc(sizeof(obj) * 10, sizeof(obj));
		}
		else {
			l->obj = (obj *)realloc(l->obj, l->bytes);
		}
	}
	//Define and Increment
	l->obj[l->i].obj = structure;
	l->obj[l->i].type = type;
	l->obj[l->i].screen = false;
	l->i++; l->current += sizeof(obj);
}

//Structure Init Functions
void InitCamera(camera *cam) {
	//Position
	cam->posx = 0.0;
	cam->posy = 0.0;
	cam->posz = 0.0;
	//Rotation
	cam->rotx = 0.0;
	cam->roty = 0.0;
	cam->rotz = -1.0;
	//Up-vector
	cam->vecx = 0.0;
	cam->vecy = 1.0;
	cam->vecz = 0.0;
	return;
}
void InitCube(cube *input) {
	//Top
	input->top.tpr.x = 1.0;
	input->top.tpr.y = 1.0;
	input->top.tpr.z = -1.0;

	input->top.tpl.x = -1.0;
	input->top.tpl.y = 1.0;
	input->top.tpl.z = -1.0;

	input->top.btl.x = -1.0;
	input->top.btl.y = 1.0;
	input->top.btl.z = 1.0;

	input->top.btr.x = 1.0;
	input->top.btr.y = 1.0;
	input->top.btr.z = 1.0;
	//Bottom
	input->bt.tpr.x = 1.0;
	input->bt.tpr.y = -1.0;
	input->bt.tpr.z = 1.0;

	input->bt.tpl.x = -1.0;
	input->bt.tpl.y = -1.0;
	input->bt.tpl.z = 1.0;

	input->bt.btl.x = -1.0;
	input->bt.btl.y = -1.0;
	input->bt.btl.z = -1.0;

	input->bt.btr.x = 1.0;
	input->bt.btr.y = -1.0;
	input->bt.btr.z = -1.0;
	//Front
	input->front.tpr.x = 1.0;
	input->front.tpr.y = 1.0;
	input->front.tpr.z = 1.0;

	input->front.tpl.x = -1.0;
	input->front.tpl.y = 1.0;
	input->front.tpl.z = 1.0;

	input->front.btl.x = -1.0;
	input->front.btl.y = -1.0;
	input->front.btl.z = 1.0;

	input->front.btr.x = 1.0;
	input->front.btr.y = -1.0;
	input->front.btr.z = 1.0;
	//Back
	input->back.tpr.x = 1.0;
	input->back.tpr.y = -1.0;
	input->back.tpr.z = -1.0;

	input->back.tpl.x = -1.0;
	input->back.tpl.y = -1.0;
	input->back.tpl.z = -1.0;

	input->back.btl.x = -1.0;
	input->back.btl.y = 1.0;
	input->back.btl.z = -1.0;

	input->back.btr.x = 1.0;
	input->back.btr.y = 1.0;
	input->back.btr.z = -1.0;
	//Left
	input->left.tpr.x = -1.0;
	input->left.tpr.y = 1.0;
	input->left.tpr.z = 1.0;

	input->left.tpl.x = -1.0;
	input->left.tpl.y = 1.0;
	input->left.tpl.z = -1.0;

	input->left.btl.x = -1.0;
	input->left.btl.y = -1.0;
	input->left.btl.z = -1.0;

	input->left.btr.x = -1.0;
	input->left.btr.y = -1.0;
	input->left.btr.z = 1.0;
	//Right
	input->right.tpr.x = 1.0;
	input->right.tpr.y = 1.0;
	input->right.tpr.z = -1.0;

	input->right.tpl.x = 1.0;
	input->right.tpl.y = 1.0;
	input->right.tpl.z = 1.0;

	input->right.btl.x = 1.0;
	input->right.btl.y = -1.0;
	input->right.btl.z = 1.0;

	input->right.btr.x = 1.0;
	input->right.btr.y = -1.0;
	input->right.btr.z = -1.0;
}
void InitSquare(square *input) {
	input->clr = white;
	input->tex = 0;

	input->tpl.x = 0.5;
	input->tpl.y = 1.0;
	input->tpl.z = -6.0;

	input->tpr.x = 2.5;
	input->tpr.y = 1.0;
	input->tpr.z = -6.0;

	input->btr.x = 2.5;
	input->btr.y = -1.0;
	input->btr.z = -6.0;

	input->btl.x = 0.5;
	input->btl.y = -1.0;
	input->btl.z = -6.0;
	return;
}
void InitText(text *input) {
	input->pos.x = 0.0;
	input->pos.y = 0.0;
	input->pos.z = 0.0;

	input->rot.x = 0.0;
	input->rot.y = 0.0;
	input->pos.z = 0.0;

	input->str = "Default";
}

//Setups
void SetupHitBox() {
	vertex vcam;
	vcam = (vertex){ .x = CG->CurLevel->cam.posx - 1.5, .z = CG->CurLevel->cam.posz + 7, .y = CG->CurLevel->cam.posy - .4};

	InitCube(&hitbox);
	hitbox = pttosqs(vcam, 1);
	hitbox = sqstocube(hitbox.top, hitbox.bt);
}

//Level Building Functions
void SetupLevel1() {
	InitCube(&cl1);
	InitCube(&cl2);
	InitCube(&ground);
	InitText(&t1);

	//Ground
	ground.top.btl = (vertex) {.x = -20, .y = -1.5, .z = -20};
	ground.top.btr = (vertex) {.x = 20, .y = -1.5, .z = -20 };
	ground.top.tpl = (vertex) {.x = -20, .y = -1.5, .z = 20 };
	ground.top.tpr = (vertex) { .x = 20, .y = -1.5, .z = 20 };
	ground.bt = ground.top;
	SlideSq(&ground.bt, AXIS_Y, -.5);
	ground = sqstocube(ground.top, ground.bt);
	ground.top.clr = gray;
	
	//Text
	t1.str = calloc(200, 1);
	strcpy(t1.str, "Hello");
	t1.pos.z = -3.0;

	ShiftCube(&cl2, SHFT_RT, 5);
	g1.levs = calloc(1, sizeof(level));
	InitCamera(&g1.levs[0].cam);
	add_obj((void *)&cl1, OBJ_CUBE, &g1.levs[0].objlst);
	add_obj((void *)&cl2, OBJ_CUBE, &g1.levs[0].objlst);
	add_obj((void *)&ground, OBJ_CUBE, &g1.levs[0].objlst);
	//add_obj((void *)&t1, OBJ_TEXT, &g1.levs[0].objlst);
	add_obj((void *)&t1, OBJ_TEXT, &screen);
	screen.obj[0].screen = true;
	//add_obj((void *)&hitbox, OBJ_CUBE, &g1.levs[0].objlst);
	CG->CurLevel = &g1.levs[0];
}

//Drawing Functions
void DrawSquare(square input) {
	if (input.tex != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, input.tex);
		glBegin(GL_QUADS);
		glColor3d(input.clr.r, input.clr.g, input.clr.b);
		glTexCoord2d(0.0, 0.0); glVertex3d(input.tpl.x, input.tpl.y, input.tpl.z);
		glTexCoord2d(0.0, 1.0); glVertex3d(input.tpr.x, input.tpr.y, input.tpr.z);
		glTexCoord2d(1.0, 1.0); glVertex3d(input.btr.x, input.btr.y, input.btr.z);
		glTexCoord2d(1.0, 0.0); glVertex3d(input.btl.x, input.btl.y, input.btl.z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glBegin(GL_QUADS);
		glColor3d(input.clr.r, input.clr.g, input.clr.b);
		glVertex3d(input.tpl.x, input.tpl.y, input.tpl.z);
		glVertex3d(input.tpr.x, input.tpr.y, input.tpr.z);
		glVertex3d(input.btr.x, input.btr.y, input.btr.z);
		glVertex3d(input.btl.x, input.btl.y, input.btl.z);
		glEnd();
	}
	return;
}
void DrawCube(cube input) {
	glBegin(GL_QUADS);
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(input.top.tpr.x, input.top.tpr.y, input.top.tpr.z);
	glVertex3d(input.top.tpl.x, input.top.tpl.y, input.top.tpl.z);
	glVertex3d(input.top.btl.x, input.top.btl.y, input.top.btl.z);
	glVertex3d(input.top.btr.x, input.top.btr.y, input.top.btr.z);
	// Bottom
	glColor3d(1.0, 0.5, 0.0);
	glVertex3d(input.bt.tpr.x, input.bt.tpr.y, input.bt.tpr.z);
	glVertex3d(input.bt.tpl.x, input.bt.tpl.y, input.bt.tpl.z);
	glVertex3d(input.bt.btl.x, input.bt.btl.y, input.bt.btl.z);
	glVertex3d(input.bt.btr.x, input.bt.btr.y, input.bt.btr.z);
	// Front
	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(input.front.tpr.x, input.front.tpr.y, input.front.tpr.z);
	glVertex3d(input.front.tpl.x, input.front.tpl.y, input.front.tpl.z);
	glVertex3d(input.front.btl.x, input.front.btl.y, input.front.btl.z);
	glVertex3d(input.front.btr.x, input.front.btr.y, input.front.btr.z);
	// back OK*/
	glColor3d(1.0, 1.0, 0.0);
	glVertex3d(input.back.tpr.x, input.back.tpr.y, input.back.tpr.z);
	glVertex3d(input.back.tpl.x, input.back.tpl.y, input.back.tpl.z);
	glVertex3d(input.back.btl.x, input.back.btl.y, input.back.btl.z);
	glVertex3d(input.back.btr.x, input.back.btr.y, input.back.btr.z);
	// left OK
	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(input.left.tpr.x, input.left.tpr.y, input.left.tpr.z);
	glVertex3d(input.left.tpl.x, input.left.tpl.y, input.left.tpl.z);
	glVertex3d(input.left.btl.x, input.left.btl.y, input.left.btl.z);
	glVertex3d(input.left.btr.x, input.left.btr.y, input.left.btr.z);
	// right
	glColor3d(1.0, 0.0, 1.0);
	glVertex3d(input.right.tpr.x, input.right.tpr.y, input.right.tpr.z);
	glVertex3d(input.right.tpl.x, input.right.tpl.y, input.right.tpl.z);
	glVertex3d(input.right.btl.x, input.right.btl.y, input.right.btl.z);
	glVertex3d(input.right.btr.x, input.right.btr.y, input.right.btr.z);
	glEnd();
	return;
}
void DrawTextStruct(text input, bool screen) {
	double x, y, z;
	if (screen) {
		x = -(CG->CurLevel->cam.poswrldx + CG->CurLevel->cam.posx + globtranx);
		y = -CG->CurLevel->cam.posy;
		z = -(CG->CurLevel->cam.poswrldz + CG->CurLevel->cam.posz + globtranz);

		glTranslated(x, y, z);
		glRotated(input.rot.x, 1.0, 0.0, 0.0);
		glRotated(input.rot.y, 0.0, 1.0, 0.0);
		glRotated(input.rot.z, 0.0, 0.0, 1.0);
		glTranslated(input.pos.x - x, input.pos.y - y, input.pos.z - z);
		glPrint(input.str);
	}
	else {
		glTranslated(input.pos.x, input.pos.y, input.pos.z);
		glRotated(input.rot.x, 1.0, 0.0, 0.0);
		glRotated(input.rot.y, 0.0, 1.0, 0.0);
		glRotated(input.rot.z, 0.0, 0.0, 1.0);
		glPrint(input.str);
	}
}
void DrawObj(obj o) {
	cube *temp; square *temp2; text *temp3;
	if (o.type == OBJ_CUBE) {
		temp = (cube *)o.obj;
		DrawCube(*temp);
	}
	else if (o.type == OBJ_SQ) {
		temp2 = (square *)o.obj;
		DrawSquare(*temp2);
	}
	else if (o.type == OBJ_TEXT) {
		temp3 = (text *)o.obj;
		DrawTextStruct(*temp3, o.screen);
	}
}
void DrawScreen() {
	int i;
	for (i = 0; i < screen.i; i++) {
		DrawObj(screen.obj[i]);
	}
}
void DrawLevel(level l) {
	int i;
	for (i = 0; i < l.objlst.i; i++) {
		DrawObj(l.objlst.obj[i]);
	}
	DrawScreen();
}

//Moving Functions
void SlideSq(square *input, int axis, double amount) {
	switch (axis) {
	case AXIS_X:
		input->btl.x += amount;
		input->tpl.x += amount;
		input->tpr.x += amount;
		input->btr.x += amount;
		break;
	case AXIS_Y:
		input->btl.y += amount;
		input->tpl.y += amount;
		input->tpr.y += amount;
		input->btr.y += amount;
		break;
	case AXIS_Z:
		input->btl.z += amount;
		input->tpl.z += amount;
		input->tpr.z += amount;
		input->btr.z += amount;
		break;
	}
}
void SlideCube(cube *input, int axis, double amount) {
	SlideSq(&input->top, axis, amount);
	SlideSq(&input->back, axis, amount);
	SlideSq(&input->bt, axis, amount);
	SlideSq(&input->front, axis, amount);
	SlideSq(&input->left, axis, amount);
	SlideSq(&input->right, axis, amount);
}
void ShiftSq(square *input, int direction, double amount) {
	switch (direction) {

	case SHFT_LEFT:
		input->btl.x -= amount;
		input->tpl.x -= amount;
		input->tpr.x -= amount;
		input->btr.x -= amount;
		break;
	case SHFT_RT:
		input->btl.x += amount;
		input->tpl.x += amount;
		input->tpr.x += amount;
		input->btr.x += amount;
		break;
	case SHFT_DOWN:
		input->btl.y -= amount;
		input->tpl.y -= amount;
		input->tpr.y -= amount;
		input->btr.y -= amount;
		break;
	case SHFT_UP:
		input->btl.y += amount;
		input->tpl.y += amount;
		input->tpr.y += amount;
		input->btr.y += amount;
		break;
	case SHFT_BACK:
		input->btl.z -= amount;
		input->tpl.z -= amount;
		input->tpr.z -= amount;
		input->btr.z -= amount;
		break;
	case SHFT_FORWARD:
		input->btl.z += amount;
		input->tpl.z += amount;
		input->tpr.z += amount;
		input->btr.z += amount;
		break;
	default:
		break;
	}
	return;
}
void ShiftCube(cube *input, int direction, double amount) {
	ShiftSq(&input->top, direction, amount);
	ShiftSq(&input->back, direction, amount);
	ShiftSq(&input->bt, direction, amount);
	ShiftSq(&input->front, direction, amount);
	ShiftSq(&input->left, direction, amount);
	ShiftSq(&input->right, direction, amount);
	return;
}
void Bounce(unsigned int direction, double amplitude, double frequency) {
	if ((walkbiasangle <= 1.0) && (direction == DIR_UP)) {
		walkbiasangle = 359.0;
		walkbias = sin(walkbiasangle * piover180 * amplitude) / frequency;
		return;
	}
	else if ((walkbiasangle >= 359.0) && (direction == DIR_DOWN)) {
		walkbiasangle = 0.0f;
		walkbias = sin(walkbiasangle * piover180 * amplitude) / frequency;
		return;
	}

	if (direction == DIR_DOWN) {
		walkbiasangle -= 1;
	} 
	else if (direction == DIR_UP) {
		walkbiasangle += 1;
	}
	walkbias = sin(walkbiasangle * piover180 * amplitude) / frequency;     
}

//Camera Movement
void CheckCamRot() {
	glRotated(CG->CurLevel->cam.rotwrldx, 1.0, 0.0, 0.0);
	glRotated(CG->CurLevel->cam.rotwrldy, 0.0, 1.0, 0.0);
	glRotated(CG->CurLevel->cam.rotwrldz, 0.0, 0.0, 1.0);
}
void MoveCam(int direction, double amount) {
	/*bounce[0] = GetAsyncKeyState('A'); 
	bounce[1] = GetAsyncKeyState('D'); 
	bounce[2] = GetAsyncKeyState('W');
	bounce[3] = GetAsyncKeyState('S');*/

	if (direction == SHFT_LEFT) {
		SlideCube(&hitbox, AXIS_X, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl())
			SlideCube(&hitbox, AXIS_X, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		else {
			CG->CurLevel->cam.poswrldx += cos(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_X, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}

		SlideCube(&hitbox, AXIS_Z, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Z, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		} 
		else {
			CG->CurLevel->cam.poswrldz -= sin(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_Z, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		//Bounce(DIR_UP, .4, 2);
	}
	if (direction == SHFT_RT) {
		SlideCube(&hitbox, AXIS_X, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_X, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldx -= cos(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_X, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}

		SlideCube(&hitbox, AXIS_Z, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Z, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldz += sin(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_Z, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		//Bounce(DIR_DOWN, .4, 2);
	}
	if (direction == SHFT_FORWARD) {
		SlideCube(&hitbox, AXIS_Z, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Z, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldz += cos(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_Z, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}

		SlideCube(&hitbox, AXIS_X, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_X, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldx += sin(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_X, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		//if (!(bounce[0] || bounce[1])) {
			//Bounce(DIR_UP, .4, 2);
		//}
	}
	if (direction == SHFT_BACK) {
		SlideCube(&hitbox, AXIS_Z, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Z, -cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldz -= cos(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_Z, cos(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}

		SlideCube(&hitbox, AXIS_X, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_X, -sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		else {
			CG->CurLevel->cam.poswrldx -= sin(CG->CurLevel->cam.rotwrldy*piover180) * amount;
			UpdateScreen(AXIS_X, sin(CG->CurLevel->cam.rotwrldy*piover180) * amount);
		}
		//if (!(bounce[0] || bounce[1])) {
			//Bounce(DIR_DOWN, .4, 2);
		//}
	}

	if (direction == SHFT_DOWN) {
		SlideCube(&hitbox, AXIS_Y, amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Y, -amount);
		}
		else {
			CG->CurLevel->cam.posy -= amount;
		}
	}

	if (direction == SHFT_UP) {
		SlideCube(&hitbox, AXIS_Y, -amount);
		if (!ChkCamColl()) {
			SlideCube(&hitbox, AXIS_Y, amount);
		}
		else {
			CG->CurLevel->cam.posy += amount;
		}
	}
}

//Collisions
bool ChkCamColl() {
	int i; bool coll = true; camera *cam = &CG->CurLevel->cam;
	cube *temp, hitbox2; double radius, tradius;
	hitbox2 = hitbox;
	radius = dabs(hitbox.top.tpl.x - hitbox.top.tpr.x) / 2.0;

	for (i = 0; i < CG->CurLevel->objlst.i; i++) { //Main Iterator
		if (CG->CurLevel->objlst.obj[i].type == OBJ_CUBE) {//For Cubes
			temp = (cube *)CG->CurLevel->objlst.obj[i].obj;
			tradius = dabs(temp->top.tpl.x - temp->top.tpr.x) / 2.0;
			//Bound Checking (for small->big)
			if (((temp->top.tpl.x > hitbox.top.tpl.x) && (temp->top.tpr.x < hitbox.top.tpl.x)) || //X Axis 1
				((temp->top.tpr.x < hitbox.top.tpr.x) && (temp->top.tpl.x > hitbox.top.tpr.x))) { //X Axis 2
				if (((temp->top.tpl.z > hitbox.top.tpl.z) && (temp->top.btl.z < hitbox.top.tpl.z)) ||  //Z Axis 1
					((temp->top.btl.z < hitbox.top.btl.z) && (temp->top.tpl.z > hitbox.top.btl.z))) {  //Z Axis 2
					if (((temp->bt.tpl.y < hitbox.top.tpl.y) && (temp->top.tpl.y > hitbox.top.tpl.y)) || //Y Axis 1
						((temp->top.tpl.y > hitbox.bt.tpl.y) && (temp->bt.tpl.y < hitbox.bt.tpl.y))) {   //Y Axis 2
						coll = false;
					}
				}
			}
			//big->small
			if (((hitbox.top.tpl.x > temp->top.tpl.x) && (hitbox.top.tpl.x < temp->top.tpr.x)) || //X Axis 1
				((hitbox.top.tpr.x < temp->top.tpr.x) && (hitbox.top.tpr.x > temp->top.tpl.x))) { //X Axis 2
				if (((hitbox.top.tpl.z > temp->top.tpl.z) && (hitbox.top.tpl.z < temp->top.btl.z)) ||  //Z Axis 1
					((hitbox.top.btl.z < temp->top.btl.z) && (hitbox.top.btl.z > temp->top.tpl.z))) {  //Z Axis 2
					if (((hitbox.bt.tpl.y < temp->top.tpl.y) && (hitbox.top.tpl.y > temp->top.tpl.y)) || //Y Axis 1
						((hitbox.bt.tpl.y < temp->bt.tpl.y) && (hitbox.top.tpl.y > temp->bt.tpl.y))) {   //Y Axis 2
						coll = false;
					}
				}
			}
			if (coll == false) {
				clearance.x = (hitbox.top.tpl.x - radius) - (temp->top.tpl.x - tradius);
				clearance.y = (hitbox.top.tpl.y - radius) - (temp->top.tpl.y - tradius);
				clearance.z = (hitbox.top.tpl.z - radius) - (temp->top.tpl.z - tradius);
				return false;
			}
		}
	}
	return true;
}

//Updates
void UpdateScreen(int axis, double amount) {
	int i; text *temp;
	for (i = 0; i < screen.i; i++) {
		if (screen.obj[i].type == OBJ_TEXT) {
			temp = (text *)screen.obj[i].obj;
			if (axis == AXIS_X)
				temp->pos.x += amount;
			if (axis == AXIS_Y)
				temp->pos.y += amount;
			if (axis == AXIS_Z)
				temp->pos.z += amount;
			if (axis == ROT_LEFT || axis == ROT_RIGHT) {
				temp->rot.y += amount;
			}
			if (axis == ROT_UP || axis == ROT_DOWN) {
				temp->rot.x += amount;
			}
		}
	}
}
void UpdateKeyStates() {
	bounce = false;
	//Move Camera
	if (GetAsyncKeyState('W')) {
		MoveCam(SHFT_FORWARD, 0.0075f);
		Bounce(DIR_UP, .4, 4);
		bounce = true;
	}
	if (GetAsyncKeyState('S')) {
		MoveCam(SHFT_BACK, 0.0075f);
		if (!bounce) {
			Bounce(DIR_DOWN, .4, 4);
			bounce = true;
		}
	}
	if (GetAsyncKeyState('D')) {
		MoveCam(SHFT_RT, 0.0075f);
		if (!bounce) {
			Bounce(DIR_UP, .4, 4);
			bounce = true;
		}
	}
	if (GetAsyncKeyState('A')) {
		MoveCam(SHFT_LEFT, 0.0075f);
		if (!bounce) {
			Bounce(DIR_DOWN, .4, 4);
			bounce = true;
		}
	}
	if (GetAsyncKeyState(VK_SPACE)) {
		MoveCam(SHFT_UP, 0.0075f);
		if (!bounce) {
			Bounce(DIR_UP, .4, 4);
			bounce = true;
		}
	}
	if (GetAsyncKeyState(VK_SHIFT)) {
		MoveCam(SHFT_DOWN, 0.0075f);
		if (!bounce) {
			Bounce(DIR_UP, .4, 4);
			bounce = true;
		}
	}
	//Mouse Movement
	GetCursorPos(&global_pos);
	if ((global_pos.x != global_x / 2) || (global_pos.y != global_y / 2)) {
		if (global_pos.x > global_x / 2) {//Right?
			CG->CurLevel->cam.rotwrldy += (global_x / 2 - global_pos.x) * sensitivity;
			UpdateScreen(ROT_RIGHT, (global_x / 2 - global_pos.x) * sensitivity);
			//heading -= .5;
		}
		if (global_pos.x < global_x / 2) {//Left?
			CG->CurLevel->cam.rotwrldy += (global_x / 2 - global_pos.x) * sensitivity;
			UpdateScreen(ROT_LEFT, (global_x / 2 - global_pos.x) * sensitivity);
			//heading += .5;
		}

		if (CG->CurLevel->cam.rotwrldy > 360)
			CG->CurLevel->cam.rotwrldy = 0 + (CG->CurLevel->cam.rotwrldy - 360);
		if (CG->CurLevel->cam.rotwrldy < 0)
			CG->CurLevel->cam.rotwrldy = 360 + CG->CurLevel->cam.rotwrldy;

		if (global_pos.y < global_y / 2)  {//Up?
			angled = (global_y / 2 - global_pos.y) * sensitivity * .05;
			if (angle + angled < 1) {
				ly = sin(angle + angled);
				angle += angled;
				UpdateScreen(ROT_UP, (global_y / 2 - global_pos.y) * sensitivity);
			}
		}
		if (global_pos.y > global_y / 2) {//Down?
			angled = (global_y / 2 - global_pos.y) * sensitivity * .05;
			if (angle + angled > -1) {
				ly = sin(angle + angled);
				angle += angled;
				UpdateScreen(ROT_DOWN, (global_y / 2 - global_pos.y) * sensitivity);
			}
		}

		if (!GetAsyncKeyState('Z'))
			SetCursorPos(global_x / 2, global_y / 2);
		if (GetAsyncKeyState('Q'))
			Sleep(1);
	}
}
void UpdatePhysics(double dtime) {
	//-----Camera Physics-----
	// X Axis
	if (CG->CurLevel->cam.phys.xacc != 0) {
		CG->CurLevel->cam.phys.xvel += CG->CurLevel->cam.phys.xacc * dtime; //Vel = Acc * Time
		if (CG->CurLevel->cam.phys.xacc > 0) { //Friction
			CG->CurLevel->cam.phys.xacc -= friction * dtime;
			if (CG->CurLevel->cam.phys.xacc < 0)
				CG->CurLevel->cam.phys.xacc = 0.0;
		}
		else if (CG->CurLevel->cam.phys.xacc < 0) {
			CG->CurLevel->cam.phys.xacc += friction * dtime;
			if (CG->CurLevel->cam.phys.xacc > 0)
				CG->CurLevel->cam.phys.xacc = 0.0;
		}
	}
	// Z Axis
	if (CG->CurLevel->cam.phys.zacc != 0) {
		CG->CurLevel->cam.phys.zvel += CG->CurLevel->cam.phys.zacc * dtime; //Vel = Acc * Time
		if (CG->CurLevel->cam.phys.zacc > 0) { //Friction
			CG->CurLevel->cam.phys.zacc -= friction * dtime;
			if (CG->CurLevel->cam.phys.zacc < 0)
				CG->CurLevel->cam.phys.zacc = 0.0;
		}
		else if (CG->CurLevel->cam.phys.zacc < 0) {
			CG->CurLevel->cam.phys.zacc += friction * dtime;
			if (CG->CurLevel->cam.phys.zacc > 0)
				CG->CurLevel->cam.phys.zacc = 0.0;
		}
	}
	// Y Axis
	if (CG->CurLevel->cam.phys.yacc != 0) {
		CG->CurLevel->cam.phys.yvel += CG->CurLevel->cam.phys.yacc * dtime; //Vel = Acc * Time
		if (CG->CurLevel->cam.phys.yacc > 0) { //Friction
			CG->CurLevel->cam.phys.yacc -= gravity * dtime;
			if (CG->CurLevel->cam.phys.yacc < 0)
				CG->CurLevel->cam.phys.yacc = 0.0;
		}
		else if (CG->CurLevel->cam.phys.yacc < 0) {
			CG->CurLevel->cam.phys.yacc += gravity * dtime;
			if (CG->CurLevel->cam.phys.yacc > 0)
				CG->CurLevel->cam.phys.yacc = 0.0;
		}
	}
	// Moving

}
void UpdateAll(double dtime) {
	UpdateKeyStates();
	UpdatePhysics(dtime);
}

//Template
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	CG->CurLevel->cam.poswrldy = -walkbias - 0.25f;

	if (GetAsyncKeyState('V'))
		Sleep(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	gluLookAt(CG->CurLevel->cam.posx, CG->CurLevel->cam.posy, CG->CurLevel->cam.posz, CG->CurLevel->cam.rotx, CG->CurLevel->cam.roty + ly, CG->CurLevel->cam.rotz, CG->CurLevel->cam.vecx, CG->CurLevel->cam.vecy, CG->CurLevel->cam.vecz);
	glRotated(360 - CG->CurLevel->cam.rotwrldy, 0.0, 1.0, 0.0);
	//glRotated(CG->CurLevel->cam.rotwrldx, 1.0, 0.0, 0.0);
	if (GetAsyncKeyState('T'))
		CG->CurLevel->cam.rotwrldx += .5;

	if (GetAsyncKeyState('H')) {
		t1.rot.x += .1;
	}

	glTranslated(CG->CurLevel->cam.poswrldx, CG->CurLevel->cam.poswrldy, CG->CurLevel->cam.poswrldz);
	glTranslated(globtranx, 0.0, globtranz);

	DrawLevel(*CG->CurLevel);
	//ShowFullTexFade(CG->global_tex, &global_alpha, global_decrease);

	if (TitleScreen) {
		//ShowFullTex(CG->t[0].texture, 1.0);
	}

	glFlush();
	return TRUE;
}
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0, 0.0, 0.0, 0.5);				// Black Background
	glClearDepth(1.0);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Set The Blending Function For Translucency

	BuildFont();

	//glEnable(GL_BLEND);			// Turn Blending On
	//glDisable(GL_DEPTH_TEST);	// Turn Depth Testing Off
	return TRUE;										// Initialization Went OK
}
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}
GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}
BOOL CreateGLWindow(LPCWSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", L"NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, L"Program Will Now Close.", L"ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Initialization Failed.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

//Main
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{

	switch (uMsg)									// Check For Windows Messages	
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		// LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
		// The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
		// A NonZero Value Indicates The Window Is Minimized.
		if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
			active = TRUE;						// Program Is Active
		else
			active = FALSE;						// Program Is No Longer Active

		return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		if (wParam == 'C') {
			if (!(lParam & (1 << 30))) {
				CG->CurLevel->cam.rotwrldx += 45;
			}
		}
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
		if (keys[wParam] == keys['P']) {
			//DispAxis(player.tpr);
		}
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}

	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		global_x = LOWORD(lParam);
		global_y = HIWORD(lParam);
		return 0;								// Jump Back
	}
	case WM_MOUSEMOVE:
	{
		mouse_x = GET_X_LPARAM(lParam);
		mouse_y = GET_Y_LPARAM(lParam);
		return 0;
	}

	}


	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;								// Bool Variable To Exit Loop
	DefineColors();
	//Setup Game
	CG = &g1;
	SetupLevel1();
	SetupHitBox();

	ShowCursor(0);
	SetCursorPos(global_x / 2, global_y / 2);

	// Ask The User Which Screen Mode They Prefer
	fullscreen = false;

	// Create Our OpenGL Window
	if (!CreateGLWindow(L"3d Game", global_x, global_y, 16, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	//Loop Settings
	double lastTime, CurrentTime, elapsed;
	lastTime = clock();
	lastTime /= CLOCKS_PER_SEC;

	while (!done)									// Loop That Runs While done=FALSE
	{
		CurrentTime = clock();
		CurrentTime /= CLOCKS_PER_SEC;
		elapsed = CurrentTime - lastTime;

		UpdateAll(elapsed);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if (!TitleScreen) {
				// Gravity
				//UpdatePhysics(*CG->CurrentLevel, elapsed);
			}

			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1] = FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow(L"NeHe's Solid Object Tutorial", global_x, global_y, 16, fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}

		lastTime = CurrentTime;
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
