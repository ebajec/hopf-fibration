#include "camera.h"
#include <cstring>
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"
#include "misc.h"

#define SIZE_F_MAT4 16

static mat3 orthCoordsLeft(vec3 normal)
{
	vec3 basis[3];

	normal = normalize(normal);
	basis[2] = normal;
	basis[0] = cross(normal, vec3({ 0,0,1 }));
	basis[1] = cross(basis[0], basis[2]);
	basis[0] = normalize(basis[0]);
	basis[1] = normalize(basis[1]);

	return mat3(basis[0],basis[1],basis[2]);
}

Camera::Camera(vec3 normal, vec3 pos, int w, int h, GLfloat FOV, GLfloat near, GLfloat far)
	:
	fov(FOV),
	near(near),
	far(far),
	position(pos)
{
	glGenBuffers(1, &ubo);
	coords = mat4(orthCoordsLeft(normal));
	aspect = (float)h / (float)w;

	updateUbo();
}

Camera::~Camera()
{
	glDeleteBuffers(1,&ubo);
}

mat4 Camera::getViewMatrix()
{
	mat4 world = mat4(1.0f);
	world[3] =	 vec4(-1.0f *(position - vec3(coords[2]) * near),1);
	return glm::transpose(coords)*world;;
}

mat4 Camera::getProjMatrix()
{
	return mat4(
		vec4((1 / tan(fov / 2))*(aspect),0,0,0),
		vec4(0,1 / tan(fov / 2),0,0),
		vec4(0,0,far/(far - near),1),
		vec4(0,0,-far*near/(far - near),0)
	);
}

void Camera::updateUbo()
{	
	mat4 view = getViewMatrix();
	mat4 proj = getProjMatrix();

	CameraUBOLayout data 
	{
		.view = view,
		.proj = proj,
		.pv = proj * view,
		.cam_pos = vec4(position,1),
		.cam_dir = view[2],
		.near = near,
		.far = far
	};

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUBOLayout), &data, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	return;
}
void Camera::bindUbo(GLuint binding) 
{
	glBindBufferBase(GL_UNIFORM_BUFFER,binding,ubo);
}

void Camera::rotate(float pitch, float yaw)
{
	if (fabs(pitch) > PI) return;
	coords = mat4(rotation(vec3(coords[0]), -pitch)) *  mat4(rotation(vec3(0,0,1),-yaw)) * coords ;
}

void Camera::translate(vec3 delta)
{
    position = position + mat3(coords[0],vec3(0,0,1),vec3(coords[2].x,coords[2].y,0))*delta;
}

void Camera::resize(int width, int height)
{
	this->aspect = (float)height/(float)width;
}