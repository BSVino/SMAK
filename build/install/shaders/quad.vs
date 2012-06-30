in vec3 vecPosition;
in vec2 vecTexCoord0;

out vec2 vecFragmentTexCoord0;

void main()
{
	gl_Position = vec4(vecPosition, 1);

	vecFragmentTexCoord0 = vec2(vecTexCoord0.x, 1-vecTexCoord0.y);
}
