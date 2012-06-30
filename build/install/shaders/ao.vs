in vec3 vecPosition;
in vec2 vecTexCoord0;
in vec3 vecVertexColor;

out vec2 vecFragmentTexCoord0;
out vec3 vecFragmentColor;

void main()
{
	gl_Position = mProjection * mView * mGlobal * vec4(vecPosition, 1.0);
	vecFragmentColor = vecVertexColor;
	vecFragmentTexCoord0 = vecTexCoord0;
}
