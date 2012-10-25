/********************************
Ryan Moriarty
CSE 470 Project 2
November 2, 2009
********************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include "bitmap.h"
#include "PerlinNoise.h"

#include <fstream>
using namespace std;

#include "loaders.h"
#include "mesh.h"

#define MIN2(a,b) (((a) < (b))?(a):(b))
#define MAX2(a,b) (((a) > (b))?(a):(b))
#define PI 3.14159
#define PORSCHE "./data/porsche.obj"
#define AL "./data/al.obj"
#define DRAGON "./data/dragon-1500.obj"
#define ASU "./data/ASU-02.obj"


#define TexWidth 512
#define TexHeight 512

// window parameters
GLint winId;
int position_x		= 0;
int position_y		= 0;
int width			= 800;
int height			= 800;
int colorWheel		= 0;

//Used to determine car movement
float x_move		= 0.0;
float y_move		= -1800.0;
float z_move		= 0.0;
float speed			= 0.0;
float speedLimit	= 30.0f;
float rotation		= 0.0;
float turn_angle = 3.0;

//Camera parameters
float fpCameraHeight = 50.0;
float ovCameraHeight = 2000.0;
float ovDistance = 1.0;
float ovCameraRotate = 0.0;

// Rotation of cylinders
float cylRotate = 0.0;

//Detect if a key is pressed
bool up_pressed		= false;
bool down_pressed	= false;
bool left_pressed	= false;
bool right_pressed	= false;
bool a_pressed		= false;
bool s_pressed		= false;

// Detect if car has been in finish area
bool finished = false;

// Menu options
bool shading_smooth = true;
bool display_lists_off = true;
bool visual_norms = false;
bool texturesOn = true;
int mat_mode = 2; // 0 = Diffuse, 1 = Specular, 2 = Design
int norm_mode = 1; // 0 = Face, 1 = Vertex
int view_mode = 2; // 0 = Top Down, 1 = First-person, 2 = Overview, 3 = 45 Degree
int nav_mode = 0; // 0 = Camera, 1 = Light source
float creaseAngle = 35;

// Texture parameters
float textureMode = GL_MODULATE;
GLint minMode = GL_NEAREST;
GLint magMode = GL_LINEAR;

// Prepare texture objects
BITMAPINFO *BitmapInfo; /* Bitmap information */
GLubyte    *BitmapBits; /* Bitmap data */
GLuint TextObj;
GLuint DiceObj;
GLuint BeanObj;
static GLubyte	textureImage[TexHeight][TexWidth][4];
ImprovedNoise noise;

// controling parameters
int mouse_button;
int mouse_x		= 0;
int mouse_y		= 0;
float scale		= 1.0;
float x_angle	= 0.0;
float y_angle	= 0.0;



//Set up lighting
float l_x = -700.0;
float l_y = -700.0; 
float l_z = 100.0;
float light_rot = 0.0;

GLfloat AmbientLight[]		= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat DiffuseLight[]		= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat SpecularLight[]		= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPos[]			= {l_x, l_y, l_z, 1.0f};

