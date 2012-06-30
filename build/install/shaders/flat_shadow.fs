uniform sampler2D iShadowMap;
uniform vec3 vecLightNormal;
uniform bool bOccludeAll;
uniform float flTime;	// Used as a random seed

in vec3 vecFragmentPosition;
in vec3 vecFragmentNormal;
in vec4 vecShadowCoord;
in vec2 vecFragmentTexCoord;

void main()
{
	float flLightDot = dot(vecLightNormal, normalize(vecFragmentNormal));

	// If the face is facing away from the light source, don't include it in the sample.
	if (flLightDot > 0.0)
	{
		vecOutputColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	else
	{
		float flShadow = 1.0;
		if (bOccludeAll)
			flShadow = 0.0;
		else if (vecShadowCoord.w > 0.0)
		{
			vec4 vecShadowCoordinateWdivide = vecShadowCoord / vecShadowCoord.w;

			// Randomize a tad to help reduce moire
			float flRandX = (Rand(vecFragmentTexCoord + flTime*2)-0.5)/500.0;
			float flRandY = (Rand(vecFragmentTexCoord + flTime*3)-0.5)/500.0;
			float flDistanceFromLight = texture2D(iShadowMap, vecShadowCoordinateWdivide.st+vec2(flRandX, flRandY)).z;

			// Reduce moire and self-shadowing
			vecShadowCoordinateWdivide.z -= 0.003 + 0.004*(1.0-flLightDot);	// Use flLightDot to get further away from surfaces at high angles.
			flDistanceFromLight += (Rand(vecFragmentTexCoord + flTime)-0.5)/100.0;

			// It's .99 because sometimes if every sample on a point is perfectly white it suffers integer overflow down the line and becomes black.
			flShadow = flDistanceFromLight < vecShadowCoordinateWdivide.z?0.0:0.99;
		}

		vecOutputColor = vec4(flShadow, flShadow, flShadow, 1.0);
	}
}
