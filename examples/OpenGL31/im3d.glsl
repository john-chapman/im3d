#if !defined(POINTS) && !defined(LINES) && !defined(TRIANGLES)
	#error No primitive type defined
#endif
#if !defined(VERTEX_SHADER) && !defined(FRAGMENT_SHADER)
	#error No shader stage defined
#endif

#define kAntialiasing 2.0

#ifdef VERTEX_SHADER
	struct VertexData
	{
		vec4 m_positionSize;
		uint m_color;
	};
	uniform VertexDataBlock
	{
		VertexData uVertexData[(64 * 1024) / 32]; // assume a 64kb block size
	};

	uniform mat4 uViewProjMatrix;
	
	in vec4 aPosition;
	
	noperspective out  float vEdgeDistance;
	noperspective out  float vSize;
	smooth out         vec4  vColor;
	
	vec4 UintToRgba(uint _u)
	{
		vec4 ret;
		return ret;
	}
	
	void main() 
	{
		int vid = gl_InstanceID;
		#if   defined(POINTS)
		#elif defined(LINES)
			vid *= 2;
		#elif defined(TRIANGLES)
			vid *= 3;
		#endif
	
		float size = uVertexData[vid].m_positionSize.w;
		vSize = max(size, kAntialiasing);
		vColor = UintToRgba(uVertexData[vid].m_color);
		#if !defined(TRIANGLES)
			vColor.a *= smoothstep(0.0, 1.0, size / kAntialiasing);
		#endif
		
		gl_Position = uViewProjMatrix * vec4(uVertexData[vid].m_positionSize.xyz, 1.0);
		gl_Position.xy += aPosition.xy * 0.01;
	}
#endif

#ifdef FRAGMENT_SHADER
	noperspective in float vEdgeDistance;
	noperspective in float vSize;
	smooth        in vec4  vColor;
	
	out vec4 fResult;
	
	void main() 
	{
		fResult = vColor;
		
		#if   defined(LINES)
			float d = abs(vEdgeDistance) / vSize;
			d = smoothstep(1.0, 1.0 - (kAntialiasing / vSize), d);
			fResult.a *= d;
			
		#elif defined(POINTS)
			float d = length(gl_PointCoord.xy - vec2(0.5));
			d = smoothstep(0.5, 0.5 - (kAntialiasing / vSize), d);
			fResult.a *= d;
			
		#endif		
	}
#endif
