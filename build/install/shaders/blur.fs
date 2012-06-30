uniform tsampler2D iSource;
uniform float aflCoefficients[3];
uniform float flOffsetX;
uniform float flOffsetY;

in vec2 vecFragmentTexCoord0;

void main(void)
{
	vec2 vecTC = vecFragmentTexCoord0;
	vec2 vecOffset = vec2(flOffsetX, flOffsetY);

	vec4 vecColorSum;
	vecColorSum  = aflCoefficients[0] * ttexture(iSource, vecTC - vecOffset);
	vecColorSum += aflCoefficients[1] * ttexture(iSource, vecTC);
	vecColorSum += aflCoefficients[2] * ttexture(iSource, vecTC + vecOffset);

	vecOutputColor = vecColorSum;
}
