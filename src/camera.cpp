#include "camera.h"
#include <cstring>

#define SIZE_F_MAT4 16

Camera::Camera(vec3 normal, vec3 pos, int w, int h, GLfloat FOV, GLfloat far)
	:
	_near_dist(1 / tan(FOV / 2)),
	_far_dist(far),
	_pos(pos)
{
	//generate orthonormal, right-handed basis for camera coordinates, with Z
	//as the normal vector. xyz
	normal = normalize(normal);
	basis[2] = normal;
	basis[0] = cross(normal, vec3({ 0,1,0 }));
	basis[1] = cross(basis[0], basis[2]);
	basis[0] = normalize(basis[0]);
	basis[1] = normalize(basis[1]);

	coord_trans = inv(basis[0] | basis[1] | basis[2]);

	setScreenRatio(w,h);

	_world = mat4(mat3::id() | -1 * (_pos - basis[2] * _near_dist));

	_model_yaw = mat4{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	_model_pitch = mat4{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
}
void Camera::init(){
	glGenBuffers(1, &ubo);
	updateUniformData();
}
void Camera::updateUniformData()
{	
	mat4 view = mat4(coord_trans) * _model_pitch * _model_yaw;
	mat4 worldview = (view * _world).transpose();
	uint16_t dsize = 2*SIZE_F_MAT4 + 4 + 4 + 2;
	GLfloat data[dsize];

	// Copy camera data into buffers
	memcpy(data, worldview.data(), SIZE_F_MAT4*sizeof(float));
	memcpy(data + SIZE_F_MAT4, _proj.data(), SIZE_F_MAT4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4, vec4(_pos).data(), 4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4 + 3, vec4(view).col(2).data(), 4*sizeof(float));
	data[2*SIZE_F_MAT4 + 8] = _near_dist;
	data[2*SIZE_F_MAT4 + 8 + 1] = _far_dist;

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, dsize*sizeof(float), data, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	return;
}
bool Camera::bindToShader(ShaderProgram shader, const char* name, int binding) {
	GLuint index = glGetUniformBlockIndex(shader.program,name);
	if (index == GL_INVALID_INDEX) {
		printf("Uniform block index for ");
		printf(name); 
		printf(" was not found.\n");
		return false;
	}
	glUniformBlockBinding(shader.program, index, binding);
	glBindBufferBase(GL_UNIFORM_BUFFER,binding,ubo);
	return true;
}
void Camera::rotate(float pitch, float yaw)
{
	if (abs(pitch) > PI) return;
	_model_yaw = mat4(rotatexz<GLfloat>(yaw)) * _model_yaw;
	_model_pitch = mat4(rot_axis(basis[0], -pitch)) * _model_pitch;
}
void Camera::setScreenRatio(int w, int h)
{
	_proj = {
		(float)h / (float)w,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
}
void Camera::translate(vec3 delta)
{
    _pos = _pos + coord_trans * mat3(_model_yaw) * mat3{1,0,0,0,1,0,0,0,1} * delta ;
	_updateTransformations();
}
void Camera::reset()
{
	_pos = { 0,0,0 };
	_model_yaw = mat4{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	_model_pitch = mat4{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	_updateTransformations();
}