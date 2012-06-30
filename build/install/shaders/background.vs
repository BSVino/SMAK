in vec3 vecPosition;
in vec2 vecTexCoord0;

out vec2 vecFragmentTexCoord0;
out vec3 vecFragmentColor;

void main()
{
	gl_Position = vec4(vecPosition, 1);

	vecFragmentTexCoord0 = vec2(vecTexCoord0.x, 1-vecTexCoord0.y);
	if (vecTexCoord0.x < 0.5 && vecTexCoord0.y < 0.5)
		vecFragmentColor = vec3(0.4, 0.4, 0.4);
	else if (vecTexCoord0.x > 0.5 && vecTexCoord0.y < 0.5)
		vecFragmentColor = vec3(0.3, 0.3, 0.3);
	else if (vecTexCoord0.x < 0.5 && vecTexCoord0.y > 0.5)
		vecFragmentColor = vec3(0.3, 0.3, 0.3);
	else
		vecFragmentColor = vec3(0.2, 0.2, 0.2);
}