// Some color vectors
GLfloat colorBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat colorRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat colorDarkRed[] = { 0.7f, 0.0f, 0.0f, 1.0f };
GLfloat colorGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat colorGray[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat colorBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat colorWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat colorMaroon[] = { .588f, 0.0f, .196f, 1.0f };
GLfloat colorGold[] = { 1.0f, .816f, 0.4f, 1.0f };

// Various mesh data structures used in environment
Mesh* carMesh;
Mesh* alMesh;
Mesh* dragonMesh;
Mesh* asuMesh;
Mesh* cubeMesh;
Mesh* groundMesh;
Mesh* cylinderMesh;

//Clock, and char buffer to display clock
clock_t start;
char timeBuffer[124];

// perspective projection parameters
GLfloat fieldofview = 90.0;
GLfloat aspect      = 1.0;
GLfloat nearPlane   = 0.1;
GLfloat farPlane    = 10000.0;

// scene parameters
Vec3f bbxMin, bbxMax, bbxCenter;

// mesh model
Mesh* model = 0;

//Display lists
GLint myCube;
GLint cubeList;
GLint groundList;
GLint cylinderList;
GLint mySquare;

// Rotation values for various models
float alRotate = 0;
float dragonRotate = 0;
float asuRotate = 0;

// Start box parameters
float startMinX = -100;
float startMaxX = 100;
float startMinY = -100;
float startMaxY = 100;

float carPosX = 0;
float carPosY = -1800;


//Caclulating the bounding box
void calc_bbox(Vec3f& bbxmin, Vec3f& bbxmax)
{
	vector<Vec3f>& v = carMesh->m_v;

	if(v.empty()) return;		

	bbxmax = bbxmin= v[0];

	for(unsigned int i = 1;i < v.size();i++)
	{	
		bbxmin.x = MIN2(bbxmin.x, v[i].x);
		bbxmin.z = MIN2(bbxmin.z, v[i].y);
		bbxmin.y = MIN2(bbxmin.y, v[i].z);
	
		bbxmax.x = MAX2(bbxmax.x, v[i].x);
		bbxmax.z = MAX2(bbxmax.z, v[i].y);
		bbxmax.y = MAX2(bbxmax.y, v[i].z);
	}

	bbxmin.x = bbxmin.x + carPosX;
	bbxmin.y = bbxmin.y + carPosY;

	bbxmax.x = bbxmax.x + carPosX;
	bbxmax.y = bbxmax.y + carPosY;
}

//Various noise generation functions
double	FireNoise(int x, int y)
{
	double scale = 0.02;
	double n;
	double amp = 100.0;

	n =  noise.noise(x*scale , y*scale , 11.5);
	for(int i= 1; i < 20; i++){
		double coef = pow(2.0, i);
		n += (noise.noise(x*coef*scale, y*coef*scale, 11.5))/coef;
	}
	return n;
}

double	MarbleNoise(int x, int y)
{
	double scale = 0.01;
	double n;

	n =  x * scale + noise.noise(x*scale , y*scale , 11.5);
	for(int i= 0; i < 20; i++){
		double coef = pow(2.0, i);
		n += (noise.noise(x*coef*scale + 100, y*coef*scale + 100, 11.5 + 100))/coef;
	}
	n = sin(n);
	return n;
}

double	PlainNoise(int x, int y)
{
	double scale = .02;

	double n;
	
	n =  noise.noise(x*scale , y*scale , 11.5)*2.0;
	 n = n - (int)n;
	
	return n;
}

// Creates noise-based texture objects
void CreateTextures()
{
	int i,j, ix;
	double n;

	for (i=0 ;  i < TexHeight;  i++){
		for (j=0;  j < TexWidth; j++)  
		{
			n = FireNoise(i,j);	
			ix = (GLubyte)((1-n)*255);
			textureImage[i][j][0] = (GLubyte) (n * 0);
			textureImage[i][j][1] = (GLubyte) (n * 255);
			textureImage[i][j][2] = (GLubyte) (n * 255);

		}
	}
}




// Calulates per-face normals for the mesh passed in
Mesh* fnorms(Mesh* model)
{
	vector<Vec3f> v = model->m_v;
	vector<int> vi = model->m_vi;
	
	//For each group of three vertices, determine a cross product and normalize
	for (unsigned int i = 0; i < vi.size(); i = i + 3)
	{
		model->f_n.push_back(((v[vi[i + 1]] - v[vi[i]]).cross(v[vi[i + 2]] - v[vi[i]])).normalize());
	}
	return model;
}

// Create per-vertex normals for the mesh passed in
Mesh* vnorms(Mesh* model)
{
	vector<Vec3f> v = model->m_v;
	vector<int> vi = model->m_vi;
	vector<Vec3f> f_n = model->f_n;
	vector<int> adjTriVerts;

	model->v_ni = vector<int>(vi.size(), 0);

	int index = 0;

	// Loop through each triangle, and compare to all other triangles
	for (unsigned int triangleA = 0; triangleA < vi.size(); triangleA = triangleA + 3)
	{
		int triANormIndex = triangleA / 3;
		Vec3f normalA(f_n.at(triANormIndex));
		int adjacentCount = 0;

		// Push the vertices of the first face
		adjTriVerts.push_back(triangleA);
		adjTriVerts.push_back(triangleA + 1);
		adjTriVerts.push_back(triangleA + 2);

		for (unsigned int triangleB = triangleA + 3; triangleB < vi.size(); triangleB = triangleB + 3)
		{
			int triBNormIndex = triangleB / 3;
			int matches = 0;

			for(unsigned int triAVertex = triangleA; triAVertex < triangleA + 3; triAVertex++)
			{
				for(unsigned int triBVertex = triangleB; triBVertex < triangleB + 3; triBVertex++)
				{
					if(vi[triAVertex] == vi[triBVertex]) // The triangles share a vertex
					{
						matches++;
						if(matches == 2){
							//we know the polys are adj, so stop looking
							triAVertex = triangleA +3;
							triBVertex = triangleB + 3;
						}
					}
				}
			}

			if(matches == 2)
			{
				Vec3f normalB(f_n.at(triBNormIndex));
				float dot = normalA.dot(normalB);
				float edgeAngle = abs(acos(dot)*180/PI);  // Find the angle between the faces

				if(edgeAngle < creaseAngle)
				{
					adjTriVerts.push_back(triangleB); // If it's a smooth edge, push the vertices of the second face
					adjTriVerts.push_back(triangleB + 1);
					adjTriVerts.push_back(triangleB + 2);
					normalA += normalB; // Sum of the per-face normals.
				}
				adjacentCount++;

				if(adjacentCount == 4) // Only checking for three adjacent triangles... lazy, I know
					triangleB = vi.size();
			}

		}

		Vec3f vertexNorm = normalA.normalize(); // Normalize vertex normals
		model->v_n.push_back(vertexNorm); // 
		index = model->v_n.size() - 1;

		for(unsigned int i = 0; i < adjTriVerts.size(); i++)
			model->v_ni[adjTriVerts.at(i)] = index;  // Push the index to all vertices
		
		adjTriVerts.clear();
	
	}
	return model;
}

// Calculating per-vertex normals for each mesh is costly, taking over two minutes. 
// This will calculate one vertex normal per mesh each time the mesh is drawn, until all normals are calculated.

Mesh* iterativeVNorms(Mesh* model, int index)
{
	vector<Vec3f> v = model->m_v;
	vector<int> vi = model->m_vi;
	vector<Vec3f> f_n = model->f_n;
	vector<int> adjTriVerts;

	if(model->v_ni.size() == 0)
		model->v_ni = vector<int>(vi.size(), 0);

	int triANormIndex = index / 3;
	Vec3f normalA(f_n.at(triANormIndex));
	int adjacentCount = 0;

	adjTriVerts.push_back(index);
	adjTriVerts.push_back(index + 1);
	adjTriVerts.push_back(index + 2);

	for (unsigned int triangleB = index + 3; triangleB < vi.size(); triangleB = triangleB + 3)
	{
		int triBNormIndex = triangleB / 3;
		int matches = 0;

		for(unsigned int triAVertex = index; triAVertex < index + 3; triAVertex++)
		{
			for(unsigned int triBVertex = triangleB; triBVertex < triangleB + 3; triBVertex++)
			{
				if(vi[triAVertex] == vi[triBVertex])
				{
					matches++;
					if(matches == 2){
						//we know the polys are adj, so stop looking
						triAVertex = index +3;
						triBVertex = triangleB + 3;
					}
				}
			}
		}

		if(matches == 2)
		{
			Vec3f normalB(f_n.at(triBNormIndex));
			float dot = normalA.dot(normalB);
			float edgeAngle = abs(acos(dot)*180/PI);

			if(edgeAngle < creaseAngle)
			{
				adjTriVerts.push_back(triangleB);
				adjTriVerts.push_back(triangleB + 1);
				adjTriVerts.push_back(triangleB + 2);
				normalA += normalB;
			}
			adjacentCount++;

			if(adjacentCount == 4)
				triangleB = vi.size();
		}

	}

	Vec3f vertexNorm = normalA.normalize();
	model->v_n.push_back(vertexNorm);
	index = model->v_n.size() - 1;

	for(unsigned int i = 0; i < adjTriVerts.size(); i++)
		model->v_ni[adjTriVerts.at(i)] = index;  // Push the index to all vertices
	
	adjTriVerts.clear();

	return model;
}


/* 
 * Draw the model in immediate mode
 */
Mesh* load_model(const char* name)
{
	model = ObjLoader::load(name);
	model = fnorms(model);
	//model = vnorms(model);
	return model;
}

void draw_model(Mesh* model, GLfloat diffuse[], GLfloat specular[], GLfloat shininess)
{
	//srand((unsigned)time(0)); 
	//float rand_float_1, rand_float_2, rand_float_3;
	
	// data references for convenience purpos
	vector<int>& vi = model->m_vi;
	vector<int>& ni = model->m_ni;
	vector<int>& ti = model->m_ti;
	vector<Vec3f>& v = model->m_v;
	vector<Vec3f>& n = model->m_n;
	vector<Vec3f>& fn = model->f_n;
	vector<Vec3f>& vn = model->v_n;
	vector<int>& vni = model->v_ni;
	vector<Vec2f>& t = model->m_texc;

	Vec2f groundTexCoords[3][3] = {Vec2f(0.0, 0.25), Vec2f(0.25, 0.75), Vec2f(0.5, 1.0),
						 Vec2f(0.0, 0.0), Vec2f(0.5, 1.0), Vec2f(1.0, 0.0),
						 Vec2f(0.25, 0.0), Vec2f(0.5, 0.5), Vec2f(0.75, 0.0) };

	// drawing in immediate mode
	glBegin(GL_TRIANGLES);

	int nx = 0;
	int textRow = 0;

	// Calculate one vertex normal for the mesh if they aren't all calculated yet.
	if(vn.size() * 3 < vi.size())
		model = iterativeVNorms(model, vn.size() * 3);

	for(unsigned int i = 0;i < vi.size();i++)
	{
		Vec3f offset = (v[vi[i]] - bbxCenter);

		if(norm_mode == 0 || vn.size() * 3 < vi.size()) // Draw face normals
		{
			glNormal3fv(&fn[nx].x);
			if((i + 1) % 3==0)
				nx++;
		}
		else
		{
			glNormal3fv(&vn[vni[nx]].x); // Draw vertex normals
			nx++;
		}
	
		// Material parameters that were passed in
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);

		// draw txture coordinates
		if (model == cylinderMesh && i >= model->m_vi.size() / 2) // Don't texture top or bottom of cylinder
			glTexCoord2fv(&t[ti[i - model->m_vi.size() / 2]].x);
		else if(model == cubeMesh)
			glTexCoord2fv(&t[ti[i]].x);
		else if(model == groundMesh)
		{
			int j = i % 3; // Make the textures a little more diverse
			glTexCoord2fv(&groundTexCoords[textRow][j].x);
			if(j == 2)
				if(textRow == 2)
					textRow = 0;
				else
					textRow++;
		}
		glVertex3fv(&v[vi[i]].x);
	}

	glEnd();


	// Draw normals if normal visualisation is on
	if(visual_norms == true)
	{
		glDisable(GL_TEXTURE_2D);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorGreen);
		glMaterialfv(GL_FRONT, GL_SPECULAR, colorGreen);
		glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);

		glBegin(GL_LINES);
		nx = 0;

		if(norm_mode == 0 || vn.size() * 3 < vi.size()) // For face normals
			for(unsigned int i = 0;i < vi.size(); i = i + 3)
			{		
				// Calculate base of normal
				Vec3f base = v[vi[i]] + v[vi[i+1]] + v[vi[i+2]];
				base /= 3;
				glVertex3fv(&base.x);
				Vec3f tip;

				// Scaling these models scales their normals as well, this reduces them to a manageable size.
				if(model == alMesh)
					tip = fn[nx] * .1;
				else if(model == asuMesh)
					tip = fn[nx] * .05;
				else if(model == dragonMesh)
					tip = n[ni[i]] * .02;
				else
					tip = fn[nx] * 5;

				//Draw the normal
				Vec3f norm = base + tip;
				glVertex3fv(&norm.x);
				nx++;
			}
		else // for vertex normals
			for(unsigned int i = 0;i < vi.size(); i++)
			{	
				Vec3f base = v[vi[i]];
				glVertex3fv(&base.x);

				Vec3f tip;

				if(model == alMesh)
					tip = vn[vni[i]] * .1;
				else if(model == asuMesh)
					tip = vn[vni[i]] * .05;
				else if(model == dragonMesh)
					tip = vn[vni[i]] * .02;
				else
					tip = vn[vni[i]] * 5;

				Vec3f norm = base + tip;
				glVertex3fv(&norm.x);
				nx++;
			}

		glEnd();
		glEnable(GL_TEXTURE_2D);
	}

	
}

void renderBitmapString(float x, float y, float z, char *string)
{
   char *c;
   
   glRasterPos3f(x, y, z);   // fonts position
   for(c=string; *c != '\0'; c++)
     glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
}

