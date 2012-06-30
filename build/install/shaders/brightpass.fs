uniform tsampler2D iSource;
uniform float flBrightness;
uniform float flScale;

in vec2 vecFragmentTexCoord0;

void main(void)
{
	vec4 vecFragmentColor = ttexture(iSource, vecFragmentTexCoord0);

	float flValue = vecFragmentColor.x;
	if (vecFragmentColor.y > flValue)
		flValue = vecFragmentColor.y;
	if (vecFragmentColor.z > flValue)
		flValue = vecFragmentColor.z;

	if (flValue < flBrightness && flValue > flBrightness - 0.2)
	{
		float flStrength = RemapVal(flValue, flBrightness - 0.2, flBrightness, 0.0, 1.0);
		vecFragmentColor = vecFragmentColor*flStrength;
	}
	else if (flValue < flBrightness - 0.2)
		vecFragmentColor = vec4(0.0, 0.0, 0.0, 0.0);

	vecOutputColor = vecFragmentColor*flScale;
	vecOutputColor.a = 1.0;
}
