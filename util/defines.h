#ifndef DEFINES_H
#define DEFINES_H

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#define SPH_ATTR_COUNT 3            

#define CONTROL_VIEWPORT_SIZE 400   // Determines height/width of lower-right viewport
#define FIBER_COUNT 300       // Number of fibers to compute
#define FIBER_SIZE 400            // Number of samples in each fiber

#ifndef PI
#define PI 3.141592654f
#endif

#ifndef ROOT2
#define ROOT2 1.4142135623f
#endif 

#define LENGTH(size,type) (size/sizeof(type))

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;
typedef glm::mat3 mat3;
typedef glm::mat2 mat2;

#endif