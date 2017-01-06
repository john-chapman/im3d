noperspective in vec3 vUv;

layout(location=0) out vec4 fResult;

void main()
{
	fResult = vec4(vUv, 1.0);
}
