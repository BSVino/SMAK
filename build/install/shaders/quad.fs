uniform sampler2D iDiffuse;
uniform vec4 vecColor = vec4(1.0, 1.0, 1.0, 1.0);

uniform float flAlpha;

in vec2 vecFragmentTexCoord0;

void main()
{
	vec4 vecDiffuse = vecColor * texture(iDiffuse, vecFragmentTexCoord0);

	vecOutputColor = vecDiffuse;
}
