uniform sampler2D iAOMap;

in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentColor;

void main()
{
	vec4 vecColor = texture2D(iAOMap, vecFragmentTexCoord0);
	if (vecColor.a == 0.0)	// No samples
		vecOutputColor = vec4(0.0, 0.0, 0.0, 0.2);
	else
		vecOutputColor = vec4(vecColor.x/vecColor.a, vecColor.y/vecColor.a, vecColor.z/vecColor.a, 1.0);
}
