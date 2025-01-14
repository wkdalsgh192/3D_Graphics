#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include    <string>

GzRender::GzRender(int xRes, int yRes)
{
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = xRes;
	yres = yRes;
	pixelbuffer = new GzPixel[xRes * yRes];
	framebuffer = new char[xRes * yRes * 3];
}

GzRender::~GzRender()
{
	/* HW1.2 clean up, free buffer memory */
	delete[] pixelbuffer;
	delete[] framebuffer;

}

int GzRender::GzDefault()
{
	/* HW1.3 set pixel buffer to some default values - start a new frame */
	int i;
	int resolution = xres * yres;
	for (i = 0; i < resolution; i++) {
		pixelbuffer[i].red = 2055;
		pixelbuffer[i].green = 1798;
		pixelbuffer[i].blue = 1541;
		pixelbuffer[i].alpha = INT_MAX;
		pixelbuffer[i].z = INT_MAX;
		framebuffer[3 * i] = (char)880;
		framebuffer[3 * i + 1] = (char)880;
		framebuffer[3 * i + 2] = (char)880;
	}
	return GZ_SUCCESS;
}


int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
	/* HW1.4 write pixel values into the buffer */
	if (i < 0 || i >= xres || j < 0 || j >= yres) {
		return GZ_SUCCESS;
	}

	int idx = j * xres + i;
	//pixelbuffer[idx].red = GzClamp(r);
	//pixelbuffer[idx].green = GzClamp(g);
	//pixelbuffer[idx].blue = GzClamp(b);
	//pixelbuffer[idx].alpha = a;
	if (z < pixelbuffer[idx].z) {
		/*pixelbuffer[idx].z = z;*/
		GzPixel currentPixel = { r, g, b, a, z };
		pixelbuffer[idx] = currentPixel;
	}
	
	return GZ_SUCCESS;
}

GzIntensity GzRender::GzClamp(GzIntensity color) {

	short minIntensity = 0;
	short maxIntensity = 4095;
	if (color < minIntensity) {
		return minIntensity;
	}
	else if (color > maxIntensity) {
		return maxIntensity;
	}
	else {
		return color;
	}
}


int GzRender::GzGet(int i, int j, GzIntensity* r, GzIntensity* g, GzIntensity* b, GzIntensity* a, GzDepth* z)
{
	/* HW1.5 retrieve a pixel information from the pixel buffer */
	if (i < 0 || i >= xres || j < 0 || j >= yres) {
		return GZ_FAILURE;
	}
	if (r == nullptr || g == nullptr || b == nullptr || a == nullptr || z == nullptr) {
		return GZ_FAILURE;
	}

	int idx = j * xres + i;
	*r = pixelbuffer[idx].red;
	*g = pixelbuffer[idx].green;
	*b = pixelbuffer[idx].blue;
	*a = pixelbuffer[idx].alpha;
	*z = pixelbuffer[idx].z;
	return GZ_SUCCESS;
}


int GzRender::GzFlushDisplay2File(FILE* outfile)
{
	/* HW1.6 write image to ppm file -- "P6 %d %d 255\r" */
	fprintf(outfile, "P6 %d %d 255\r", xres, yres);

	int resolution = xres * yres;
	for (int i = 0; i < resolution; i++) {
		GzPixel pixel = pixelbuffer[i];
		fputc(pixel.red >> 4 & 0xff, outfile);
		fputc(pixel.green >> 4 & 0xff, outfile);
		fputc(pixel.blue >> 4 & 0xff, outfile);
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
		GzPixel pixel = pixelbuffer[i];
		framebuffer[i * 3] = pixel.blue >> 4 & 0xff;
		framebuffer[i * 3 + 1] = pixel.green >> 4 & 0xff;
		framebuffer[i * 3 + 2] = pixel.red >> 4 & 0xff;
	}

	return GZ_SUCCESS;
}


/***********************************************/
/* HW2 methods: implement from here */

int GzRender::GzPutAttribute(int numAttributes, GzToken	*nameList, GzPointer *valueList) 
{
/* HW 2.1
-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
-- In later homeworks set shadekkrs, interpolaters, texture maps, and lights
	1. Check the token name (For HW2, only GZ_RGB_COLOR is passed)
	2. Cast the corresponding in valueList to the correct data type
	3. Store the cast data value to its destination (stored in flatcolor for HW2)
*/
	for (int i = 0; i < numAttributes; i++) {
		if (nameList[i] == GZ_RGB_COLOR) {
			float* color = (float*)valueList[i];

			// Assign individual components to flatcolor
			flatcolor[0] = color[0];  // Red component
			flatcolor[1] = color[1];  // Green component
			flatcolor[2] = color[2];  // Blue component
		}
	}
	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int	numParts, GzToken* nameList, GzPointer* valueList)
