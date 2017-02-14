#if !defined(POINTS) && !defined(LINES) && !defined(TRIANGLES)
	#error No primitive type defined
#endif
#if !defined(VERTEX_SHADER) && !defined(GEOMETRY_SHADER) && !defined(FRAGMENT_SHADER)
	#error No shader stage defined
#endif

struct VS_OUTPUT
{
	float4 m_position     : SV_POSITION;
	float4 m_color        : COLOR0;
	float2 m_uv           : TEXCOORD0;
	float  m_size         : PSIZE0;
	float  m_edgeDistance : EDGE_DISTANCE;
};

#define kAntialiasing 2.0

#ifdef VERTEX_SHADER
	cbuffer cbViewProjMatrix : register(b0)
	{
		float4x4 uViewProjMatrix;
	};
	
	struct VS_INPUT
	{
		float4 m_positionSize : POSITION_SIZE;
		float4 m_color        : COLOR;
	};
	
	VS_OUTPUT main(VS_INPUT _vin) 
	{
		VS_OUTPUT ret;
		ret.m_color = _vin.m_color
		#if !defined(TRIANGLES)
			ret.m_color.a *= smoothstep(0.0, 1.0, _vin.m_positionSize.w / kAntialiasing);
		#endif
		ret.m_size = max(_vin.m_positionSize.w, kAntialiasing);
		ret.m_position = mul(uViewProjMatrix, float4(_vin.m_positionSize.xyz, 1.0);
		return ret;
	}
#endif

#ifdef GEOMETRY_SHADER
	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;
	
	uniform float2 uViewport;
	
	in  VertexData vData[];
	out VertexData vDataOut;
	
	void main() 
	{
		float2 pos0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
		float2 pos1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
		
		float2 dir = pos0 - pos1;
		dir = normalize(float2(dir.x, dir.y * uViewport.y / uViewport.x)); // correct for aspect ratio
		float2 tng0 = float2(-dir.y, dir.x);
		float2 tng1 = tng0 * vData[1].m_size / uViewport;
		tng0 = tng0 * vData[0].m_size / uViewport;
		
		gl_Position = float4((pos0 - tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw); 
		vDataOut.m_edgeDistance = -vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		vDataOut.m_color = vData[0].m_color;
		EmitVertex();
		
		gl_Position = float4((pos0 + tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
		vDataOut.m_color = vData[0].m_color;
		vDataOut.m_edgeDistance = vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		EmitVertex();
		
		gl_Position = float4((pos1 - tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_edgeDistance = -vData[1].m_size;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_color = vData[1].m_color;
		EmitVertex();
		
		gl_Position = float4((pos1 + tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_color = vData[1].m_color;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_edgeDistance = vData[1].m_size;
		EmitVertex();
	}
#endif

#ifdef FRAGMENT_SHADER
	float4 main(VS_OUTPUT _pin) 
	{
		float4 ret = _pin.m_color;
		
		#if   defined(LINES)
			float d = abs(_pin.m_edgeDistance) / _pin.m_size;
			d = smoothstep(1.0, 1.0 - (kAntialiasing / _pin.m_size), d);
			ret.a *= d;
			
		#elif defined(POINTS)
			float d = length(_pin.m_uv - float2(0.5));
			d = smoothstep(0.5, 0.5 - (kAntialiasing / _pin.m_size), d);
			ret.a *= d;
			
		#endif
		
		return ret;
	}
#endif
