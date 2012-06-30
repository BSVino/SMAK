uniform vec4 vecColor;
uniform vec3 vecLightDirection;

uniform vec3 clrMaterialAmbient;
uniform vec3 clrMaterialDiffuse;
uniform vec3 clrMaterialEmissive;
uniform vec3 clrMaterialSpecular;
uniform float flMaterialShininess;
uniform vec3 clrLightAmbient;
uniform vec3 clrLightDiffuse;
uniform vec3 clrLightSpecular;

in vec3 vecPosition;
in vec3 vecNormal;
in vec3 vecTangent;
in vec3 vecBitangent;
in vec2 vecTexCoord0;
in vec3 vecVertexColor;

out vec3 vecFragmentPosition;
out vec3 vecFragmentNormal;
out vec3 vecFragmentTangent;
out vec3 vecFragmentBitangent;
out vec2 vecFragmentTexCoord0;
out vec3 vecFragmentColor;

out vec3 vecCameraDirection;
out vec3 vecLocalLightDirection;
out vec3 vecLocalLightHalf;
out vec3 clrAmbientLight;
out vec3 clrDiffuseLight;
out vec3 clrSpecularLight;

void main()
{
	vec4 vecUnprojected = mView * mGlobal * vec4(vecPosition, 1.0);

	vecCameraDirection = -normalize(vec3(vecUnprojected));
	vecLocalLightDirection = -vec3(mView * mGlobal * vec4(vecLightDirection, 0.0));
	vecLocalLightHalf = vecCameraDirection + vecLocalLightDirection;

	clrAmbientLight = clrLightAmbient * clrMaterialAmbient;
	clrDiffuseLight = clrLightDiffuse * clrMaterialDiffuse;
	clrSpecularLight = clrLightSpecular * clrMaterialSpecular;

	gl_Position = mProjection * vecUnprojected;

	vecFragmentPosition = vecPosition;
	vecFragmentNormal = normalize(vecNormal);
	vecFragmentTangent = normalize(vecTangent);
	vecFragmentBitangent = normalize(vecBitangent);
	vecFragmentTexCoord0 = vec2(vecTexCoord0.x, 1-vecTexCoord0.y);
	vecFragmentColor = vecVertexColor;
}
