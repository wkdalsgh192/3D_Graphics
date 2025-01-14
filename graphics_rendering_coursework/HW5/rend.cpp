/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

#include	<algorithm>

struct vec3 {
	float x, y, z;

	// Constructor
	vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

	// Operator overloads for vector math
	vec3 operator+(const vec3& other) const {
		return vec3(x + other.x, y + other.y, z + other.z);
	}

	vec3 operator-(const vec3& other) const {
		return vec3(x - other.x, y - other.y, z - other.z);
	}

	vec3& operator+=(const vec3& other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	vec3 operator*(float scalar) const {
		return vec3(x * scalar, y * scalar, z * scalar);
	}

	friend vec3 operator*(float scalar, const vec3& v) {
		return v * scalar;
	}

	// Dot product
	float dot(const vec3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	// Normalize the vector
	vec3 normalize() const {
		float len = sqrt(x * x + y * y + z * z);
		return len > 0 ? vec3(x / len, y / len, z / len) : *this;
	}
};

struct Plane {
	float A, B, C, D;

	// Constructor to calculate plane coefficients given two edges and a vertex
	Plane(vec3 edge0, vec3 edge1, vec3 vertex) {
		A = (edge0.y * edge1.z) - (edge0.z * edge1.y);
		B = -((edge0.x * edge1.z) - (edge0.z * edge1.x));
		C = (edge0.x * edge1.y) - (edge0.y * edge1.x);
		D = -1.0f * (A * vertex.x + B * vertex.y + C * vertex.z);
	}
};

/***********************************************/
/* HW1 methods: copy here the methods from HW1 */

#define RGBA_DIMEMSION	4	/* RGBA -> 4D color */
#define RGB_DIMEMSION	3	/* RGB -> 3D color */
#define COLOR_LIMIT		4095	/* Clamping the color value into 0~4095. */

#define PI (float) 3.14159265358979323846

static short matlevelNormal;	/* Global variable of the head of matrix stack of normals */
int pushMatToStack(short& head, GzMatrix* matStack, GzMatrix mat);
int popMatToStack(short& head, GzMatrix* matStack);
vec3 shadingEquation(GzRender* render, vec3 normal);
void swapArrays(float arrays[3][3], int i, int j);

int GzRender::GzRotXMat(float degree, GzMatrix mat)
{
	/* HW 3.1
	// Create rotate matrix : rotate along x axis
	// Pass back the matrix using mat value
	*/
	float radianAngle = degree * PI / 180.0f;
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			mat[i][j] = 0;
		}
	}
	mat[0][0] = 1.0;
	mat[1][1] = (float)cos((double)radianAngle);
	mat[1][2] = -1.0f * (float)sin((double)radianAngle);
	mat[2][1] = (float)sin((double)radianAngle);
	mat[2][2] = (float)cos((double)radianAngle);
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzRotYMat(float degree, GzMatrix mat)
{
	/* HW 3.2
	// Create rotate matrix : rotate along y axis
	// Pass back the matrix using mat value
	*/
	float radianAngle = degree * PI / 180.0f;
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			mat[i][j] = 0;
		}
	}
	mat[0][0] = (float)cos((double)radianAngle);
	mat[0][2] = (float)sin((double)radianAngle);
	mat[1][1] = 1.0;
	mat[2][0] = -1.0f * (float)sin((double)radianAngle);
	mat[2][2] = (float)cos((double)radianAngle);
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzRotZMat(float degree, GzMatrix mat)
{
	/* HW 3.3
	// Create rotate matrix : rotate along z axis
	// Pass back the matrix using mat value
	*/
	float radianAngle = degree * PI / 180.0f;
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			mat[i][j] = 0;
		}
	}
	mat[0][0] = (float)cos((double)radianAngle);
	mat[0][1] = -1.0f * (float)sin((double)radianAngle);
	mat[1][0] = (float)sin((double)radianAngle);
	mat[1][1] = (float)cos((double)radianAngle);
	mat[2][2] = 1.0;
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzTrxMat(GzCoord translate, GzMatrix mat)
{
	/* HW 3.4
	// Create translation matrix
	// Pass back the matrix using mat value
	*/
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			mat[i][j] = 0;
		}
	}
	mat[0][0] = 1.0;
	mat[0][3] = translate[0];
	mat[1][1] = 1.0;
	mat[1][3] = translate[1];
	mat[2][2] = 1.0;
	mat[2][3] = translate[2];
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}


int GzRender::GzScaleMat(GzCoord scale, GzMatrix mat)
{
	/* HW 3.5
	// Create scaling matrix
	// Pass back the matrix using mat value
	*/
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			mat[i][j] = 0;
		}
	}
	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}


