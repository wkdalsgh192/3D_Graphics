/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

#include <math.h>
#include <stdlib.h>

/* Define number of feature points for the Voronoi pattern */
#define NUM_FEATURE_POINTS 10

typedef struct {
    float x, y;
} Vec2;

Vec2 featurePoints[NUM_FEATURE_POINTS];

/* Initialize feature points with random values */
void init_feature_points() {
    for (int i = 0; i < NUM_FEATURE_POINTS; i++) {
        featurePoints[i].x = (float)rand() / RAND_MAX;
        featurePoints[i].y = (float)rand() / RAND_MAX;
    }
}

GzColor	*image=NULL;
int xs, ys;
int reset = 1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
    unsigned char		pixel[3];
    unsigned char     dummy;
    char  		foo[8];
    int   		i, j;
    FILE* fd;

    if (reset) {          /* open and load texture file */
        fd = fopen("texture", "rb");
        if (fd == NULL) {
            fprintf(stderr, "texture file not found\n");
            exit(-1);
        }
        fscanf(fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
        image = (GzColor*)malloc(sizeof(GzColor) * (xs + 1) * (ys + 1));
        if (image == NULL) {
            fprintf(stderr, "malloc for texture image failed\n");
            exit(-1);
        }

        for (i = 0; i < xs * ys; i++) {	/* create array of GzColor values */
            fread(pixel, sizeof(pixel), 1, fd);
            image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
            image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
            image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
        }

        reset = 0;          /* init is done */
        fclose(fd);
    }
    if (u < 0) {
        u = 0.0f;
    }

    if (u > 1) {
        u = 1.0f;
    }

    if (v < 0) {
        v = 0.0f;
    }

    if (v > 1) {
        v = 1.0f;
    }

    float x, y, s, t;
    int lowerBoundX, lowerBoundY, upperBoundX, upperBoundY;
    GzColor A, B, C, D;
    x = u * (xs - 1);
    y = v * (ys - 1);

    upperBoundX = (int)ceil(x);
    upperBoundY = (int)ceil(y);
    lowerBoundX = (int)floor(x);
    lowerBoundY = (int)floor(y);

    for (i = 0; i < 3; i++) {
        A[i] = 0.0f;
        B[i] = 0.0f;
        C[i] = 0.0f;
        D[i] = 0.0f;
    }

    A[0] = image[lowerBoundX + lowerBoundY * xs][RED];
    A[1] = image[lowerBoundX + lowerBoundY * xs][GREEN];
    A[2] = image[lowerBoundX + lowerBoundY * xs][BLUE];

    B[0] = image[upperBoundX + lowerBoundY * xs][RED];
    B[1] = image[upperBoundX + lowerBoundY * xs][GREEN];
    B[2] = image[upperBoundX + lowerBoundY * xs][BLUE];

    C[0] = image[upperBoundY * xs + lowerBoundX][RED];
    C[1] = image[upperBoundY * xs + lowerBoundX][GREEN];
    C[2] = image[upperBoundY * xs + lowerBoundX][BLUE];

    D[0] = image[upperBoundY * xs + upperBoundX][RED];
    D[1] = image[upperBoundY * xs + upperBoundX][GREEN];
    D[2] = image[upperBoundY * xs + upperBoundX][BLUE];

    s = x - lowerBoundX;
    t = y - lowerBoundY;

    color[0] = s * t * C[0] + (1 - s) * t * D[0] + s * (1 - t) * B[0] + (1 - s) * (1 - t) * A[0];
    color[1] = s * t * C[1] + (1 - s) * t * D[1] + s * (1 - t) * B[1] + (1 - s) * (1 - t) * A[1];
    color[2] = s * t * C[2] + (1 - s) * t * D[2] + s * (1 - t) * B[2] + (1 - s) * (1 - t) * A[2];
  
    return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color) {
    // Initialize feature points if not already initialized
    static int initialized = 0;
    if (!initialized) {
        init_feature_points();
        initialized = 1;
    }

    float minDist1 = 1e10, minDist2 = 1e10;

    // Find the two nearest feature points
    for (int i = 0; i < NUM_FEATURE_POINTS; i++) {
        float dx = u - featurePoints[i].x;
        float dy = v - featurePoints[i].y;
        float dist = dx * dx + dy * dy;  // Use squared distance for efficiency

        if (dist < minDist1) {
            minDist2 = minDist1;
            minDist1 = dist;
        }
        else if (dist < minDist2) {
            minDist2 = dist;
        }
    }

    // Use distance to modulate color (simple grayscale based on distance)
    float intensity = minDist1 / (minDist1 + minDist2);
    color[0] = intensity;
    color[1] = intensity;
    color[2] = intensity;

    return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}