/* numParts - how many names and values */
{
	/* HW 2.2
	-- Pass in a triangle description with tokens and values corresponding to
		  GZ_NULL_TOKEN:		do nothing - no values
		  GZ_POSITION:		3 vert positions in model space
	-- Invoke the rastrizer/scanline framework
	-- Return error code
	*/
	for (int i = 0; i < numParts; i++) {
		if (nameList[i] == GZ_POSITION) {
			// Invoke the rasterizer framework
			GzCoord* vertexList = (GzCoord*)valueList[0];
			GzCoord vertices0 = { vertexList[0][0], vertexList[0][1], vertexList[0][2] };
			GzCoord vertices1 = { vertexList[1][0], vertexList[1][1], vertexList[1][2] };
			GzCoord vertices2 = { vertexList[2][0], vertexList[2][1], vertexList[2][2] };

			if (vertices0[1] > vertices1[1]) {
				swapGzCoord(vertices0, vertices1);
			}
			if (vertices0[1] > vertices2[1]) {
				swapGzCoord(vertices0, vertices2);
			}
			if (vertices1[1] > vertices2[1]) {
				swapGzCoord(vertices1, vertices2);
			}

			//sorted as CW. 3 edges: 1-2, 2-3, 3-1 -> retrieve edges and compute A-B-C vector for z-interpolation
			float deltaX12, deltaY12, deltaX23, deltaY23, deltaX31, deltaY31;
			float A12, B12, C12, A23, B23, C23, A31, B31, C31;

			deltaX12 = vertices1[0] - vertices0[0];
			deltaY12 = vertices1[1] - vertices0[1];
			deltaX23 = vertices2[0] - vertices1[0];
			deltaY23 = vertices2[1] - vertices1[1];
			deltaX31 = vertices0[0] - vertices2[0];
			deltaY31 = vertices0[1] - vertices2[1];

			A12 = deltaY12;
			B12 = -1.0f * deltaX12;
			C12 = deltaX12 * vertices0[1] - deltaY12 * vertices0[0];
			A23 = deltaY23;
			B23 = -1.0f * deltaX23;
			C23 = deltaX23 * vertices1[1] - deltaY23 * vertices1[0];
			A31 = deltaY31;
			B31 = -1.0f * deltaX31;
			C31 = deltaX31 * vertices2[1] - deltaY31 * vertices2[0];
			
			// Get the current plane to interpolate Z:
			GzCoord edge1 = { vertices1[0] - vertices0[0],vertices1[1] - vertices0[1],vertices1[2] - vertices0[2] };
			GzCoord edge2 = { vertices2[0] - vertices0[0],vertices2[1] - vertices0[1],vertices2[2] - vertices0[2] };
			float planeA = edge1[1] * edge2[2] - edge1[2] * edge2[1];
			float planeB = -(edge1[0] * edge2[2] - edge2[0] * edge1[2]);
			float planeC = edge1[0] * edge2[1] - edge1[1] * edge2[0];
			float planeD = -1.0f * (planeA * vertices0[0] + planeB * vertices0[1] + planeC * vertices0[2]);

			float minX = min(min(vertices0[0], vertices1[0]), vertices2[0]);
			float maxX = max(max(vertices0[0], vertices1[0]), vertices2[0]);
			float minY = min(min(vertices0[1], vertices1[1]), vertices2[1]);
			float maxY = max(max(vertices0[1], vertices1[1]), vertices2[1]);
			int minXPixel = (int)(minX + 0.5);
			int maxXPixel = (int)(maxX + 0.5);
			int minYPixel = (int)(minY + 0.5);
			int maxYPixel = (int)(maxY + 0.5);

			for (int i = minXPixel; i <= maxXPixel; i++) {
				for (int j = minYPixel; j <= maxYPixel; j++) {

					float res0 = A12 * (float)i + B12 * (float)j + C12;
					float res1 = A23 * (float)i + B23 * (float)j + C23;
					float res2 = A31 * (float)i + B31 * (float)j + C31;

					// Left side > 0
					// Right < 0
					if ((res0 > 0 && res1 > 0 && res2 > 0 && planeC != 0)
						|| (res0 < 0 && res1 < 0 && res2 < 0 && planeC != 0)
						|| res0 == 0 || res1 == 0 || res2 == 0) {
						// color the pixel by using flatcolor
						float interpolatedZ = -1.0f * (planeA * (float)i + planeB * (float)j + planeD) / planeC;
						// Call GzPut to push the pixel to pixelbuffer.
						GzPut(i, j, ctoi(flatcolor[0]), ctoi(flatcolor[1]), ctoi(flatcolor[2]), 1, interpolatedZ);
					}
				}
			}
		}
	}
	return GZ_SUCCESS;
}

void GzRender::swapGzCoord(GzCoord v1, GzCoord v2) {
	for (int i = 0; i < 3; ++i) {
		float temp = v1[i];
		v1[i] = v2[i];
		v2[i] = temp;
	}
}

