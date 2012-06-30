uniform vec4 vecColor;

uniform vec4 vecDimensions;

uniform bool bTexCoords;
uniform vec4 vecTexCoords;

in int iVertex;
in vec3 vecPosition;
in vec2 vecTexCoord0;

out vec3 vecFragmentPosition;
out vec3 vecFragmentNormal;
out vec2 vecFragmentTexCoord0;

void main()
{
	vec3 vecGLPosition = vecPosition;

	float x = vecDimensions.x;
	float y = vecDimensions.y;
	float w = vecDimensions.z;
	float h = vecDimensions.w;

	if (iVertex == 0)
	{
		vecFragmentNormal = vec3(-0.707, 0.707, 0);
		vecGLPosition = vec3(x, y, 0);
	}
	else if (iVertex == 1)
	{
		vecFragmentNormal = vec3(-0.707, -0.707, 0);
		vecGLPosition = vec3(x, y+h, 0);
	}
	else if (iVertex == 2)
	{
		vecFragmentNormal = vec3(0.707, -0.707, 0);
		vecGLPosition = vec3(x+w, y+h, 0);
	}
	else if (iVertex == 3)
	{
		vecFragmentNormal = vec3(0.707, 0.707, 0);
		vecGLPosition = vec3(x+w, y, 0);
	}

	vecFragmentPosition = vecGLPosition;
	gl_Position = mProjection * mGlobal * vec4(vecGLPosition, 1.0);

	if (bTexCoords)
	{
		float tx = vecTexCoords.x;
		float ty = vecTexCoords.y;
		float tw = vecTexCoords.z;
		float th = vecTexCoords.w;

		if (iVertex == 0)
			vecFragmentTexCoord0 = vec2(tx, 1-ty);
		else if (iVertex == 1)
			vecFragmentTexCoord0 = vec2(tx, 1-ty-th);
		else if (iVertex == 2)
			vecFragmentTexCoord0 = vec2(tx+tw, 1-ty-th);
		else if (iVertex == 3)
			vecFragmentTexCoord0 = vec2(tx+tw, 1-ty);
	}
	else
		vecFragmentTexCoord0 = vecTexCoord0;
}
