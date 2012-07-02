uniform vec4 vecDimensions;
uniform vec4 clrSelected;
uniform vec3 hslSelected;
uniform vec3 vecMouse;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentColor;

void main()
{
	vec3 clrRGB = GetRGBFromHSL(hslSelected.x, hslSelected.y, 0.5);

	if (vecFragmentPosition.x < vecDimensions.x + 7.0 || vecFragmentPosition.x > vecDimensions.x + vecDimensions.z - 7.0)
	{
		float flBar = RemapVal(hslSelected.z, 1.0, 0.0, vecDimensions.y, vecDimensions.y+vecDimensions.w);
		if (abs(vecFragmentPosition.y - flBar) < 6.0)
			vecOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		else
			discard;
	}
	else
	{
		if (vecFragmentTexCoord0.y < 0.5)
		{
			float flRamp = vecFragmentTexCoord0.y*2.0; //RemapVal(vecFragmentTexCoord0.y, 0.0, 0.5, 0.0, 1.0);

			vec3 vecColor = flRamp * clrRGB + (1.0-flRamp) * vec3(1.0, 1.0, 1.0);

			vecOutputColor = vec4(vecColor, 1.0);
		}
		else
		{
			float flRamp = RemapVal(vecFragmentTexCoord0.y, 0.5, 1.0, 1.0, 0.0);

			vec3 vecColor = flRamp * clrRGB + (1.0-flRamp) * vec3(0.0, 0.0, 0.0);

			vecOutputColor = vec4(vecColor, 1.0);
		}

		float flBar = RemapVal(hslSelected.z, 1.0, 0.0, vecDimensions.y, vecDimensions.y+vecDimensions.w);
		if (abs(vecFragmentPosition.y - flBar) < 3.0)
			vecOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
