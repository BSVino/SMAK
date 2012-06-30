uniform mat4 mBiasedLightMatrix;

in vec3 vecPosition;
in vec3 vecNormal;
in vec2 vecTexCoord0;
in vec3 vecVertexColor;

out vec3 vecFragmentPosition;
out vec3 vecFragmentNormal;
out vec4 vecShadowCoord;
out vec2 vecFragmentTexCoord;

void main()
{
	vecShadowCoord = mBiasedLightMatrix * vec4(vecPosition, 1.0);
	vecFragmentNormal = vecNormal;
	gl_Position = vec4(vecTexCoord0.x*2.0-1.0, vecTexCoord0.y*2.0-1.0, 0.0, 1.0);
	vecFragmentTexCoord = vecTexCoord0;
}
