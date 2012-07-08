uniform bool bDiffuse = false;
uniform sampler2D iDiffuse;

uniform bool bAO = false;
uniform sampler2D iAO;

uniform bool bNormal = false;
uniform sampler2D iNormal;

uniform bool bNormal2 = false;
uniform sampler2D iNormal2;

uniform bool bCavity = false;
uniform sampler2D iCavity;
float flCavityStrength = 1.0;

uniform float flAlpha;

uniform bool bLight;
uniform bool bShadeBottoms;
uniform vec3 clrRimLight;
uniform vec3 clrMaterialAmbient;
uniform vec3 clrMaterialDiffuse;
uniform vec3 clrMaterialEmissive;
uniform vec3 clrMaterialSpecular;
uniform float flMaterialShininess;

in vec3 vecCameraDirection;
in vec3 vecLocalLightDirection;
in vec3 vecLocalLightHalf;

in vec3 clrAmbientLight;
in vec3 clrDiffuseLight;
in vec3 clrSpecularLight;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentTangent;
in vec3 vecFragmentBitangent;
in vec3 vecFragmentColor;

uniform mat4 mView;
uniform mat4 mGlobal;

void main()
{
	vec4 clrDiffuseColor;
	vec3 clrLight;
	vec3 vecTangentNormal = vec3(0.0, 0.0, 0.0);

	if (bDiffuse)
		clrDiffuseColor = texture(iDiffuse, vecFragmentTexCoord0);
	else
		clrDiffuseColor = vec4(0.8, 0.8, 0.8, 1.0);

	if (bCavity)
	{
		vec4 clrCavity = texture(iCavity, vecFragmentTexCoord0);
		clrCavity = (clrCavity - 0.5) * flCavityStrength + 0.5;
		clrDiffuseColor = clrDiffuseColor * (clrDiffuseColor + (2.0*clrCavity)*(1.0-clrDiffuseColor));
	}

	if (!bNormal && !bNormal2)
		vecTangentNormal = vec3(0.0, 0.0, 1.0);

	if (bNormal)
		vecTangentNormal = normalize(texture2D(iNormal, vecFragmentTexCoord0.xy).xyz * 2.0 - 1.0);

	vec3 vecNormal2;
	if (bNormal2)
	{
		vecNormal2 = normalize(texture2D(iNormal2, vecFragmentTexCoord0.xy).xyz * 2.0 - 1.0);
		if (bNormal)
		{
			// Transform the normal to the tangent space of the first normal map's normal
			vec3 vecSmoothBitangent = normalize(cross(vecTangentNormal, vec3(1, 0, 0)));
			vec3 vecSmoothTangent = normalize(cross(vecSmoothBitangent, vecTangentNormal));
			mat3 mTBN = mat3(vecSmoothTangent, vecSmoothBitangent, vecTangentNormal);
			vecTangentNormal = mTBN * vecNormal2;
		}
		else
		{
			vecTangentNormal = vecNormal2;
		}
	}

	vec3 clrAOColor = vec3(1.0, 1.0, 1.0);
	if (bAO)
		clrAOColor = texture(iAO, vecFragmentTexCoord0).xyz;

	vec3 vecFragmentNormalized = normalize(vecFragmentNormal);
	vec3 vecLightVectorNormalized = normalize(vecLocalLightDirection);
	vec3 vecLightHalfVectorNormalized = normalize(vecLocalLightHalf);
	vec3 vecCameraDirectionNormalized = normalize(vecCameraDirection);
	vec3 vecTranslatedNormal;

	if (bLight)
	{
		if (bNormal || bNormal2)
		{
			// If we are in normal map mode, vecFragmentNormal is really part of the inverse TBN matrix
			mat3 mTBN = mat3(normalize(vecFragmentTangent), normalize(vecFragmentBitangent), vecFragmentNormalized);
			vecTranslatedNormal = normalize((mView * mGlobal * vec4(mTBN * vecTangentNormal, 0.0)).xyz);
		}
		else
			// If we are not in normal map mode, vecFragmentNormal is just a normal normal
			vecTranslatedNormal = (mView * mGlobal * vec4(vecFragmentNormalized, 0.0)).xyz;

		float flLightStrength = clamp(dot(vecLightVectorNormalized, vecTranslatedNormal), 0.0, 1.0);
		float flNormalDotHalfVector = max(0.0, dot(vecTranslatedNormal, vecLightHalfVectorNormalized));
		float flPowerFactor = pow(flNormalDotHalfVector, flMaterialShininess);
		clrLight = clrMaterialEmissive + (clrAmbientLight +
				clrDiffuseLight * flLightStrength) * clrAOColor +
				clrSpecularLight * flPowerFactor;
	}
	else
	{
		vec3 clrDiffuseNoLight;
		if (bShadeBottoms)
		{
			if (bNormal || bNormal2)
			{
				mat3 mTBN = mat3(normalize(vecFragmentTangent), normalize(vecFragmentBitangent), vecFragmentNormalized);
				vecTranslatedNormal = normalize((mView * mGlobal * vec4(mTBN * vecTangentNormal, 0.0)).xyz);
			}
			else
				vecTranslatedNormal = normalize((mView * mGlobal * vec4(vecFragmentNormalized, 0.0)).xyz);

			float flDot = dot(vecTranslatedNormal, vec3(0, 1, 0));
			clrDiffuseNoLight = vec3(1.0, 1.0, 1.0) * (flDot * 0.5) + vec3(0.45, 0.45, 0.45);
		}
		else
			clrDiffuseNoLight = vec3(1.0, 1.0, 1.0);

		vecLightHalfVectorNormalized = normalize(vecCameraDirection + vec3(0, 1, 0));
		float flNormalDotHalfVector = max(0.0, dot(vecTranslatedNormal, vecLightHalfVectorNormalized));
		float flPowerFactor = pow(flNormalDotHalfVector, flMaterialShininess);

		clrLight = clrMaterialEmissive + (clrMaterialAmbient +
				clrMaterialDiffuse * clrDiffuseNoLight) * clrAOColor +
				clrMaterialSpecular * flPowerFactor;
	}

	// Add a rim light.
	if (LengthSqr(clrRimLight) > 0.0)
		clrLight += (RemapValClamped(Lerp(1-dot(vecTranslatedNormal, vecCameraDirectionNormalized), 0.8), 0.6, 1.0, 0.0, 1.0) * clrRimLight);

	vecOutputColor = clrDiffuseColor;
	vecOutputColor.rgb *= clrLight;// * clrAO * clrCAO;
	vecOutputColor.a = clrDiffuseColor.a * flAlpha;

	// A bit of randomness to get rid of the banding.
	float flFragmentRand = Rand(vecFragmentTexCoord0)/250.0;
	vecOutputColor += flFragmentRand;
}
