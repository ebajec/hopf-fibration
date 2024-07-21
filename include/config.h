#ifndef CONFIG_H
#define CONFIG_H

#include "matrix.h"
#include <GL/glew.h>

#define WIDTH 1280                  
#define HEIGHT 720

#define SPH_ATTR_COUNT 3            

#define CONTROL_VIEWPORT_SIZE 600   // Determines height/width of lower-right viewport

#define FIBER_COUNT 100             // Number of fibers to compute
#define FIBER_SIZE 1000             // Number of samples in each fiber

#define HF_CAMERA_BINDING 1
#define C_CAMERA_BINDING 0

typedef matrix<4,1,GLfloat> vec4;
typedef matrix<3,1,GLfloat> vec3;
typedef matrix<2,1,GLfloat> vec2;

typedef matrix<4,4,GLfloat> mat4;
typedef matrix<3,3,GLfloat> mat3;
typedef matrix<2,2,GLfloat> mat2;

#endif