void shut()
{
	if(carMesh) delete carMesh;
	if(alMesh) delete alMesh;
	if(dragonMesh) delete dragonMesh;
	if(asuMesh) delete asuMesh;
}

// Drawing functions
void drawAl(void)
{
	/*GLfloat diffuse[] = {0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat specHigh[] = {1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specLow[] = colorBlack;*/
	GLfloat shininess = 80.0f;
	

	switch(mat_mode)
	{
		case 0: draw_model(alMesh, colorBlue, colorBlack, 128.0f);
		break;
		
		case 1: draw_model(alMesh, colorBlue, colorWhite, shininess);
		break;

		case 2: draw_model(alMesh, colorBlue, colorWhite, shininess);
		break;
	}
}

void drawDragon(void)
{
	/*GLfloat diffuse[] = colorGreen;
	GLfloat specHigh[] = colorWhite;
	GLfloat specLow[] = colorBlack;*/
	GLfloat shininess = 100.0f;
		
	switch(mat_mode)
	{
		case 0: draw_model(dragonMesh, colorGray, colorBlack, 128.0f);
		break;
		
		case 1: draw_model(dragonMesh, colorGray, colorWhite, shininess);
		break;

		case 2: draw_model(dragonMesh, colorGray, colorWhite, shininess);
		break;
	}
}

void drawCar(void)
{
	/*GLfloat diffuse[] = colorRed;
	GLfloat specHigh[] = colorWhite;
	GLfloat specLow[] = colorBlack;*/
	GLfloat shininess = 100.0f;
	
	switch(mat_mode)
	{
		case 0: draw_model(carMesh, colorRed, colorBlack, 128.0f);
		break;
		
		case 1: draw_model(carMesh, colorRed, colorWhite, shininess);
		break;

		case 2: draw_model(carMesh, colorRed, colorWhite, shininess);
		break;
	}
}

