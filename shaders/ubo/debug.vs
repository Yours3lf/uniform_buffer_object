#version 420 core

layout(std140) uniform trans_prop
{
	mat4 modelview;
	mat4 proj;
};

layout(location=0) in vec4 in_vertex;

void main()
{
  gl_Position = proj * modelview * in_vertex;
}
