uniform vec4 vecDimensions;
uniform vec4 clrSelected;
uniform vec3 vecMouse;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentColor;

void main()
{
	float flRadius = vecDimensions.z/2.0;
	float flRadiusSqr = flRadius*flRadius;
	vec2 vecCenter = vec2(vecDimensions.x + flRadius, vecDimensions.y + flRadius);

	vec2 vecFromCenter = vecFragmentPosition.xy - vecCenter;
	float flDistanceSqr = LengthSqr(vecFromCenter);
	if (flDistanceSqr > flRadiusSqr)
		discard;

	float flDistance = sqrt(flDistanceSqr);

	float flTan = atan(vecFromCenter.y, -vecFromCenter.x);

	float flHue = RemapVal(flTan, -PI, PI, 0.0, 360.0/60.0);
	float flSaturation = flDistance/flRadius;
	float flLightness = 0.5;

	vecOutputColor = vec4(GetRGBFromHSL(flHue, flSaturation, flLightness), 1.0);

	vec2 vecPreview;
	vec3 clrPreview;

	vec3 vecHSL = GetHSLFromRGB(clrSelected.rgb);

	float flHueRadians = RemapVal(vecHSL.x, 0.0, 360.0, -PI, PI) - PI/2.0;
	float flRatio = tan(flHueRadians);

	float flDistanceFromCenter = vecHSL.y * flRadius;

	vecPreview = vecCenter + vec2(sin(flHueRadians)*flDistanceFromCenter, cos(flHueRadians)*flDistanceFromCenter);
	clrPreview = clrSelected.rgb;

	vec2 vecFromPreviewCenter = vecFragmentPosition.xy - vecPreview;
	float flPreviewDistanceSqr = LengthSqr(vecFromPreviewCenter);
	if (flPreviewDistanceSqr < 18.0*18.0)
	{
		vecOutputColor.rgb *= 0.1;

		if (flPreviewDistanceSqr < 16.0*16.0)
			vecOutputColor = vec4(clrPreview, 1.0);
	}

	vec2 vecMouseFromCenter = vecMouse.xy - vecCenter;
	float flMouseDistanceSqr = LengthSqr(vecMouseFromCenter);
	if (flMouseDistanceSqr < flRadiusSqr)
	{
		vecPreview = vecMouse.xy;

		flTan = atan(vecMouseFromCenter.y, -vecMouseFromCenter.x);

		flHue = RemapVal(flTan, -PI, PI, 0.0, 360.0/60.0);
		flSaturation = sqrt(flMouseDistanceSqr)/flRadius;

		clrPreview = GetRGBFromHSL(flHue, flSaturation, flLightness);

		vec2 vecFromPreviewCenter = vecFragmentPosition.xy - vecPreview;
		float flPreviewDistanceSqr = LengthSqr(vecFromPreviewCenter);
		if (flPreviewDistanceSqr < 15.0*15.0)
		{
			vecOutputColor.rgb *= 0.1;
			vecOutputColor.rgb += 0.4;

			if (flPreviewDistanceSqr < 14.0*14.0)
				vecOutputColor = vec4(clrPreview, 1.0);
		}
	}
}
