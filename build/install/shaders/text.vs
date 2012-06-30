in vec3 vecPosition;
in vec2 vecTexCoord0;

out vec3 vecFragmentPosition;
out vec2 vecFragmentTexCoord0;

void main()
{
	vecFragmentPosition = vecPosition;
	gl_Position = mProjection * mView * mGlobal * vec4(vecPosition, 1);

	vecFragmentTexCoord0 = vecTexCoord0;
}
