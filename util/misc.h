#ifndef MISC_H
#define MISC_H

#include <GL/glew.h>
#include <vector>
#include "matrix.h"
#include "defines.h"

/**********************************************************************************
 * 
 * Helper functions
 * 
 ***********************************************************************************/
static inline vec3 S2(GLfloat phi, GLfloat theta) 
{
    return vec3{sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta)};
}

template<typename T>
static constexpr unsigned int factorial(T x) {
    return (x > 0) ? x*factorial(x-(T)1) : (T)1;
}
template<int n,typename T>
static constexpr glm::mat<n,n,T> exp(glm::mat<n,n,T> mat) {
    auto result = glm::mat<n,n,T>(1.0f);// 0
    result = result + mat;//1
    mat = mat*mat;//2
    result = result + T(pow(factorial(2.0f),-1))*mat; 
    mat = mat*mat;//3
    result = result + T(pow(factorial(3.0f),-1))*mat;
    mat = mat*mat;//4
    result = result + T(pow(factorial(4.0f),-1))*mat;
    return result;
}

static mat3 rotation(vec3 axis, float angle) 
{   
    axis = normalize(axis);
    mat3 differential = mat3(
         0,         -axis.z,   axis.y,
         axis.z,   0,         -axis.x,
        -axis.y,   axis.x,   0
    );

    mat3 rotation = exp(mat3(angle*differential));
    return rotation;
} 

static inline int modulo(int dividend, int divisor) {
	return (dividend % divisor + divisor) % divisor;
}

static inline bool isFloat(GLenum type)
{
    return 
        type == GL_FLOAT      ||
        type == GL_HALF_FLOAT ||
        type == GL_FIXED;     
}
static bool isIntegral(GLenum type)
{
    return 
        type == GL_INT            ||
        type == GL_BYTE           ||
        type == GL_SHORT          ||
        type == GL_UNSIGNED_INT   ||
        type == GL_UNSIGNED_BYTE  ||
        type == GL_UNSIGNED_SHORT;
}

static inline void printData(GLfloat* data, int x, int y) 
{
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            std::cout << data[i*y + j] << " ";
        }
        std::cout << "\n";
    }
}
template<typename T> static std::vector<T> lerp(T start, T end, int steps) 
{
    std::vector<T>  out(steps+1);   
    T diff = end - start;
    float step = pow(steps,-1);

    for (int i = 0; i < steps + 1; i++) {
        out[i] = start + (i*step)*diff;
    }
    return out;
}


static float uRand() 
{
    return static_cast<float>(rand())/static_cast<float>(RAND_MAX);

}
#endif