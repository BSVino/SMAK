in vec3 vecFragmentColor;

uniform mat4 mView;
uniform mat4 mGlobal;

void main()
{
	vecOutputColor = vec4(vecFragmentColor, 1.0);
}
