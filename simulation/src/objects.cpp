#include <cstring>

#include "objects.h"
#include "misc.h"

/**********************************************************************************
 * 
 * Implementation details for Sphere.
 * 
************************************************************************************/
 Sphere::Sphere(int phi_step, int theta_step,vec3 color)
 : phi_step(phi_step), theta_step(theta_step)
 {
    draw_firsts.resize(phi_step);
    draw_counts.resize(theta_step);

    this->init(color);
 }
 
 Sphere::~Sphere() {
    glDeleteBuffers(3,vbo);
    glDeleteVertexArrays(1,&vao);
}
 void Sphere::init(vec3 color,GLuint usage) {

    int data_size = 8*phi_step*theta_step;
    GLfloat* data_pos = new GLfloat[data_size];
    GLfloat* data_color = new GLfloat[data_size];

    vector<GLfloat> phi = lerp(static_cast<GLfloat>(0),2*PI,phi_step);
    vector<GLfloat> theta = lerp(static_cast<GLfloat>(0),PI,theta_step);

    int idx = 0;
    vec4 p;
    auto s_color = [](GLfloat in_phi, GLfloat in_theta) {
        vec3 s = S2(in_phi/4,in_theta/2);
        return vec4{(float)pow(s[0],2),(float)pow(s[1],2),(float)pow(s[2],2),1};
    };   

    for (int i = 0; i < phi_step; i++) {
        for (int j = 0; j < theta_step; j++) {
            // First point
            p = vec4(S2(phi[i],theta[j]),0);
            memcpy(data_pos + idx, &p,4*sizeof(float));

            p = vec4(color,0);
            memcpy(data_color + idx, &p,4*sizeof(float));

            idx += 4;

            // Second point
            p = vec4(S2(phi[i+1],theta[j+1]),0);
            memcpy(data_pos + idx, &p,4*sizeof(float));

            p = vec4(color,0);
            memcpy(data_color + idx, &p,4*sizeof(float));

            idx += 4;
        }
    }

    // Need to set this up to use glMultiDrawArrays
    for (int i = 0; i < phi_step; i++){ 
        draw_firsts[i] = 2*theta_step*i;
        draw_counts[i] = 2*theta_step;
    }

    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    for (int i = 0; i < SPH_ATTR_COUNT; i++) {
        glEnableVertexAttribArray(i);
        glGenBuffers(1,vbo + i);
    }

    glBindBuffer(GL_ARRAY_BUFFER,vbo[POSITION]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_pos,usage);
    glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER,vbo[COLOR]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_color,usage);
    glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // On the unit sphere we can just use the positions as normals!
    glBindBuffer(GL_ARRAY_BUFFER,vbo[NORMAL]);
    glBufferData(GL_ARRAY_BUFFER,data_size*sizeof(float),data_pos,usage);
    glVertexAttribPointer(NORMAL, 4, GL_FLOAT, GL_FALSE, 0, 0);

    delete[] data_pos;
    delete[] data_color;
    return;
}
void Sphere::render(ShaderProgram& shader) {
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    shader.setUniform("model",geometry,GL_TRUE);
    shader.setUniform("scale",scale);
    glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLE_STRIP,0,2*phi_step*theta_step);
    glMultiDrawArrays(GL_TRIANGLE_STRIP,draw_firsts.data(),draw_counts.data(),phi_step);
    return;
}
void Sphere::setScale(GLfloat in_scale) 
{
    this->scale = in_scale;
    return;
}
void Sphere::setPos(vec3 pos) 
{
    geometry[0][3] = pos[0];
    geometry[1][3] = pos[1];
    geometry[2][3] = pos[2];
    return;
}
void Sphere::rotate(vec3 axis, GLfloat angle) 
{   
    axis = normalize(axis);
    mat3 differential = mat3{
         0,         -axis[2],   axis[1],
         axis[2],   0,         -axis[0],
        -axis[1],   axis[0],   0
    };
    mat4 rotation = mat4(exp<>(angle*differential));
    geometry = geometry * rotation;
    return;
} 