void drawASU(void)
{
	/*GLfloat diffuse[] = colorMaroon;
	GLfloat specHigh[] = colorGold;
	GLfloat specLow[] = colorBlack;*/
	GLfloat shininess = 0.0f;
	
	switch(mat_mode)
	{
		case 0: draw_model(asuMesh, colorMaroon, colorBlack, 128.0f);
		break;
		
		case 1: draw_model(asuMesh, colorMaroon, colorGold, shininess);
		break;

		case 2: draw_model(asuMesh, colorMaroon, colorGold, shininess);
		break;

		
	}
}
void drawGround(void)
{
	GLfloat diffuse[] = {.255, .250, .240, 1.0 };
	GLfloat specHigh[] = {1, 1, 1, 1.0};
	GLfloat specLow[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat shininess = 0.0f;
	GLfloat designDiffuse[] = {.3, .3, .3, 1.0};
	GLfloat designSpec[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat designShiny = 128.0f;

	//if(matDiffuse)
	//if(display_lists_off)
		switch(mat_mode)
		{
			case 0: draw_model(groundMesh, diffuse, specLow, 128.0f);
			break;
			
			case 1: draw_model(groundMesh, diffuse, specHigh, shininess);
			break;

			case 2: draw_model(groundMesh, designDiffuse, designSpec, designShiny);
				break;
		}
	//else
	//	glCallList(groundList);
}

void drawCube()
{
	GLfloat diffuse[] = { .945, .5, .5, 1.0 };
	GLfloat specHigh[] = {.945, .5, .5, 1.0};
	GLfloat specLow[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat shininess = 50.0f;

	GLfloat designDiffuse[] = {.6, .3, .3, 1.0};
	GLfloat designSpec[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat designShiny = 128.0f;

	
	if(display_lists_off)
		switch(mat_mode)
		{
			case 0: draw_model(cubeMesh, diffuse, specLow, 128.0f);
			break;
			
			case 1: draw_model(cubeMesh, diffuse, specHigh, shininess);
			break;

			case 2: draw_model(cubeMesh, designDiffuse, designSpec, designShiny);
				break;
		}
	else
		glCallList(cubeList);
}

void drawCylinder(void)
{
	GLfloat diffuse[] = { .945, .945, .945, 1.0 };
	GLfloat specHigh[] = {.945, .945, .945, 1.0};
	GLfloat specLow[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat shininess = 0.0f;

	GLfloat designDiffuse[] = {.5, .5, .5, 1.0};
	GLfloat designSpec[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat designShiny = 128.0f;

	if(display_lists_off)
		switch(mat_mode)
		{
			case 0: draw_model(cylinderMesh, diffuse, specLow, 128.0f);
			break;
			
			case 1: draw_model(cylinderMesh, diffuse, specHigh, shininess);
			break;

			case 2: draw_model(cylinderMesh, designDiffuse, designSpec, designShiny);
				break;
		}
	else
		glCallList(cylinderList);
}



//Ground mesh and display list
void genGroundMesh(void)
{
	groundMesh = new Mesh;

	for(int i = 0; i <= 20; i++)
		for(int j = 0; j <= 20; j++)
			groundMesh->m_v.push_back(Vec3f(i * 200 - 2000, j * 200 - 2000, 0));

	/*groundMesh->m_texc.push_back(Vec2f(0,0));
	groundMesh->m_texc.push_back(Vec2f(0,1));
	groundMesh->m_texc.push_back(Vec2f(1,1));
	groundMesh->m_texc.push_back(Vec2f(1,0));*/

	for(int n = 0; n < 20; n++)
		for(int i = 0; i < 20; i++)
		{
			groundMesh->m_vi.push_back(i + (21 * n));
			groundMesh->m_vi.push_back(i + (21 * n) + 20 + 1);
			groundMesh->m_vi.push_back(i + (21 * n) + 1);
			groundMesh->m_vi.push_back(i + (21 * n) + 1);
			groundMesh->m_vi.push_back(i + (21 * n) + 20 + 1);
			groundMesh->m_vi.push_back(i + (21 * n) + 20 + 2);

			groundMesh->m_ti.push_back(0);
			groundMesh->m_ti.push_back(3);
			groundMesh->m_ti.push_back(1);
			groundMesh->m_ti.push_back(1);
			groundMesh->m_ti.push_back(3);
			groundMesh->m_ti.push_back(2);
		}

	groundMesh = fnorms(groundMesh);
	groundMesh = vnorms(groundMesh);
}


void genGroundList(void)
{
	groundList = glGenLists (1);

	glNewList (groundList, GL_COMPILE);
		glBegin(GL_TRIANGLES);
			for(int i = 0; i <= 40; i++)
				for(int j = 0; j <= 40; j++)
				{
					glVertex3f(i * 100 - 2000, j * 100 - 2000, 0);
					glVertex3f(i * 100 - 2000, j + 1 * 100 - 2000, 0);
					glVertex3f(i + 1 * 100 - 2000, j * 100 - 2000, 0);

					glVertex3f(i * 100 - 2000, j + 1 * 100 - 2000, 0);
					glVertex3f(i + 1 * 100 - 2000, j * 100 - 2000, 0);
					glVertex3f(i + 1 * 100 - 2000, j + 1 * 100 - 2000, 0);
				}
		glEnd();
	glEndList();
}



//Cylinder mesh and display list
void genCylinderMesh(void)
{
	cylinderMesh = new Mesh;

	float angle;
	float radius = 50;
	float height = 100;
	int vertex_count = 25;
	Vec3f top_middle = Vec3f(0,0,height);
	Vec3f bot_middle = Vec3f(0, 0, 0);

	for(int i = 0; i < vertex_count; i++)
	{
		angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
		cylinderMesh->m_v.push_back(Vec3f(radius * cos(angle), radius * sin(angle), height));
		cylinderMesh->m_texc.push_back(Vec2f((float)i / vertex_count, 1));
	}

	for(int i = 0; i < vertex_count; i++)
	{
		angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
		cylinderMesh->m_v.push_back(Vec3f(radius * cos(angle), radius * sin(angle), 0));
		cylinderMesh->m_texc.push_back(Vec2f((float)i / vertex_count, 0));
	}
	cylinderMesh->m_texc.push_back(Vec2f(1, 1));
	cylinderMesh->m_texc.push_back(Vec2f(1, 0));
	cylinderMesh->m_v.push_back(top_middle);
	cylinderMesh->m_v.push_back(bot_middle);

	//Draw top of cylinder
	for(int i = 0 ; i < vertex_count - 1; i++) 
	{
		cylinderMesh->m_vi.push_back(vertex_count * 2);
		cylinderMesh->m_vi.push_back(i);
		cylinderMesh->m_vi.push_back(i + 1);

	}
	cylinderMesh->m_vi.push_back(vertex_count * 2);
	cylinderMesh->m_vi.push_back(vertex_count - 1);
	cylinderMesh->m_vi.push_back(0);

	//Draw bottom of cylinder
	for(int i = 0; i < vertex_count - 1; i++)
	{
		cylinderMesh->m_vi.push_back(vertex_count * 2 + 1);
		cylinderMesh->m_vi.push_back(i + vertex_count + 1);
		cylinderMesh->m_vi.push_back(i + vertex_count);
	}
	cylinderMesh->m_vi.push_back(vertex_count * 2 + 1);
	cylinderMesh->m_vi.push_back(vertex_count);
	cylinderMesh->m_vi.push_back(vertex_count * 2 - 1);

	//Draw side(?) of cylinder

	for(int i = 0; i < vertex_count - 1; i++)
	{
		cylinderMesh->m_vi.push_back(i);
		cylinderMesh->m_vi.push_back(i + vertex_count);
		cylinderMesh->m_vi.push_back(i + vertex_count + 1);

		cylinderMesh->m_vi.push_back(i);
		cylinderMesh->m_vi.push_back(i + vertex_count + 1);
		cylinderMesh->m_vi.push_back(i + 1);

		cylinderMesh->m_ti.push_back(i);
		cylinderMesh->m_ti.push_back(i + vertex_count);
		cylinderMesh->m_ti.push_back(i + vertex_count + 1);

		cylinderMesh->m_ti.push_back(i);
		cylinderMesh->m_ti.push_back(i + vertex_count + 1);
		cylinderMesh->m_ti.push_back(i + 1);
	}

	cylinderMesh->m_vi.push_back(vertex_count - 1);
	cylinderMesh->m_vi.push_back(vertex_count * 2 - 1);
	cylinderMesh->m_vi.push_back(vertex_count);

	cylinderMesh->m_vi.push_back(vertex_count - 1);
	cylinderMesh->m_vi.push_back(vertex_count);
	cylinderMesh->m_vi.push_back(0);

	cylinderMesh->m_ti.push_back(vertex_count - 1);
	cylinderMesh->m_ti.push_back(vertex_count * 2);
	cylinderMesh->m_ti.push_back(vertex_count * 2 - 1);

	cylinderMesh->m_ti.push_back(vertex_count - 1);
	cylinderMesh->m_ti.push_back(vertex_count * 2 + 1);
	cylinderMesh->m_ti.push_back(vertex_count * 2);

	cylinderMesh = fnorms(cylinderMesh);
	cylinderMesh = vnorms(cylinderMesh);
}

void genCylinderList(void)
{
	cylinderList = glGenLists (1);

	float angle = 0;
	float radius = 75;
	float height = 100;
	int vertex_count = 10;
	Vec3f top_middle = Vec3f(0,0,height);
	Vec3f bot_middle = Vec3f(0, 0, 0);

	glNewList (cylinderList, GL_COMPILE);
		glBegin(GL_TRIANGLES);
			for(int i = 0; i < vertex_count; i++)
			{
				glVertex3f(0, 0, height);
				glVertex3f(radius * cos(angle), radius * sin(angle), height);
				angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
				glVertex3f(radius * cos(angle), radius * sin(angle), height);
			}
			glVertex3f(0, 0, height);
			glVertex3f(radius * cos(angle), radius * sin(angle), height);
			angle = 0;
			glVertex3f(radius * cos(angle), radius * sin(angle), height);

			for(int i = 0; i < vertex_count; i++)
			{
				glVertex3f(0, 0, 0);
				glVertex3f(radius * cos(angle), radius * sin(angle), 0);
				angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
				glVertex3f(radius * cos(angle), radius * sin(angle), 0);
			}
			glVertex3f(0, 0, 0);
			glVertex3f(radius * cos(angle), radius * sin(angle), 0);
			angle = 0;
			glVertex3f(radius * cos(angle), radius * sin(angle), 0);

			for(int i = 0; i < vertex_count - 1; i++)
			{
				glVertex3f(radius * cos(angle), radius * sin(angle), height);
				glVertex3f(radius * cos(angle), radius * sin(angle), 0);
				angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
				glVertex3f(radius * cos(angle), radius * sin(angle), 0);
			}

			glVertex3f(radius * cos(angle), radius * sin(angle), height);
			glVertex3f(radius * cos(angle), radius * sin(angle), 0);
			angle = 0;
			glVertex3f(radius * cos(angle), radius * sin(angle), 0);

			for(int i = 0; i < vertex_count - 1; i++)
			{
				glVertex3f(radius * cos(angle), radius * sin(angle), height);
				angle = ((float)i*360.0) / vertex_count  * PI / 180.0f;
				glVertex3f(radius * cos(angle), radius * sin(angle), height);
				glVertex3f(radius * cos(angle), radius * sin(angle), 0);
			}

			glVertex3f(radius * cos(angle), radius * sin(angle), height);
			angle = 0;
			glVertex3f(radius * cos(angle), radius * sin(angle), height);
			glVertex3f(radius * cos(angle), radius * sin(angle), 0);


		glEnd();	
	glEndList();
}


//Cube mesh and display list
void genCubeMesh(void)
{
	cubeMesh = new Mesh;
	float diameter = 100;
	float angle = 45.0 * PI / 180.0;
	do
	{
		cubeMesh->m_v.push_back(Vec3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle), sqrt(pow(diameter / 2, 2) * 2) * sin(angle), diameter));
		angle = angle + 90.0 * PI / 180.0;
	}while(angle < 360.0 * PI / 180.0);

	angle = 45.0 * PI / 180.0;
	do
	{
		cubeMesh->m_v.push_back(Vec3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle), sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 0));
		angle = angle + 90.0 * PI / 180.0;
	}while(angle < 360.0 * PI / 180.0);

	cubeMesh->m_texc.push_back(Vec2f(0, .333));
	cubeMesh->m_texc.push_back(Vec2f(.25, .333));
	cubeMesh->m_texc.push_back(Vec2f(.25, .666));
	cubeMesh->m_texc.push_back(Vec2f(0, .666));
	
	cubeMesh->m_texc.push_back(Vec2f(.25, .333));
	cubeMesh->m_texc.push_back(Vec2f(.5, .333));
	cubeMesh->m_texc.push_back(Vec2f(.5, .666));
	cubeMesh->m_texc.push_back(Vec2f(.25, .666));
	
	cubeMesh->m_texc.push_back(Vec2f(.5, .333));
	cubeMesh->m_texc.push_back(Vec2f(.75, .333));
	cubeMesh->m_texc.push_back(Vec2f(.75, .666));
	cubeMesh->m_texc.push_back(Vec2f(.5, .666));
	
	cubeMesh->m_texc.push_back(Vec2f(.75, .333));
	cubeMesh->m_texc.push_back(Vec2f(1, .333));
	cubeMesh->m_texc.push_back(Vec2f(1, .666));
	cubeMesh->m_texc.push_back(Vec2f(.75, .666));
	
	cubeMesh->m_texc.push_back(Vec2f(.25, 0));
	cubeMesh->m_texc.push_back(Vec2f(.5, 0));
	cubeMesh->m_texc.push_back(Vec2f(.5, .333));
	cubeMesh->m_texc.push_back(Vec2f(.25, .333));
	
	cubeMesh->m_texc.push_back(Vec2f(.25, .666));
	cubeMesh->m_texc.push_back(Vec2f(.5, .666));
	cubeMesh->m_texc.push_back(Vec2f(.5, 1));
	cubeMesh->m_texc.push_back(Vec2f(.25, 1));


	for(int i = 0; i < 3; i++)
	{
		cubeMesh->m_vi.push_back(i);
		cubeMesh->m_vi.push_back(i + 4);
		cubeMesh->m_vi.push_back(i + 1);
		cubeMesh->m_vi.push_back(i + 1);
		cubeMesh->m_vi.push_back(i + 4);
		cubeMesh->m_vi.push_back(i + 5);
	}

	cubeMesh->m_vi.push_back(3);
	cubeMesh->m_vi.push_back(7);
	cubeMesh->m_vi.push_back(0);
	cubeMesh->m_vi.push_back(0);
	cubeMesh->m_vi.push_back(7);
	cubeMesh->m_vi.push_back(4);

	cubeMesh->m_vi.push_back(1);
	cubeMesh->m_vi.push_back(2);
	cubeMesh->m_vi.push_back(0);
	cubeMesh->m_vi.push_back(0);
	cubeMesh->m_vi.push_back(2);
	cubeMesh->m_vi.push_back(3);

	cubeMesh->m_vi.push_back(4);
	cubeMesh->m_vi.push_back(7);
	cubeMesh->m_vi.push_back(5);
	cubeMesh->m_vi.push_back(5);
	cubeMesh->m_vi.push_back(7);
	cubeMesh->m_vi.push_back(6);

	for (int i = 0; i < 6; i++)
	{
		cubeMesh->m_ti.push_back(4 * i + 3);
		cubeMesh->m_ti.push_back(4 * i + 0);
		cubeMesh->m_ti.push_back(4 * i + 2);
		cubeMesh->m_ti.push_back(4 * i + 2);
		cubeMesh->m_ti.push_back(4 * i + 0);
		cubeMesh->m_ti.push_back(4 * i + 1);
	}

	

	cubeMesh = fnorms(cubeMesh);
	cubeMesh = vnorms(cubeMesh);
}

void genCubeList(void)
{
	cubeList = glGenLists (1);

	float angle;
	float diameter = 100;

	glNewList (cubeList, GL_COMPILE);
		glBegin(GL_TRIANGLES);
			
			for(int i = 0; i < 2; i++)
			{
				angle = 45.0 * PI / 180.0;
				for(int j = 0; j < 2; j++)
				{
					glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), diameter * i);
					 
					for(int k = 0; k < 2; k++)
					{
						angle = angle + 90.0 * PI / 180.0;
						glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), diameter * i);
					}
				}	
			}

			angle = 45.0 * PI / 180.0;
			for(int i = 0; i < 4; i++)
			{
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 100);
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 0);
				angle = angle + 90.0 * PI / 180.0;
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 0);
				angle = angle - 90.0 * PI / 180.0;
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 100);
				angle = angle + 90.0 * PI / 180.0;
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 100);
				glVertex3f(sqrt(pow(diameter / 2, 2) * 2) * cos(angle),sqrt(pow(diameter / 2, 2) * 2) * sin(angle), 0);
			}


		glEnd();	
	glEndList();
}


	
void crazyCube(void)
{
	myCube = glGenLists (1);

	glNewList (myCube, GL_COMPILE);
		glBegin(GL_QUADS);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(-50.0f, -50.0f, -50.0f);
			glColor3f(0.0, 0.0, 0.0);
			glVertex3f(-50.0f, 50.0f, -50.0f);
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(-50.0f, 50.0f, 50.0f);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(-50.0f, -50.0f, 50.0f);
			
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(-50.0f, -50.0f, 50.0f);
			glColor3f(1.0, 1.0, 0.0);
			glVertex3f(50.0f, -50.0f, 50.0f);
			glColor3f(0.0, 1.0, 1.0);
			glVertex3f(50.0f, 50.0f, 50.0f);
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(-50.0f, 50.0f, 50.0f);

			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(-50.0f, 50.0f, 50.0f);
			glColor3f(0.0, 1.0, 1.0);
			glVertex3f(50.0f, 50.0f, 50.0f);
			glColor3f(1.0, 0.0, 1.0);
			glVertex3f(50.0f, 50.0f, -50.0f);
			glColor3f(0.0, 0.0, 0.0);
			glVertex3f(-50.0f, 50.0f, -50.0f);

			glColor3f(0.0, 1.0, 1.0);
			glVertex3f(50.0f, 50.0f, 50.0f);
			glColor3f(1.0, 1.0, 0.0);
			glVertex3f(50.0f, -50.0f, 50.0f);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(50.0f, -50.0f, -50.0f);
			glColor3f(1.0, 0.0, 1.0);
			glVertex3f(50.0f, 50.0f, -50.0f);

			glColor3f(1.0, 0.0, 1.0);
			glVertex3f(50.0f, 50.0f, -50.0f);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(50.0f, -50.0f, -50.0f);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(-50.0f, -50.0f, -50.0f);
			glColor3f(0.0, 0.0, 0.0);
			glVertex3f(-50.0f, 50.0f, -50.0f);

			glColor3f(1.0, 1.0, 0.0);
			glVertex3f(50.0f, -50.0f, 50.0f);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(50.0f, -50.0f, -50.0f);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(-50.0f, -50.0f, -50.0f);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(-50.0f, -50.0f, 50.0f);
		glEnd();	
		
	glEndList();

}

