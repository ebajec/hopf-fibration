#ifndef OBJECTS_H
#define OBJECTS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "defines.h"
#include "matrix.h"
#include "shader.h"


/**********************************************************************************
 *
 * Sphere
 * 
 **********************************************************************************/
class Sphere {
public:
    Sphere(int phi_step, int theta_step, vec3 color = {0.5,0.5,0.5});
    ~Sphere();
    
    void setScale(GLfloat s);
    void setPos(vec3 pos);
    void rotate(vec3 axis, GLfloat angle);
    void render(Shader& shader);
    
    enum ATTRIBUTES {POSITION,COLOR,NORMAL};
private:
    void init(vec3 color = {0.5,0.5,0.5},GLuint usage = GL_STATIC_DRAW);

    int phi_step, theta_step;
    GLuint vao;             
    GLuint vbo[SPH_ATTR_COUNT];                   
    std::vector<GLint>   draw_firsts;        // For glMultiDrawArrays
    std::vector<GLsizei> draw_counts;    // For glMultiDrawArrays

    GLfloat scale = 1;
    mat4 geometry = mat4::id();
};




#endif