#ifdef VERTEX_SHADER
	uniform mat4 uWorldMatrix;
	uniform mat4 uViewProjMatrix;
	
	layout(location=0) in vec3 aPosition;
	layout(location=1) in vec3 aNormal;
	
	smooth out vec3 vNormalW;
	
	void main() 
	{
		vNormalW = mat3(uWorldMatrix) * aNormal;
		gl_Position = uViewProjMatrix * (uWorldMatrix * vec4(aPosition, 1.0));
	}
#endif

#ifdef FRAGMENT_SHADER
	smooth in vec3 vNormalW;
	
	layout(location=0) out vec3 fResult;
	
	void main() 
	{
		vec3 nrm = normalize(vNormalW);
		vec3 ldir = normalize(vec3(1.0));
		float ndotl = max(dot(nrm, ldir), 0.0) * 0.5;
		fResult = vec3(ndotl);
	}
#endif