GzRender::GzRender(int xRes, int yRes)
{
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = (unsigned short)xRes;
	yres = (unsigned short)yRes;

	int resolution = 0;
	resolution = xres * yres;
	int frameBufferDepth = (RGB_DIMEMSION)*resolution;	// Add 1 as the Z 
	framebuffer = new char[frameBufferDepth];
	pixelbuffer = new GzPixel[resolution];

	/* HW 3.6
	- setup Xsp and anything only done once
	- init default camera
	*/
	numlights = 0;
	matlevel = -1;
	matlevelNormal = -1;

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			Xsp[i][j] = 0;
		}
	}
	Xsp[0][0] = (float)xres / 2.0f;
	Xsp[0][3] = (float)xres / 2.0f;
	Xsp[1][1] = -1.0f * (float)yres / 2.0f;
	Xsp[1][3] = (float)yres / 2.0f;
	Xsp[2][2] = (float)MAXINT;
	Xsp[3][3] = 1.0;

	m_camera.position[0] = DEFAULT_IM_X;
	m_camera.position[1] = DEFAULT_IM_Y;
	m_camera.position[2] = DEFAULT_IM_Z;
	for (int i = 0; i < 3; i++) {
		m_camera.lookat[i] = 0;
		m_camera.worldup[i] = 0;
	}
	m_camera.worldup[1] = 1.0;
	m_camera.FOV = DEFAULT_FOV;

	numlights = 0;
}

GzRender::~GzRender()
{
	/* HW1.2 clean up, free buffer memory */
	delete[] framebuffer;
	delete[] pixelbuffer;
}

int GzRender::GzDefault()
{
	/* HW1.3 set pixel buffer to some default values - start a new frame */
	GzPixel defaultPixel = { 2880, 2880, 2880, 1, MAXINT };

	int resolution = xres * yres;
	for (int i = 0; i < resolution; i++) {
		pixelbuffer[i] = defaultPixel;
		framebuffer[RGB_DIMEMSION * i] = (char)2880;
		framebuffer[RGB_DIMEMSION * i + 1] = (char)2880;
		framebuffer[RGB_DIMEMSION * i + 2] = (char)2880;
		//framebuffer[RGB_DIMEMSION * i + 3] = (char)MAXINT;	// initialize Z.
	}

	return GZ_SUCCESS;
}

int GzRender::GzBeginRender()
{
	/* HW 3.7
	- setup for start of each frame - init frame buffer color,alpha,z
	- compute Xiw and projection xform Xpi from camera definition
	- init Ximage - put Xsp at base of stack, push on Xpi and Xiw
	- now stack contains Xsw and app can push model Xforms when needed
	*/

	// Define I:
	GzMatrix matrix_I;
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			matrix_I[j][i] = 0;
		}
	}
	matrix_I[0][0] = 1.0f;
	matrix_I[1][1] = 1.0f;
	matrix_I[2][2] = 1.0f;
	matrix_I[3][3] = 1.0f;

	// Compute Xiw:
	GzCoord cl, newUp, the_X, the_Y, the_Z;
	for (int i = 0; i < 3; i++) {
		cl[i] = m_camera.lookat[i] - m_camera.position[i];
	}
	the_Z[0] = cl[0] / (float)sqrt((double)(cl[0] * cl[0] + cl[1] * cl[1] + cl[2] * cl[2]));
	the_Z[1] = cl[1] / (float)sqrt((double)(cl[0] * cl[0] + cl[1] * cl[1] + cl[2] * cl[2]));
	the_Z[2] = cl[2] / (float)sqrt((double)(cl[0] * cl[0] + cl[1] * cl[1] + cl[2] * cl[2]));

	float upDotZ = m_camera.worldup[0] * the_Z[0] + m_camera.worldup[1] * the_Z[1] + m_camera.worldup[2] * the_Z[2];
	newUp[0] = m_camera.worldup[0] - upDotZ * the_Z[0];
	newUp[1] = m_camera.worldup[1] - upDotZ * the_Z[1];
	newUp[2] = m_camera.worldup[2] - upDotZ * the_Z[2];
	the_Y[0] = newUp[0] / (float)sqrt((double)(newUp[0] * newUp[0] + newUp[1] * newUp[1] + newUp[2] * newUp[2]));
	the_Y[1] = newUp[1] / (float)sqrt((double)(newUp[0] * newUp[0] + newUp[1] * newUp[1] + newUp[2] * newUp[2]));
	the_Y[2] = newUp[2] / (float)sqrt((double)(newUp[0] * newUp[0] + newUp[1] * newUp[1] + newUp[2] * newUp[2]));

	the_X[0] = the_Y[1] * the_Z[2] - the_Y[2] * the_Z[1];
	the_X[1] = the_Y[2] * the_Z[0] - the_Y[0] * the_Z[2];
	the_X[2] = the_Y[0] * the_Z[1] - the_Y[1] * the_Z[0];

	m_camera.Xiw[0][0] = the_X[0];
	m_camera.Xiw[0][1] = the_X[1];
	m_camera.Xiw[0][2] = the_X[2];
	m_camera.Xiw[1][0] = the_Y[0];
	m_camera.Xiw[1][1] = the_Y[1];
	m_camera.Xiw[1][2] = the_Y[2];
	m_camera.Xiw[2][0] = the_Z[0];
	m_camera.Xiw[2][1] = the_Z[1];
	m_camera.Xiw[2][2] = the_Z[2];
	m_camera.Xiw[0][3] = -1.0f * (the_X[0] * m_camera.position[0] + the_X[1] * m_camera.position[1] + the_X[2] * m_camera.position[2]);
	m_camera.Xiw[1][3] = -1.0f * (the_Y[0] * m_camera.position[0] + the_Y[1] * m_camera.position[1] + the_Y[2] * m_camera.position[2]);
	m_camera.Xiw[2][3] = -1.0f * (the_Z[0] * m_camera.position[0] + the_Z[1] * m_camera.position[1] + the_Z[2] * m_camera.position[2]);
	m_camera.Xiw[3][0] = 0;
	m_camera.Xiw[3][1] = 0;
	m_camera.Xiw[3][2] = 0;
	m_camera.Xiw[3][3] = 1;

	// Compute Xpi:
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			m_camera.Xpi[i][j] = 0;
		}
	}
	m_camera.Xpi[0][0] = 1;
	m_camera.Xpi[1][1] = 1;
	m_camera.Xpi[2][2] = (float)tan((double)(m_camera.FOV * PI / 180.0) / 2.0);
	m_camera.Xpi[3][2] = (float)tan((double)(m_camera.FOV * PI / 180.0) / 2.0);
	m_camera.Xpi[3][3] = 1;

	// Push Xsp:
	int status = 0;
	status |= GzPushMatrix(Xsp);
	status |= GzPushMatrix(m_camera.Xpi);
	status |= GzPushMatrix(m_camera.Xiw);

	if (status)
		return GZ_FAILURE;
	else
		return GZ_SUCCESS;

}