void square(void) 
{
	mySquare = glGenLists (1);

	glNewList (mySquare, GL_COMPILE);

	glBegin(GL_LINE_STRIP);
		glVertex3f(-100, -100, 0);
		glVertex3f(-100, 100, 0);
		glVertex3f(100, 100, 0);
		glVertex3f(100, -100, 0);
		glVertex3f(-100, -100, 0);

	glEnd();

	glEndList();
}

void recalculateNorms()
{
	cubeMesh = vnorms(cubeMesh);
	cylinderMesh = vnorms(cylinderMesh);
	groundMesh = vnorms(groundMesh);

}
	


//Menus
void shadingMenu(int menuId)
{
	if(menuId == 0)
		glShadeModel(GL_FLAT);
	else if(menuId == 1)
		glShadeModel(GL_SMOOTH);
	
	glutPostRedisplay();
}

void displayListMenu(int menuId)
{
	if(menuId == 0)
		display_lists_off = false;
	else if(menuId == 1)
		display_lists_off = true;

	glutPostRedisplay();
}

void viewMenu(int menuId)
{
	view_mode = menuId;

	if(view_mode == 1)
	{
		speedLimit = 10.0f;
		if(speed > speedLimit)
			speed = speedLimit;
	}
	else
		speedLimit = 30.0f;
	
	glutPostRedisplay();
}

void textureMenu(int menuId)
{
	if(menuId == 0)
		textureMode = GL_REPLACE;
	else if(menuId == 1)
		textureMode = GL_MODULATE;
	else if(menuId == 2)
		texturesOn = true;
	else if(menuId == 3)
		texturesOn = false;
}

void filterMenu(int menuId)
{
	
}

void minificationMenu(int menuId)
{
	if(menuId==0)
		minMode = GL_NEAREST;
	else 
		minMode = GL_LINEAR_MIPMAP_LINEAR;
}

void magnificationMenu(int menuId)
{
	if(menuId==0)
		magMode = GL_NEAREST;
	else
		magMode = GL_LINEAR;
}

void materialsMenu(int menuId)
{
	mat_mode = menuId;
}

void creaseAngleMenu(int menuId)
{
	switch(menuId)
	{
		case 0:
			creaseAngle = 5;
			break;
	
		case 1:
			creaseAngle = 15;
			break;

		case 2:
			creaseAngle = 25;
			break;

		case 3:
			creaseAngle = 35;
			break;

		case 4:
			creaseAngle = 180;
			break;
	}

	// Changing the crease angle means we need to recalculate vertex normals
	// This will clear all vertex normals so they can be gradually calculated again.

	carMesh->v_n.clear();
	carMesh->v_ni.clear();
	asuMesh->v_n.clear();
	asuMesh->v_ni.clear();
	dragonMesh->v_n.clear();
	dragonMesh->v_ni.clear();
	alMesh->v_n.clear();
	alMesh->v_ni.clear();

	recalculateNorms(); // Recalc norms for cube, cylinder, and ground right now
}
//Various menus
void normalMenu(int menuId)
{
	norm_mode = menuId;
}

void navigationMenu(int menuId)
{
	nav_mode = menuId;
}

void normvisualMenu(int menuId)
{
	if(menuId == 0)
		visual_norms = true;
	else
		visual_norms = false;
}

void menu(int menuId)
{
	if(menuId == 0)
	{
		glutDestroyWindow(winId);
		shut();
		exit(0);
		
	}

	glutPostRedisplay();
}

