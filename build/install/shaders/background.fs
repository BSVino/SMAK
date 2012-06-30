in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentColor;

void main()
{
	// A bit of randomness to get rid of the banding.
	float flFragmentRand = Rand(vecFragmentTexCoord0)/100.0;
	vecOutputColor = vec4(vecFragmentColor + vec3(flFragmentRand, flFragmentRand, flFragmentRand), 0.0);
}