int GzRender::GzPutCamera(GzCamera camera)
{
	/* HW 3.8
	/*- overwrite renderer camera structure with new camera definition
	*/
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			m_camera.Xiw[i][j] = camera.Xiw[i][j];
			m_camera.Xpi[i][j] = camera.Xpi[i][j];
		}
	}

	for (int i = 0; i < 3; i++) {
		m_camera.position[i] = camera.position[i];
		m_camera.lookat[i] = camera.lookat[i];
		m_camera.worldup[i] = camera.worldup[i];
	}
	m_camera.FOV = camera.FOV;

	return GZ_SUCCESS;
}

int GzRender::GzPushMatrix(GzMatrix	matrix)
{
	/* HW 3.9
	- push a matrix onto the Ximage stack
	- check for stack overflow
	*/

	int status = 0;
	status |= pushMatToStack(matlevel, Ximage, matrix);

	GzMatrix matrix_I;
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			matrix_I[j][i] = 0;
		}
	}
	matrix_I[0][0] = 1.0f;
	matrix_I[1][1] = 1.0f;
	matrix_I[2][2] = 1.0f;
	matrix_I[3][3] = 1.0f;

	// For Xnorm: Normal Stack:
	if (matlevelNormal < 2) {	// Xsp and Xpi, Push I
		status |= pushMatToStack(matlevelNormal, Xnorm, matrix_I);
	}
	else if (matlevelNormal == 2) {	// Xiw.
		GzMatrix Xnorm_iw;
		for (int j = 0; j < 4; j++) {
			for (int i = 0; i < 4; i++) {
				Xnorm_iw[j][i] = m_camera.Xiw[j][i];
			}
		}
		Xnorm_iw[0][3] = 0;
		Xnorm_iw[1][3] = 0;
		Xnorm_iw[2][3] = 0;

		status |= pushMatToStack(matlevelNormal, Xnorm, Xnorm_iw);
	}
	else {	// Only Allow Rotation Matrices.
		if (matrix[0][1] == 0 && matrix[0][2] == 0 && matrix[1][0] == 0
			&& matrix[1][2] == 0 && matrix[2][0] == 0 && matrix[2][1] == 0) {
			status |= pushMatToStack(matlevelNormal, Xnorm, matrix_I);
		}
		else {
			status |= pushMatToStack(matlevelNormal, Xnorm, matrix);
		}
	}

	if (status)
		return GZ_FAILURE;
	return GZ_SUCCESS;

}

int GzRender::GzPopMatrix()
{
	/* HW 3.10
	- pop a matrix off the Ximage stack
	- check for stack underflow
	*/

	if (popMatToStack(matlevel, Ximage))
		return GZ_FAILURE;
	return GZ_SUCCESS;
}

int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
	/* HW1.4 write pixel values into the buffer */
	if (i >= 0 && i < xres && j >= 0 && j < yres) {
		int index = ARRAY(i, j);
		if (z > 0 && z < MAXINT) {  // Only render objects in front of the camera (positive z)
			if (z < pixelbuffer[index].z) {
				GzPixel currentPixel = { r, g, b, a, z };
				pixelbuffer[index] = currentPixel;
			}
		}
	}
	return GZ_SUCCESS;
}


