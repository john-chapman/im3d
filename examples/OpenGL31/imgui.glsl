#ifdef VERTEX_SHADER
	in vec2 aPosition;
	in vec2 aTexcoord;
	in vec4 aColor;
	
	uniform mat4 uProjMatrix;
	
	noperspective out vec2 vUv;
	noperspective out vec4 vColor;
	
	void main() 
	{
		vUv         = aTexcoord;
		vColor      = aColor;
		gl_Position = uProjMatrix * vec4(aPosition.xy, 0.0, 1.0);
	}
#endif

#ifdef FRAGMENT_SHADER
	noperspective in vec2 vUv;
	noperspective in vec4 vColor;
	
	uniform sampler2D txTexture;
	
	out vec4 fResult;
	
	void main() 
	{
		fResult = vColor;
		fResult.a *= texture(txTexture, vUv).r;
	}
#endif