void glutMenus (void)
{	
	GLint shadingSubMenu = glutCreateMenu(shadingMenu);
	glutAddMenuEntry("Flat", 0);
	glutAddMenuEntry("Smooth", 1);

	GLint displayListSubMenu = glutCreateMenu(displayListMenu);
	glutAddMenuEntry("ON", 0);
	glutAddMenuEntry("OFF", 1);

	GLint viewSubMenu = glutCreateMenu(viewMenu);
	glutAddMenuEntry("Top Down", 0);
	glutAddMenuEntry("First-person",1);
	glutAddMenuEntry("Overview", 2);
	glutAddMenuEntry("45", 3);
	
	GLint textureSubMenu = glutCreateMenu(textureMenu);
	glutAddMenuEntry("Replace", 0);
	glutAddMenuEntry("Blend", 1);
	glutAddMenuEntry("On", 2);
	glutAddMenuEntry("Off", 3);

	GLint minificationSubMenu = glutCreateMenu(minificationMenu);
	glutAddMenuEntry("Simple", 0);
	glutAddMenuEntry("Smooth", 1);

	GLint magnificationSubMenu = glutCreateMenu(magnificationMenu);
	glutAddMenuEntry("Simple", 0);
	glutAddMenuEntry("Smooth", 1);

	GLint filterSubMenu = glutCreateMenu(filterMenu);
	glutAddSubMenu("Minification", minificationSubMenu);
	glutAddSubMenu("Magnification", magnificationSubMenu);

	GLint normalSubMenu = glutCreateMenu(normalMenu);
	glutAddMenuEntry("Per Face", 0);
	glutAddMenuEntry("Per Vertex", 1);
	glutAddMenuEntry("Per Vertex, Weighted", 2);

	GLint normvisualSubMenu = glutCreateMenu(normvisualMenu);
	glutAddMenuEntry("On" , 0);
	glutAddMenuEntry("Off", 1);

	GLint creaseAngleSubMenu = glutCreateMenu(creaseAngleMenu);
	glutAddMenuEntry("5",0);
	glutAddMenuEntry("15",1);
	glutAddMenuEntry("25",2);
	glutAddMenuEntry("35",3);
	glutAddMenuEntry("Off",4);

	GLint materialsSubMenu = glutCreateMenu(materialsMenu);
	glutAddMenuEntry("Diffuse",0);
	glutAddMenuEntry("Specular",1);
	glutAddMenuEntry("Design",2);

	GLint navigationSubMenu = glutCreateMenu(navigationMenu);
	glutAddMenuEntry("Camera", 0);
	glutAddMenuEntry("Light Source", 1);
	
	glutCreateMenu(menu);
	glutAddSubMenu("Shading", shadingSubMenu);
	glutAddSubMenu("Display List", displayListSubMenu);
	glutAddSubMenu("View", viewSubMenu);
	glutAddSubMenu("Texture", textureSubMenu);
	glutAddSubMenu("Filter", filterSubMenu);
	glutAddSubMenu("Normals", normalSubMenu);
	glutAddSubMenu("Normal Visualization", normvisualSubMenu);
	glutAddSubMenu("Crease Angle", creaseAngleSubMenu);
	glutAddSubMenu("Materials", materialsSubMenu);
	glutAddSubMenu("Navigation", navigationSubMenu);


	glutAddMenuEntry("Quit", 0);	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void mouse(int button, int state, int x, int y)
{
	// button -- GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON.
	// state  -- GLUT_UP, GLUT_DOWN

	mouse_x = x;
	mouse_y = y;

	mouse_button = button;

}

void motion(int x, int y)
{
	/*if(mouse_button == GLUT_LEFT_BUTTON)
	{
		// compute the angle (0..360) around x axis, and y-axis
		y_angle += (float(x - mouse_x)/width)*360.0;
		x_angle += (float(y - mouse_y)/height)*360.0;

	}

	if(mouse_button == GLUT_RIGHT_BUTTON)
	{
		// scale
		scale += (y-mouse_y)/100.0;
			
		if(scale < 0.1) scale = 0.1;     // too small
		if(scale > 7)	scale = 7;		 // too big

	}

	mouse_x	= x;		// update current mouse position
	mouse_y	= y;
	glutPostRedisplay();*/

}

void init()
{	
	// use black to clean background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// always re-normalize normal (due to scale transform)
	glEnable(GL_NORMALIZE);	
 
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	// Enable lighting
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularLight);
	glEnable(GL_LIGHT0);

	CreateTextures(); // Generate noise texture, set parameters
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TextObj);
	glBindTexture(GL_TEXTURE_2D, TextObj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexWidth, TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA ,TexWidth, TexHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureImage);

	
	// Load dice and bean can textures and set parameters
	BitmapBits = LoadDIBitmap("./data/dice.bmp", &BitmapInfo);

	glGenTextures(1, &DiceObj);
	glGenTextures(1, &BeanObj);
	glBindTexture(GL_TEXTURE_2D, DiceObj);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // must set to 1 for compact data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,			
		0,								// mipmap level
		GL_RGB,							// internal format
		BitmapInfo->bmiHeader.biWidth,	// width
		BitmapInfo->bmiHeader.biHeight,	// height
		0,								// border
		GL_BGR_EXT,						// data format  (NOTE: Bitmap's format is BGR!!!!)
		GL_UNSIGNED_BYTE,				// element per pixel
		BitmapBits);					// data

	BitmapBits = LoadDIBitmap("./data/canOfBeans.bmp", &BitmapInfo);

	glBindTexture(GL_TEXTURE_2D, BeanObj);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // must set to 1 for compact data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,			
		0,								// mipmap level
		GL_RGB,							// internal format
		BitmapInfo->bmiHeader.biWidth,	// width
		BitmapInfo->bmiHeader.biHeight,	// height
		0,								// border
		GL_BGR_EXT,						// data format  (NOTE: Bitmap's format is BGR!!!!)
		GL_UNSIGNED_BYTE,				// element per pixel
		BitmapBits);					// data

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


	// smooth shading
	glShadeModel(GL_SMOOTH);

	crazyCube();
	genGroundMesh();
	genGroundList();
	genCubeMesh();
	genCubeList();
	genCylinderMesh();
	genCylinderList();
	square();

	// load the first model by default
	//load_model(modelfile[0]);
	
	alMesh = load_model (AL);
	dragonMesh = load_model (DRAGON);
	asuMesh = load_model (ASU);
	carMesh = load_model (PORSCHE);

	calc_bbox(bbxMin, bbxMax);
	bbxCenter = (bbxMin+bbxMax)*0.5f;

	// setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	aspect = (double)width/(double)height;
	//farPlane = nearPlane+4.0f*(bbxMax-bbxMin).length();
	gluPerspective(fieldofview, aspect, nearPlane, farPlane);

	// setup viewing matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	gluLookAt(0.0f, 0.0f, 1000.0f, 
		0.0f, 0.0f, 0.0, 
		0.0f, 1.0f, 0.0f);

	// set up menus
	glutMenus();
}

void startClock(void)
{
	finished = false;
	start = clock();
}