int GzRender::GzGet(int i, int j, GzIntensity* r, GzIntensity* g, GzIntensity* b, GzIntensity* a, GzDepth* z)
{
	/* HW1.5 retrieve a pixel information from the pixel buffer */
	if (i >= 0 && i < xres && j >= 0 && j < yres) {
		int index = ARRAY(i, j);
		GzPixel currentPixel = pixelbuffer[index];
		*r = currentPixel.red;
		*g = currentPixel.green;
		*b = currentPixel.blue;
		*a = currentPixel.alpha;
		*z = currentPixel.z;
	}
	return GZ_SUCCESS;
}


int GzRender::GzFlushDisplay2File(FILE* outfile)
{
	/* HW1.6 write image to ppm file -- "P6 %d %d 255\r" */
	fprintf(outfile, "P6 %d %d 255\n", xres, yres);
	int resolution = xres * yres;
	for (int i = 0; i < resolution; i++) {
		GzPixel currentPixel = pixelbuffer[i];

		GzIntensity gotRed = currentPixel.red;
		GzIntensity gotGreen = currentPixel.green;
		GzIntensity gotBlue = currentPixel.blue;
		if (currentPixel.red < 0)
			gotRed = 0;
		if (currentPixel.red > COLOR_LIMIT)
			gotRed = COLOR_LIMIT;
		if (currentPixel.green < 0)
			gotGreen = 0;
		if (currentPixel.green > COLOR_LIMIT)
			gotGreen = COLOR_LIMIT;
		if (currentPixel.blue < 0)
			gotBlue = 0;
		if (currentPixel.blue > COLOR_LIMIT)
			gotBlue = COLOR_LIMIT;

		GzIntensity red2 = gotRed >> 4;
		char redValue = (char)(red2 & 0xFF);
		GzIntensity green2 = gotGreen >> 4;
		char greenValue = (char)(green2 & 0xFF);
		GzIntensity blue2 = gotBlue >> 4;
		char blueValue = (char)(blue2 & 0xFF);

		char color[3];
		color[0] = redValue;
		color[1] = greenValue;
		color[2] = blueValue;
		fwrite(color, 1, 3, outfile);
	}
	return GZ_SUCCESS;
}

int GzRender::GzFlushDisplay2FrameBuffer()
{
	/* HW1.7 write pixels to framebuffer:
		- put the pixels into the frame buffer
		- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red
		- NOT red, green, and blue !!!
	*/
	int resolution = xres * yres;
	for (int i = 0; i < resolution; i++) {
		GzPixel currentPixel = pixelbuffer[i];

		GzIntensity gotRed = currentPixel.red;
		GzIntensity gotGreen = currentPixel.green;
		GzIntensity gotBlue = currentPixel.blue;
		GzDepth gotZ = currentPixel.z;
		if (currentPixel.red < 0)
			gotRed = 0;
		if (currentPixel.red > COLOR_LIMIT)
			gotRed = COLOR_LIMIT;
		if (currentPixel.green < 0)
			gotGreen = 0;
		if (currentPixel.green > COLOR_LIMIT)
			gotGreen = COLOR_LIMIT;
		if (currentPixel.blue < 0)
			gotBlue = 0;
		if (currentPixel.blue > COLOR_LIMIT)
			gotBlue = COLOR_LIMIT;

		GzIntensity red2 = gotRed >> 4;
		char redValue = (char)(red2 & 0xFF);
		framebuffer[RGB_DIMEMSION * i + 2] = redValue;

		GzIntensity green2 = gotGreen >> 4;
		char greenValue = (char)(green2 & 0xFF);
		framebuffer[RGB_DIMEMSION * i + 1] = greenValue;

		GzIntensity blue2 = gotBlue >> 4;
		char blueValue = (char)(blue2 & 0xFF);
		framebuffer[RGB_DIMEMSION * i] = blueValue;
	}
	return GZ_SUCCESS;
}


/***********************************************/
/* HW2 methods: implement from here */

int GzRender::GzPutAttribute(int numAttributes, GzToken* nameList, GzPointer* valueList)
{
	/* HW 2.1
	-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
	-- In later homeworks set shaders, interpolaters, texture maps, and lights
	*/

	for (int i = 0; i < numAttributes; i++) {
		GzLight* light;
		float* color;
		switch (nameList[i]) {
			case GZ_RGB_COLOR: {
				color = (float*)valueList[i];
				flatcolor[0] = color[0];
				flatcolor[1] = color[1];
				flatcolor[2] = color[2];
				break;
			}
			case GZ_DIRECTIONAL_LIGHT: {
				if (numlights >= MAX_LIGHTS) {
					// sys::cout << "Maximum number of lights reached!" << endl;
				}
				GzLight* light = (GzLight*)valueList[i];  // Declare within block
				lights[numlights] = *light;
				numlights++;
				break;
			}
			case GZ_AMBIENT_LIGHT: {
				GzLight* light = (GzLight*)valueList[i];  // Declare within block
				ambientlight = *light;
				break;
			}
			case GZ_INTERPOLATE: {
				interp_mode = *(int*)valueList[i];
				break;
			}
			case GZ_DIFFUSE_COEFFICIENT: {
				float* color = (float*)valueList[i];
				Kd[0] = color[0];
				Kd[1] = color[1];
				Kd[2] = color[2];
				break;
			}
			case GZ_AMBIENT_COEFFICIENT: {
				float* color = (float*)valueList[i];
				Ka[0] = color[0];
				Ka[1] = color[1];
				Ka[2] = color[2];
				break;
			}
			case GZ_SPECULAR_COEFFICIENT: {
				float* color = (float*)valueList[i];
				Ks[0] = color[0];
				Ks[1] = color[1];
				Ks[2] = color[2];
				break;
			}
			case GZ_DISTRIBUTION_COEFFICIENT: {
				spec = *(float*)valueList[i];
				break;
			}
			case GZ_TEXTURE_MAP: {
				GzTexture func = (GzTexture) valueList[i];
				tex_fun = func;
				break;
			}

		}
	}

	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int numParts, GzToken* nameList, GzPointer* valueList)
