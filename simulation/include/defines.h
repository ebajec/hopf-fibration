#ifndef DEFINES_H
#define DEFINES_H

#include "matrix.h"
#include <GL/glew.h>

#define WIDTH 1280                  
#define HEIGHT 720

#define SPH_ATTR_COUNT 3            

#define CONTROL_VIEWPORT_SIZE 600   // Determines height/width of lower-right viewport

#define FIBER_COUNT 100            // Number of fibers to compute
#define FIBER_SIZE 100            // Number of samples in each fiber

#ifndef PI
    #define PI 3.141592654f
#endif

#ifndef ROOT2
    #define ROOT2 1.4142135623f
#endif 

#define LENGTH(size,type) (size/sizeof(type))

typedef matrix<4,1,GLfloat> vec4;
typedef matrix<3,1,GLfloat> vec3;
typedef matrix<2,1,GLfloat> vec2;

typedef matrix<4,4,GLfloat> mat4;
typedef matrix<3,3,GLfloat> mat3;
typedef matrix<2,2,GLfloat> mat2;

#endif