//Calculate how to move the car
void calcMovement()
{
	float temp_x;
	float temp_y;
	float temp_carX;
	float temp_carY;
	
	if(up_pressed && !down_pressed)
		if(nav_mode != 1) // Camera Mode
		{	
			if(view_mode != 2 && speed < speedLimit) // Top-down and 45-degree mode
				speed = speed + 1.0;
			else if(view_mode == 2 && ovCameraHeight <= 3000) // overview mode, change camera height
				ovCameraHeight = ovCameraHeight + 10.0; // Raise camera
		}
		else if(nav_mode == 1) // Light mode
		{
			if(l_z < 1000) // Raise light
				l_z = l_z + 50;
		}
	if(down_pressed)
		if(nav_mode != 1)
		{
			if (view_mode == 0 || view_mode == 3)
				if( speed > 5 )
					speed = speed - 5.0;
				else
					speed = 0;
			else if(view_mode == 1 && speed > -speedLimit)
				speed = speed - 1.0;

			else if (view_mode == 2 && ovCameraHeight >= 500)
				ovCameraHeight = ovCameraHeight - 50.0;
		}
		else if(nav_mode == 1)
		{
			if(l_z > 0)
				l_z = l_z - 50;
		}

	if(!up_pressed && !down_pressed && view_mode == 1)
		speed = 0;


	if(left_pressed && !right_pressed) {

		if(nav_mode == 0)
		{
			if(view_mode != 2 && (speed > 0 || view_mode == 1))
			{
				if(rotation == 180.0f)
					rotation = -180.0f + turn_angle;
				else
					rotation = rotation + turn_angle;

				temp_x = x_move;
				temp_y = y_move;
				temp_carX = carPosX;
				temp_carY = carPosY;

				x_move = ( temp_x * cos( -1 * turn_angle * PI / 180.0f ) - temp_y * sin ( -1 * turn_angle * PI / 180.0f ));
				y_move = ( temp_x * sin( -1 * turn_angle * PI / 180.0f ) + temp_y * cos ( -1 * turn_angle * PI / 180.0f ));
			}
			else if(view_mode == 2)
				if(ovCameraRotate <= -179.999)
					ovCameraRotate = 180;
				else 
					ovCameraRotate = ovCameraRotate - .001;
		}	

		else if(nav_mode == 1)
		{
			if(light_rot == -180.0f + 10)
				light_rot = 180.0f - 10;
			else
				light_rot = light_rot - 10;

		}
	}
	
	if(right_pressed && !left_pressed) {
		if(nav_mode == 0)
		{
			if(view_mode != 2 &&(speed > 0 || view_mode == 1))
			{
				if(rotation == -180.0f + turn_angle)
					rotation = 180.0f;
				else
					rotation = rotation - turn_angle;
				
				temp_x = x_move;
				temp_y = y_move;


				x_move = ( temp_x * cos( turn_angle * PI / 180.0f ) - temp_y * sin ( turn_angle * PI / 180.0f ));
				y_move = ( temp_x * sin( turn_angle * PI / 180.0f ) + temp_y * cos ( turn_angle * PI / 180.0f ));
	
			}
			else if(view_mode == 2)
				if(ovCameraRotate >= 180)
					ovCameraRotate = -179.999;
				else 
					ovCameraRotate = ovCameraRotate + 0.001;
				
		}
		else if(nav_mode == 1)
		{
			if(light_rot == 180.0f)
				light_rot = -180.0f + 10;
			else
				light_rot = light_rot + 10;
		}
	}

	if(a_pressed && !s_pressed)
		if(view_mode == 1 && fpCameraHeight <= 1000)
			fpCameraHeight = fpCameraHeight + 50.0;
		else if (view_mode == 2 && ovDistance  <= 1.50)
			ovDistance = ovDistance + .01;
	
	if(s_pressed && !a_pressed)
		if(view_mode == 1 && fpCameraHeight >= 10)
			fpCameraHeight = fpCameraHeight - 50.0;
		else if (view_mode == 2 && ovDistance >= .2)
			ovDistance = ovDistance - .01;

		

	y_move = y_move + speed;

	GLfloat LightPos[]			= {l_x, l_y, l_z, 1.0f};

	// Move car and bounding box
	carPosX = carPosX - sin ( rotation * PI / 180.0f ) * speed;
	bbxMin.x = bbxMin.x + sin ( rotation * PI / 180.0f ) * speed;
	bbxMax.x = bbxMax.x + sin ( rotation * PI / 180.0f ) * speed;
	carPosY = carPosY + cos ( rotation * PI / 180.0f ) * speed;
	bbxMin.y = bbxMin.y + cos ( rotation * PI / 180.0f ) * speed;
	bbxMax.y = bbxMax.y + cos ( rotation * PI / 180.0f ) * speed;

	// Detect if car is in start box
	if(bbxMin.x < 100 && bbxMax.x > -100 && bbxMax.y < -1700 && bbxMin.y > -1900)
		startClock();

	// Detect if car is in finish box
	if((x_move > -80 && x_move < 80 && y_move > -45 && y_move < 45) ||
		(x_move > -45 && x_move < 45 && y_move > -80 && y_move < 80))
	{
		finished = true;
	}
	
	// Rotate models
	if(alRotate == 359)
		alRotate = 0;
	else
		alRotate = alRotate + 1;
	
	if(asuRotate == 359)
		asuRotate = 0;
	else
		asuRotate = asuRotate + 3;
	
	if(dragonRotate == 359)
		dragonRotate = 0;
	else
		dragonRotate = dragonRotate + 3;

	glutPostRedisplay();


}