/* numParts - how many names and values */
{
	/* HW 2.2
	-- Pass in a triangle description with tokens and values corresponding to
		  GZ_NULL_TOKEN:		do nothing - no values
		  GZ_POSITION:		3 vert positions in model space
	-- Invoke the rastrizer/scanline framework
	-- Return error code
	*/
	GzCoord* verticesPointer = (GzCoord*)valueList[0];
	GzCoord* normalsPointer = (GzCoord*)valueList[1];
	GzTextureIndex* uvsPointer = (GzTextureIndex*)valueList[2];
	
	GzCoord vertices[3], normals[3];
	GzTextureIndex uvs[3];

	// Construct 4D vector:
	float vertices4D[3][4], normals4D[3][4];
	float transedVertices4D[3][4], transedNormals4D[3][4];
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			vertices4D[j][i] = verticesPointer[j][i];
			normals4D[j][i] = normalsPointer[j][i];
		}
		vertices4D[j][3] = 1.0;
		normals4D[j][3] = 1.0;
	}
	// Transformation M * Vertex Coord.
	for (int k = 0; k < 3; k++) {
		for (int j = 0; j < 4; j++) {
			float sum = 0;
			float sumN = 0;
			for (int i = 0; i < 4; i++) {
				sum += Ximage[matlevel][j][i] * vertices4D[k][i];
				sumN += Xnorm[matlevel][j][i] * normals4D[k][i];
			}
			transedVertices4D[k][j] = sum;
			transedNormals4D[k][j] = sumN;
		}
	}
	// 4D => 3D
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			vertices[j][i] = transedVertices4D[j][i] / transedVertices4D[j][3];
			normals[j][i] = transedNormals4D[j][i] / transedNormals4D[j][3];
		}
		for (int i = 0; i < 2; i++) {
			uvs[j][i] = uvsPointer[j][i];
		}
	}

	// Begin Rasterization:
	if (vertices[0][1] > vertices[1][1]) {
		swapArrays(vertices, 0, 1);
		swapArrays(normals, 0, 1);
		for (int k = 0; k < 2; k++) {
			std::swap(uvs[0][k], uvs[1][k]);
		}
	};
	if (vertices[0][1] > vertices[2][1]) {
		swapArrays(vertices, 0, 2);
		swapArrays(normals, 0, 2);
		for (int k = 0; k < 2; k++) {
			std::swap(uvs[0][k], uvs[2][k]);
		}
	}
	if (vertices[1][1] > vertices[2][1]) {
		swapArrays(vertices, 1, 2);
		swapArrays(normals, 1, 2);
		for (int k = 0; k < 2; k++) {
			std::swap(uvs[1][k], uvs[2][k]);
		}
	}

	if ((int)(vertices[0][1] + 0.5) == (int)(vertices[1][1] + 0.5) && vertices[0][0] > vertices[1][0]) {
		swapArrays(vertices, 1, 2);
		swapArrays(normals, 1, 2);
		for (int k = 0; k < 2; k++) {
			std::swap(uvs[1][k], uvs[2][k]);
		}
	}
	else if ((int)(vertices[1][1] + 0.5) == (int)(vertices[2][1] + 0.5) && vertices[2][0] > vertices[1][0]) {
		swapArrays(vertices, 1, 2);
		swapArrays(normals, 1, 2);
		for (int k = 0; k < 2; k++) {
			std::swap(uvs[1][k], uvs[2][k]);
		}
	}
	else {
		float middleX;
		if ((int)(vertices[0][0] + 0.5) == (int)(vertices[2][0] + 0.5)) {
			middleX = vertices[0][0];
		}
		else {
			float slopeOfSideEdge = (vertices[0][1] - vertices[2][1]) / (vertices[0][0] - vertices[2][0]);
			middleX = (vertices[1][1] - vertices[0][1]) / slopeOfSideEdge + vertices[0][0];
		}

		if (middleX > vertices[1][0]) {
			swapArrays(vertices, 1, 2);
			swapArrays(normals, 1, 2);
			for (int k = 0; k < 2; k++) {
				std::swap(uvs[1][k], uvs[2][k]);
			}
		}
	}

	//sorted as CW. 3 edges: 1-2, 2-3, 3-1.
	float deltaX12, deltaY12, deltaX23, deltaY23, deltaX31, deltaY31;
	float A12, B12, C12, A23, B23, C23, A31, B31, C31;

	deltaX12 = vertices[1][0] - vertices[0][0];
	deltaY12 = vertices[1][1] - vertices[0][1];
	deltaX23 = vertices[2][0] - vertices[1][0];
	deltaY23 = vertices[2][1] - vertices[1][1];
	deltaX31 = vertices[0][0] - vertices[2][0];
	deltaY31 = vertices[0][1] - vertices[2][1];

	A12 = deltaY12;
	B12 = -1.0f * deltaX12;
	C12 = deltaX12 * vertices[0][1] - deltaY12 * vertices[0][0];
	A23 = deltaY23;
	B23 = -1.0f * deltaX23;
	C23 = deltaX23 * vertices[1][1] - deltaY23 * vertices[1][0];
	A31 = deltaY31;
	B31 = -1.0f * deltaX31;
	C31 = deltaX31 * vertices[2][1] - deltaY31 * vertices[2][0];

	// Get the current plane to interpolate Z:
	vec3 E0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], vertices[1][2] - vertices[0][2]);
	vec3 E1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], vertices[2][2] - vertices[0][2]);
	Plane depthPlane(E0, E1, vec3(vertices[0][0], vertices[0][1], vertices[0][2]));

	// Lighting of 3 Vertices:
	vec3 finalIntensity[3];
	vec3 RE0, RE1, GE0, GE1, BE0, BE1;
	vec3 redVertex, greenVertex, blueVertex;
	if (interp_mode == GZ_COLOR) {
		for (int i = 0; i < 3; i++) {
			vec3 normal(normals[i][0], normals[i][1], normals[i][2]);
			finalIntensity[i] = shadingEquation(this, normal);
		}

		// Define edge vectors for each color component
		RE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].x - finalIntensity[0].x);
		RE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].x - finalIntensity[0].x);
		GE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].y - finalIntensity[0].y);
		GE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].y - finalIntensity[0].y);
		BE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].z - finalIntensity[0].z);
		BE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].z - finalIntensity[0].z);

		// Define vertices for each color plane
		redVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].x);
		greenVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].y);
		blueVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].z);
	}
	
	// Create planes for each color component
	Plane redPlane(RE0, RE1, redVertex);
	Plane greenPlane(GE0, GE1, greenVertex);
	Plane bluePlane(BE0, BE1, blueVertex);

	// X-axis normal interpolation
	vec3 XE0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], normals[1][0] - normals[0][0]);
	vec3 XE1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], normals[2][0] - normals[0][0]);
	Plane xnormPlane(XE0, XE1, vec3(vertices[0][0], vertices[0][1], normals[0][0]));

	// Y-axis normal interpolation
	vec3 YE0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], normals[1][1] - normals[0][1]);
	vec3 YE1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], normals[2][1] - normals[0][1]);
	Plane ynormPlane(YE0, YE1, vec3(vertices[0][0], vertices[0][1], normals[0][1]));

	// Z-axis normal interpolation
	vec3 ZE0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], normals[1][2] - normals[0][2]);
	vec3 ZE1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], normals[2][2] - normals[0][2]);
	Plane znormPlane(ZE0, ZE1, vec3(vertices[0][0], vertices[0][1], normals[0][2]));

	// Perspective Correction:
	float vZPrime;
	GzTextureIndex perspUVList[3];
	for (int j = 0; j < 3; j++) {
		vZPrime = vertices[j][2] / ((float)MAXINT - vertices[j][2]);
		perspUVList[j][U] = uvs[j][U] / (vZPrime + 1.0f);
		perspUVList[j][V] = uvs[j][V] / (vZPrime + 1.0f);
	}

	// U interpolation
	vec3 UE0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], perspUVList[1][0] - perspUVList[0][0]);
	vec3 UE1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], perspUVList[2][0] - perspUVList[0][0]);
	Plane unormPlane(UE0, UE1, vec3(vertices[0][0], vertices[0][1], perspUVList[0][0]));

	// V interpolation
	vec3 VE0(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], perspUVList[1][1] - perspUVList[0][1]);
	vec3 VE1(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], perspUVList[2][1] - perspUVList[0][1]);
	Plane vnormPlane(VE0, VE1, vec3(vertices[0][0], vertices[0][1], perspUVList[0][1]));

	// Get Bounding Box:
	float minX = min(min(vertices[0][0], vertices[1][0]), vertices[2][0]);
	float maxX = max(max(vertices[0][0], vertices[1][0]), vertices[2][0]);
	float minY = min(min(vertices[0][1], vertices[1][1]), vertices[2][1]);
	float maxY = max(max(vertices[0][1], vertices[1][1]), vertices[2][1]);
	int minXPixel = (int)(minX + 0.5);
	int maxXPixel = (int)(maxX + 0.5);
	int minYPixel = (int)(minY + 0.5);
	int maxYPixel = (int)(maxY + 0.5);

	// Start Rasterization:
	for (int i = minXPixel; i <= maxXPixel; i++) {
		for (int j = minYPixel; j <= maxYPixel; j++) {
			float LEE12 = A12 * (float)i + B12 * (float)j + C12;
			float LEE23 = A23 * (float)i + B23 * (float)j + C23;
			float LEE31 = A31 * (float)i + B31 * (float)j + C31;

			if ((LEE12 > 0 && LEE23 > 0 && LEE31 > 0 && depthPlane.C != 0)
				|| (LEE12 < 0 && LEE23 < 0 && LEE31 < 0 && depthPlane.C != 0)
				|| LEE12 == 0 || LEE23 == 0 || LEE31 == 0) { // Any pixel inside or on the 3 edges.
				float interpolatedZ = -1.0f * (depthPlane.A * (float)i + depthPlane.B * (float)j + depthPlane.D) / depthPlane.C;
				int currentZ = (int)(interpolatedZ + 0.5);
				GzIntensity redIntensity, greenIntensity, blueIntensity;
				if (currentZ >= 0) {
					if (interp_mode == GZ_FLAT) {
						redIntensity = ctoi(flatcolor[0]);
						greenIntensity = ctoi(flatcolor[1]);
						blueIntensity = ctoi(flatcolor[2]);
					}
					else if (interp_mode == GZ_COLOR) {

						if (tex_fun != NULL && tex_fun != (void*)0xcccccccc) {

							GzTextureIndex currentUV;
							float vzPrimeInterp = (float)currentZ / ((float)MAXINT - (float)currentZ);

							currentUV[U] = -(unormPlane.A * i + unormPlane.B * j + unormPlane.D) / unormPlane.C;
							currentUV[V] = -(vnormPlane.A * i + vnormPlane.B * j + vnormPlane.D) / vnormPlane.C;
							currentUV[U] = currentUV[U] * (vzPrimeInterp + 1.0f);
							currentUV[V] = currentUV[V] * (vzPrimeInterp + 1.0f);

							GzColor color;
							for (int i = 0; i < 3; i++) {
								color[i] = 0.0f;
							};
							int status = tex_fun(currentUV[U], currentUV[V], color);
							if (status) {
								return GZ_FAILURE;
							}

							Ka[0] = color[0];
							Ka[1] = color[1];
							Ka[2] = color[2];

							Kd[0] = color[0];
							Kd[1] = color[1];
							Kd[2] = color[2];

							Ks[0] = color[0];
							Ks[1] = color[1];
							Ks[2] = color[2];

							vec3 finalIntensity[3];
							for (int i = 0; i < 3; i++) {
								vec3 normal(normals[i][0], normals[i][1], normals[i][2]);
								finalIntensity[i] = shadingEquation(this, normal);
							}

							// Define edge vectors for each color component
							RE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].x - finalIntensity[0].x);
							RE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].x - finalIntensity[0].x);
							GE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].y - finalIntensity[0].y);
							GE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].y - finalIntensity[0].y);
							BE0 = vec3(vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], finalIntensity[1].z - finalIntensity[0].z);
							BE1 = vec3(vertices[2][0] - vertices[0][0], vertices[2][1] - vertices[0][1], finalIntensity[2].z - finalIntensity[0].z);

							// Define vertices for each color plane
							redVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].x);
							greenVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].y);
							blueVertex = vec3(vertices[0][0], vertices[0][1], finalIntensity[0].z);

							// Create planes for each color component
							redPlane = Plane(RE0, RE1, redVertex);
							greenPlane = Plane(GE0, GE1, greenVertex);
							bluePlane = Plane(BE0, BE1, blueVertex);
						}

						float red, green, blue;
						red = -(redPlane.A * i + redPlane.B * j + redPlane.D) / redPlane.C;
						green = -(greenPlane.A * i + greenPlane.B * j + greenPlane.D) / greenPlane.C;
						blue = -(bluePlane.A * i + bluePlane.B * j + bluePlane.D) / bluePlane.C;
						redIntensity = ctoi(red);
						greenIntensity = ctoi(green);
						blueIntensity = ctoi(blue);
					}
					else if (interp_mode == GZ_NORMAL) {

						if (tex_fun != NULL) {
							GzTextureIndex currentUV;
							float vzPrimeInterp = (float)currentZ / ((float)MAXINT - (float)currentZ);

							currentUV[U] = -(unormPlane.A * i + unormPlane.B * j + unormPlane.D) / unormPlane.C;
							currentUV[V] = -(vnormPlane.A * i + vnormPlane.B * j + vnormPlane.D) / vnormPlane.C;
							currentUV[U] = currentUV[U] * (vzPrimeInterp + 1.0f);
							currentUV[V] = currentUV[V] * (vzPrimeInterp + 1.0f);

							GzColor color;
							for (int i = 0; i < 3; i++) {
								color[i] = 0.0f;
							};
							int status = tex_fun(currentUV[U], currentUV[V], color);
							if (status) {
								return GZ_FAILURE;
							}
							Ka[0] = color[0];
							Ka[1] = color[1];
							Ka[2] = color[2];

							Kd[0] = color[0];
							Kd[1] = color[1];
							Kd[2] = color[2];
						
						}

						vec3 finalColor;
						float Nx, Ny, Nz;
						Nx = -(xnormPlane.A * i + xnormPlane.B * j + xnormPlane.D) / xnormPlane.C;
						Ny = -(ynormPlane.A * i + ynormPlane.B * j + ynormPlane.D) / ynormPlane.C;
						Nz = -(znormPlane.A * i + znormPlane.B * j + znormPlane.D) / znormPlane.C;
						finalColor = shadingEquation(this, vec3(Nx, Ny, Nz).normalize());
						redIntensity = ctoi(finalColor.x);
						greenIntensity = ctoi(finalColor.y);
						blueIntensity = ctoi(finalColor.z);
					}
					else if (interp_mode == GZ_TEXTURE_INDEX) {
						float red, green, blue;

						red = -(unormPlane.A * i + unormPlane.B * j + unormPlane.D) / unormPlane.C;
						green = -(vnormPlane.A * i + vnormPlane.B * j + vnormPlane.D) / vnormPlane.C;
						blue = 0;
						redIntensity = ctoi(red);
						greenIntensity = ctoi(green);
						blueIntensity = ctoi(blue);
					}
					GzPut(i, j, redIntensity, greenIntensity, blueIntensity, 1, currentZ);
				}
			}
		}
	}

	return GZ_SUCCESS;
}

