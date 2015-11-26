uniform mat4 proj_modelview;

layout(location=0) in vec2 position;

void main(void)
{
	gl_Position = proj_modelview*vec4(position, 0, 1);
}
