uniform vec4 vecColor = vec4(1.0, 1.0, 1.0, 1.0);

uniform float flAlpha;

uniform bool bLight;
uniform vec3 clrLightAmbient;
uniform vec3 clrLightDiffuse;
uniform vec3 clrGlobalAmbient;

in vec3 vecCameraDirection;
in vec3 vecLocalLightDirection;
in vec3 vecLocalLightHalf;

in vec3 vecFragmentPosition;
in vec3 vecFragmentNormal;

uniform mat4 mView;
uniform mat4 mGlobal;

void main()
{
	vec4 vecDiffuse = vecColor;

	vecOutputColor.a *= flAlpha;

	vec3 vecFragmentNormalized = normalize((mView * mGlobal * vec4(vecFragmentNormal, 0.0)).xyz);
	vec3 vecLightVectorNormalized = normalize(vecLocalLightDirection);
	vec3 vecLightHalfVectorNormalized = normalize(vecLocalLightHalf);
	vec3 vecCameraDirectionNormalized = normalize(vecCameraDirection);

	float flDot = dot(vecFragmentNormalized, vecCameraDirectionNormalized);
	vecOutputColor.xyz += RemapVal(Lerp(RemapValClamped(flDot, -1.0, 1.0, 0.0, 1.0), 0.4), 0.0, 1.0, 0.3, 0.8);

	if (bLight)
	{
		float flLightStrength = RemapVal(clamp(dot(vecLocalLightDirection, vecFragmentNormalized), 0.0, 1.0), 0.0, 1.0, 0.3, 2.0);
		vec3 clrLight = clrGlobalAmbient + clrLightAmbient + clrLightDiffuse * flLightStrength;
		vecOutputColor.xyz *= clrLight;
	}
}