void display()
	
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//Set up first viewport and projection
	glViewport(0,height * .25,width,height * .75);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldofview, aspect, nearPlane, farPlane);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Position the cameras
	if(view_mode == 0)	// Top Down Camera
		gluLookAt(carPosX, carPosY, 1000.0f, 
					  carPosX, carPosY, 0.0, 
					  0.0f, 1.0f, 0.0f);
	else if(view_mode == 1) // First-person camera
		gluLookAt(carPosX, carPosY, fpCameraHeight,
				  carPosX - sin (rotation * .0003 * 180.0f / PI), carPosY + cos(rotation * .0003 * 180.0f / PI), fpCameraHeight,
				  0, 0, 1);
	else if(view_mode == 2) // Overview camera
		gluLookAt((0.0f * cos(ovCameraRotate * 180.0f / PI) - -2200 * sin(ovCameraRotate * 180.0f / PI)) * ovDistance, (0.0f * sin(ovCameraRotate * 180.0f / PI) + -2200 * cos(ovCameraRotate * 180.0f / PI)) * ovDistance, ovCameraHeight * ovDistance,
				  0.0f, 0.0f, 0.0f,
				 (0.0f * cos(ovCameraRotate * 180.0f / PI) - sin(ovCameraRotate * 180.0f / PI)), (0.0f * sin(ovCameraRotate * 180.0f / PI) + cos(ovCameraRotate * 180.0f / PI)), 0);
	else	// 45-degree camera
		gluLookAt(-1000, -1000, 800,
				  carPosX, carPosY, 0,
				  0, 0, 1);
	glPushMatrix();
		glRotatef(light_rot, 0.0f, 0.0f, 1.0f);
		glTranslatef(l_x, l_y, l_z);
		glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
		//drawCylinder();
	glPopMatrix();

	glPushMatrix();


		glScalef(scale, scale, scale);
		glRotatef(x_angle, 1.0f, 0.0f, 0.0f);
		glRotatef(y_angle, 0.0f, 1.0f, 0.0f);
		//glTranslatef(-bbxCenter.x, -bbxCenter.y, -bbxCenter.z);

		glPushMatrix();
		
			//Draw the car model
			if(view_mode != 1)
			{
				glTranslatef (carPosX, carPosY, z_move);
				glRotatef(90, 1.0f, 0.0f, 0.0f);
				glRotatef(180, 0.0f, 1.0f, 0.0f);
				glRotatef (rotation, 0.0f, 1.0f, 0.0f);
				drawCar();
			}

		glPopMatrix();

		calcMovement();	

		glPushMatrix ();	
			if(texturesOn)
			{
				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textureMode); 
				glBindTexture(GL_TEXTURE_2D, TextObj);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magMode);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMode);
			}

			drawGround();

			glDisable(GL_TEXTURE_2D);
		
			glPushMatrix();
				//Start box
				glColor3f(1.0f, 0.0f, 0.0f);
				glTranslatef(0.0, -1800.0, 1.0f);
				glCallList( mySquare );
			glPopMatrix();

			glPushMatrix();
				// Al Mesh
				glTranslatef(0, 750, 0);
				glPushMatrix();
					glTranslatef(0, 0, 67);
					glScalef(20,20,20);
					glRotatef(90, 1.0, 0.0, 0.0);
					//glRotatef(90, 0.0, 1.0, .0);
					glPushMatrix();
						// Dragon Mesh
						glTranslatef(10 * cos( alRotate * PI / 180.0f ) - 10 * sin ( alRotate * PI / 180.0f ), 0 ,
									10 * sin( alRotate * PI / 180.0f ) + 10 * cos ( alRotate * PI / 180.0f ));
							
						glPushMatrix();
							glRotatef(-alRotate - 25.0, 0.0, 1.0, 0.0);
							drawAl();
						glPopMatrix();
						glPushMatrix();
									glTranslatef(4 * cos( -dragonRotate * PI / 180.0f ) - 4 * sin ( -dragonRotate * PI / 180.0f ), 0 ,
									4 * sin( -dragonRotate * PI / 180.0f ) + 4 * cos ( -dragonRotate * PI / 180.0f ));
							glRotatef(dragonRotate - 45, 0.0, 1.0, 0.0);
							glScalef(4, 4, 4);
							drawDragon();
						glPopMatrix();
					glPopMatrix();
				glPopMatrix();

			glPopMatrix();

			glPushMatrix();
				// ASU Mesh
				glTranslatef(-250, -1450, 0);
				glScalef(100,100,100);
									glPushMatrix();
						glTranslatef(2 * cos( asuRotate * PI / 180.0f ) - 2 * sin (asuRotate * PI / 180.0f ),
									2 * sin(asuRotate * PI / 180.0f ) + 2 * cos (asuRotate * PI / 180.0f ),
									0.0);
						glDisable(GL_TEXTURE_2D);
						drawASU();
					glPopMatrix();
			glPopMatrix();

			/***************
			Lines 1883 through 2127 lay out the map
			***************/
			glPushMatrix();
				if(texturesOn)
				{
					glEnable(GL_TEXTURE_2D);
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textureMode); 
					glBindTexture(GL_TEXTURE_2D, DiceObj);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magMode);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMode);
				}
				
				glTranslatef(0,0,10);
				glColor3f(.6, .6, .6);

				glPushMatrix();
					//Walls at start position
					glTranslatef(-160, -1950, 0);
					for(int i = 0; i < 4; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(320, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}
					
				glPopMatrix();


				glPushMatrix();
					//Long walls outside start position
					glTranslatef(-700, -1700, 0);
					for(int i = 0; i < 10; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(1400, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}
					for(int i = 0; i < 5; i++)
					{
						drawCube();
						glTranslatef(0, 100, 0);
					}

					glPushMatrix();
						//Horizontal blocking wall on left side
						glTranslatef(-400, -600, 0);
						for(int i = 0; i < 4; i++)
						{
							drawCube();
							glTranslatef(100, 0, 0);
						}
						//...And right side
						glTranslatef(1400, 0, 0);
						for(int i = 0; i < 9; i++)
						{
							drawCube();
							glTranslatef(100, 0, 0);
						}
					glPopMatrix();

					//Vertical arms on center box
		
					for(int i = 0; i < 3; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(1400, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}

					//Vertical bars on top wall
					glTranslatef(0, 1600, 0);
					for(int i = 0; i < 4; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(1400, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}
				glPopMatrix();
				
				glPushMatrix();
					//Horizontal wall in front of start area
					glTranslatef (-700, -1200, 0.0);
					glPushMatrix();
						for(int i = 0; i < 13; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Horizontal arms on center box
					glTranslatef (-700, 0, 0.0);
					glPushMatrix();
						for(int i = 0; i < 3; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							drawCube();
						}
					
						glTranslatef (700, 0, 0.0);
					
						for(int i = 0; i < 3; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Center box, top
					glTranslatef (-500, 300, 0.0);
					glPushMatrix();
						for(int i = 0; i < 9; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Center box, bottom
					glTranslatef (-500, -300, 0.0);
					glPushMatrix();
						for(int i = 0; i < 3; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					
						glTranslatef (300, 0, 0.0);
					
						for(int i = 0; i < 3; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Center box, left
					glTranslatef (-400, -300, 0.0);
					glPushMatrix();
						for(int i = 0; i < 6; i++) {
							glTranslatef(0.0, 100.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
					
					//Center box, right
					glTranslatef (800, 0, 0.0);
					glPushMatrix();
						for(int i = 0; i < 6; i++) {
							glTranslatef(0.0, 100.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Long walls on sides
					glTranslatef (-1600, -1500, 0.0);
					glPushMatrix();
						for(int i = 0; i < 27; i++) {
							glTranslatef(0.0, 100.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
					
					glTranslatef (3200, 0, 0.0);
					glPushMatrix();
						for(int i = 0; i < 27; i++) {
							glTranslatef(0.0, 100.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Long wall along top
					glTranslatef (-1600, 1200, 0.0);
					glPushMatrix();
						for(int i = 0; i < 32; i++) {
							glTranslatef(100.0, 0.0, 0.0);
							//glCallList (myCube);
							drawCube();
						}
					glPopMatrix();
				glPopMatrix ();

				glPushMatrix();
					//Longer vertical walls off bottom
					glTranslatef(-1100, -1950, 0);
					for(int i = 0; i < 8; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(2200, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}
					for(int i = 0; i < 4; i++)
					{
						drawCube();
						glTranslatef(0, 100, 0);
					}

					//...and corresponding walls near top
					glTranslatef(0, 2000, 0);
					for(int i = 0; i < 5; i++)
					{
						drawCube();
						glPushMatrix();
							glTranslatef(2200, 0, 0);
							drawCube();
						glPopMatrix();
						glTranslatef(0, 100, 0);
					}
				glPopMatrix();
				
				glPushMatrix();
					//Center top vertical wall
					glTranslatef(0, 1200, 0);
					for(int i = 0; i < 6; i++)
					{
						drawCube();
						glTranslatef(0, 100, 0);
					}
				glPopMatrix();
				
				// Load up bean can texture for cylinders, caclulate new rotation
				glBindTexture(GL_TEXTURE_2D, BeanObj);

				if(cylRotate == 180)
					cylRotate = -178;
				else
					cylRotate = cylRotate + 2;

				glPushMatrix();
					//Left and right cylinders
					glTranslatef(-1150, 0, 0);
					glPushMatrix();
						glRotatef(cylRotate, 0.0, 0.0, 1.0);
						drawCylinder();
					glPopMatrix();
					glTranslatef(2300, 0, 0);
					glPushMatrix();
						glRotatef(cylRotate + 90, 0.0, 0.0, 1.0);
						drawCylinder();
					glPopMatrix();
				glPopMatrix();

				glPushMatrix();
					//Top and bottom cylinders
					glTranslatef(0, 750, 0);
					glPushMatrix();
						glRotatef(cylRotate + 180, 0.0, 0.0, 1.0);
						drawCylinder();
					glPopMatrix();
					glTranslatef(0, -1500, 0);
					glPushMatrix();
						glRotatef(cylRotate + 270, 0.0, 0.0, 1.0);
						drawCylinder();
					glPopMatrix();
				glPopMatrix();
				glDisable(GL_TEXTURE_2D);

				float colorGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorGreen);
				

				glPushMatrix ();
					//Finish Square
					glPushMatrix();
						glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
					glPopMatrix();
					glColor3f(0.0, 1.0, 0.0);
					glCallList( mySquare );
				glPopMatrix ();

			glPopMatrix();

		glPopMatrix ();

	glPopMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);

	// Set up second viewport
	glViewport(0,0,width,height * .25);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fieldofview, aspect * 3, nearPlane, farPlane);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 1000.0f, 
	0.0f, 0.0f, 0.0, 
	0.0f, 1.0f, 0.0f);

	// Get the current time and use it to display the clock
	clock_t time;
	int minutes;
	int seconds;

	if(!finished)
	{	
		time = (clock() - start)/CLOCKS_PER_SEC ;
		minutes = time / 60;
		seconds = time % 60;

		if(seconds < 10) 
			sprintf(timeBuffer,"Time: %d:0%d", minutes, seconds);
		else
			sprintf(timeBuffer,"Time: %d:%d", minutes, seconds);
	}

	glColor3f(1.0, 0.0, 0.0);
	glScalef(5, 5, 5);
	glTranslatef(-350, 90, 0);
	renderBitmapString(0.0f,0.0f, 0.0f, timeBuffer);

	// Display "FINISHED" message
	if(finished)
	{
		glTranslatef(-600, -120, 0);
		switch(colorWheel) {
			case 0:	glColor3f(1.0, 1.0, 0.0);
					colorWheel = 1;
					break;

			case 1:	glColor3f(1.0, 1.0, 0.0);
					colorWheel = 2;
					break;

			case 2:	glColor3f(0.0, 1.0, 0.0);
					colorWheel = 3;
					break;

			case 3:	glColor3f(0.0, 0.0, 1.0);
					colorWheel = 0;
					break;
		}

		renderBitmapString(0.0f,0.0f, 0.0f, "FINISHED!");
	}
	
	glPopMatrix();
	glutSwapBuffers();
}


//Handle pressing of keys
void specialDown(GLint key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			up_pressed = true;
			break;
		case GLUT_KEY_DOWN:
			down_pressed = true;
			break;
		case GLUT_KEY_LEFT:
			left_pressed = true;
			break;
		case GLUT_KEY_RIGHT:
			right_pressed = true;
			break;
		
	}
	calcMovement();

}

//Handle releasing of keys
void specialUp(GLint key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			up_pressed = false;
			break;
		case GLUT_KEY_DOWN:
			down_pressed = false;
			break;
		case GLUT_KEY_LEFT:
			left_pressed = false;
			break;
		case GLUT_KEY_RIGHT:
			right_pressed = false;
			break;
		
	}
	calcMovement();

}

void keyboardDown(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 65:
		case 97:
			a_pressed = true;
			break;
		case 83:
		case 115:
			s_pressed = true;
			break;
		case 108:
			aspect = aspect + .1;
			break;
		case 111:
			aspect = aspect - .1;
			break;
		case 105:
			fieldofview = fieldofview + 1;
			break;
		case 107:
			fieldofview = fieldofview - 1;
			break;
	}
	calcMovement();
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 65:
		case 97:
			a_pressed = false;
			break;
		case 83:
		case 115:
			s_pressed = false;
			break;		
	}
	calcMovement();
}

void reshape(int x, int y)
{
    width  = x;
	height = y;

	if (height == 0)		// not divided by zero
		height = 1;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0,height * .25,width,height * .75);

	aspect = (float)width/(float)height / .75;
	fieldofview = min(105.105 - 29.418 * log (aspect * .75), 180.0);

	gluPerspective(fieldofview, aspect, nearPlane, farPlane);
	
	glMatrixMode(GL_MODELVIEW);

}

void main(int argc, char* argv[])
{
	glutInit(&argc,argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(position_x,position_y);
	glutInitWindowSize(width,height);
	winId = glutCreateWindow("Moriarty Project 1");
	
	glutReshapeFunc(reshape);
	glutSpecialFunc(specialDown);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutDisplayFunc(display);		// display function

	init();

	glutMainLoop();

	shut();
}






