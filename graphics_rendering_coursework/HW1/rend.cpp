#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	<stdexcept>


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
	}
	return GZ_SUCCESS;
}


int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* HW1.4 write pixel values into the buffer */
	if (i < 0 || i >= xres || j < 0 || j >= yres) {
		return GZ_FAILURE;
	}

	int idx = j * xres + i;
	pixelbuffer[idx].red = GzClamp(r);
	pixelbuffer[idx].green = GzClamp(g);
	pixelbuffer[idx].blue = GzClamp(b);
	pixelbuffer[idx].alpha = a;
	pixelbuffer[idx].z = z;
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


int GzRender::GzGet(int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
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