uniform vec4 vecDimensions;

uniform bool bDiffuse = false;
uniform tsampler2D iDiffuse;
uniform vec4 vecColor;

uniform bool bScissor = false;
uniform vec4 vecScissor;

uniform int iBorder = 0;
uniform bool bHighlight = false;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentColor;

void main()
{
	vec4 vecDiffuse = vecColor;

	if (bDiffuse)
		vecDiffuse *= ttexture(iDiffuse, vecFragmentTexCoord0);

	if (bScissor)
	{
		if (vecFragmentPosition.x < vecScissor.x)
			discard;
		if (vecFragmentPosition.y < vecScissor.y)
			discard;
		if (vecFragmentPosition.x > vecScissor.x+vecScissor.z)
			discard;
		if (vecFragmentPosition.y > vecScissor.y+vecScissor.w)
			discard;

		/*if (vecFragmentPosition.x > vecScissor.x &&
			vecFragmentPosition.y > vecScissor.y &&
			vecFragmentPosition.x < vecScissor.x+vecScissor.z &&
			vecFragmentPosition.y < vecScissor.y+vecScissor.w)
		{
			vecDiffuse += vec4(0.5, 0.5, 0.0, 0.0);
		}*/
	}

	if (iBorder > 0)
	{
		float x = vecDimensions.x;
		float y = vecDimensions.y;
		float w = vecDimensions.z;
		float h = vecDimensions.w;

		float flBorderStrength = 0.2;
		float flDistanceLeft = RemapVal(abs(x - vecFragmentPosition.x), 0.0, iBorder, flBorderStrength, 0.0);
		float flDistanceTop = RemapVal(abs(y - vecFragmentPosition.y), 0.0, iBorder, flBorderStrength, 0.0);
		float flDistanceRight = RemapVal(abs(x+w - vecFragmentPosition.x), 0.0, iBorder, flBorderStrength, 0.0);
		float flDistanceBottom = RemapVal(abs(y+h - vecFragmentPosition.y), 0.0, iBorder, flBorderStrength, 0.0);

		float flDistance = max(max(max(flDistanceLeft, flDistanceRight), flDistanceTop), flDistanceBottom);
		if (flDistance > 0.0)
		{
			vecDiffuse.r += flDistance;
			vecDiffuse.g += flDistance;
			vecDiffuse.b += flDistance;
			vecDiffuse.a += flDistance * 1.0/flBorderStrength;
		}
	}

	vecOutputColor = vecDiffuse;

	if (vecColor.a > 0 && bHighlight)
	{
		float y = vecDimensions.y;
		float m = vecDimensions.x + vecDimensions.z/2;	// Midpoint

		float flDistance = abs(y - vecFragmentPosition.y);
		if (flDistance < 250.0)
		{
			float flAdd = RemapVal(flDistance, 0.0, 250.0, 0.1, 0.01);
			float flGlow = RemapVal(LengthSqr(vecFragmentPosition.xy - vec2(m, y)), 0.0, 300.0*300.0, 0.08, 0.0);
			if (flGlow > 0)
				flAdd += flGlow;
			vecOutputColor.r += flAdd;
			vecOutputColor.g += flAdd;
			vecOutputColor.b += flAdd;
		}
	}

	//vecOutputColor += vec4(0.3, 0.3, 0.3, 0.3);
}