int pushMatToStack(short& head, GzMatrix* matStack, GzMatrix mat) {

	if (head < MATLEVELS) {
		if (head == -1) {
			for (int j = 0; j < 4; j++) {
				for (int i = 0; i < 4; i++) {
					matStack[0][i][j] = mat[i][j];
				}
			}
		}
		else {
			// Matrix Multiplication:
			for (int k = 0; k < 4; k++) {
				for (int j = 0; j < 4; j++) {
					float sum = 0;
					for (int i = 0; i < 4; i++) {
						sum += matStack[head][k][i] * mat[i][j];
					}
					matStack[head + 1][k][j] = sum;
				}
			}
		}
		// increment head
		head++;
	}
	else
		return GZ_FAILURE;

	return GZ_SUCCESS;
}

vec3 shadingEquation(GzRender* render, vec3 normal) {
	GzColor specIntensity, diffIntensity, ambIntensity, finalIntensity;

	for (int i = 0; i < 3; i++) {
		specIntensity[i] = 0.0f;
		diffIntensity[i] = 0.0f;
		ambIntensity[i] = 0.0f;
		finalIntensity[i] = 0.0f;
	}

	for (int j = 0; j < render->numlights; j++) {
		vec3 E(0.0f, 0.0f, -1.0f);  // Camera direction
		vec3 L(render->lights[j].direction[0], render->lights[j].direction[1], render->lights[j].direction[2]);

		float NdotL, NdotE;
		NdotL = normal.dot(L);
		NdotE = normal.dot(E);

		if (NdotL * NdotE > 0) {
			// calculate and normalize R 
			vec3 R = (normal * (2.0f * NdotL) - L).normalize();
			float RdotE = max(0.0f, R.dot(E));
			RdotE = NdotL > 0 ? pow(RdotE, render->spec) : 0.0f;

			// each color
			for (int k = 0; k < 3; k++) {
				if (NdotL > 0 && NdotE > 0) {
					specIntensity[k] += render->Ks[k] * render->lights[j].color[k] * pow(RdotE, render->spec);
					diffIntensity[k] += render->Kd[k] * render->lights[j].color[k] * NdotL;
				}
				else if (NdotL < 0 && NdotE < 0) {
					vec3 negativeR = vec3(
						-2.0f * NdotL * normal.x - L.x,
						-2.0f * NdotL * normal.y - L.y,
						-2.0f * NdotL * normal.z - L.z
					).normalize();
					float negRdotE = max(0.0f, negativeR.dot(E));
					specIntensity[k] += render->Ks[k] * render->lights[j].color[k] * pow(negRdotE, render->spec);
					diffIntensity[k] += render->Kd[k] * render->lights[j].color[k] * (-NdotL);
				}
			}
		}
	}

	for (int k = 0; k < 3; k++) {
		ambIntensity[k] += render->Ka[k] * render->ambientlight.color[k];
	}

	for (int k = 0; k < 3; k++) {
		finalIntensity[k] = ambIntensity[k] + diffIntensity[k] + specIntensity[k];

		// Clamp color to [0, 1] range
		finalIntensity[k] = min(1.0f, max(0.0f, finalIntensity[k]));
	}

	vec3 result(finalIntensity[0], finalIntensity[1], finalIntensity[2]);
	return result;
}

int popMatToStack(short& head, GzMatrix* matStack) {
	if (head > -1) {
		head--;
	}
	else
		return GZ_FAILURE;

	return GZ_SUCCESS;
}

void swapArrays(float arrays[3][3], int i, int j) {
	for (int k = 0; k < 3; k++) {
		std::swap(arrays[i][k], arrays[j][k]);
	}
}

