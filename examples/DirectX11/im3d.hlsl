#if !defined(POINTS) && !defined(LINES) && !defined(TRIANGLES)
	#error No primitive type defined
#endif
#if !defined(VERTEX_SHADER) && !defined(GEOMETRY_SHADER) && !defined(PIXEL_SHADER)
	#error No shader stage defined
#endif

struct VS_OUTPUT
{
	linear        float4 m_position     : SV_POSITION;
	linear        float4 m_color        : COLOR;
	linear        float2 m_uv           : TEXCOORD;
	noperspective float  m_size         : SIZE;
	noperspective float  m_edgeDistance : EDGE_DISTANCE;
};

#define kAntialiasing 2.0

#ifdef VERTEX_SHADER
	cbuffer cbContextData : register(b0)
	{
		float4x4 uViewProjMatrix;
		float2   uViewport;
	};
	
	struct VS_INPUT
	{
		float4 m_positionSize : POSITION_SIZE;
		float4 m_color        : COLOR;
	};
	
	VS_OUTPUT main(VS_INPUT _in) 
	{
		VS_OUTPUT ret;
		ret.m_color = _in.m_color.abgr; // swizzle to correct endianness
		#if !defined(TRIANGLES)
			ret.m_color.a *= smoothstep(0.0, 1.0, _in.m_positionSize.w / kAntialiasing);
		#endif
		ret.m_size = max(_in.m_positionSize.w, kAntialiasing);
		ret.m_position = mul(uViewProjMatrix, float4(_in.m_positionSize.xyz, 1.0));
		return ret;
	}
#endif

#ifdef GEOMETRY_SHADER
	cbuffer cbContextData : register(b0)
	{
		float4x4 uViewProjMatrix;
		float2   uViewport;
	};

	#if defined(POINTS)
	 // expand point -> triangle strip (quad)
		[maxvertexcount(4)]
		void main(point VS_OUTPUT _in[1], inout TriangleStream<VS_OUTPUT> out_)
		{
			VS_OUTPUT ret;
			
			float2 scale = 1.0 / uViewport * _in[0].m_size;
			ret.m_size  = _in[0].m_size;
			ret.m_color = _in[0].m_color;
			ret.m_edgeDistance = _in[0].m_edgeDistance;
			
			ret.m_position = float4(_in[0].m_position.xy + float2(-1.0, -1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
			ret.m_uv = float2(0.0, 0.0);
			out_.Append(ret);
			
			ret.m_position = float4(_in[0].m_position.xy + float2( 1.0, -1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
			ret.m_uv = float2(1.0, 0.0);
			out_.Append(ret);
			
			ret.m_position = float4(_in[0].m_position.xy + float2(-1.0,  1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
			ret.m_uv = float2(0.0, 1.0);
			out_.Append(ret);
			
			ret.m_position = float4(_in[0].m_position.xy + float2( 1.0,  1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
			ret.m_uv = float2(1.0, 1.0);
			out_.Append(ret);
		}
	
	#elif defined(LINES)
	 // expand line -> triangle strip
		[maxvertexcount(4)]
		void main(line VS_OUTPUT _in[2], inout TriangleStream<VS_OUTPUT> out_)
		{
			float2 pos0 = _in[0].m_position.xy / _in[0].m_position.w;
			float2 pos1 = _in[1].m_position.xy / _in[1].m_position.w;
		
			float2 dir = pos0 - pos1;
			dir = normalize(float2(dir.x, dir.y * uViewport.y / uViewport.x)); // correct for aspect ratio
			float2 tng0 = float2(-dir.y, dir.x);
			float2 tng1 = tng0 * _in[1].m_size / uViewport;
			tng0 = tng0 * _in[0].m_size / uViewport;
		
			VS_OUTPUT ret;
			
		 // line start
			ret.m_size = _in[0].m_size;
			ret.m_color = _in[0].m_color;
			ret.m_uv = float2(0.0, 0.0);
			ret.m_position = float4((pos0 - tng0) * _in[0].m_position.w, _in[0].m_position.zw); 
			ret.m_edgeDistance = -_in[0].m_size;
			out_.Append(ret);
			ret.m_position = float4((pos0 + tng0) * _in[0].m_position.w, _in[0].m_position.zw);
			ret.m_edgeDistance = _in[0].m_size;
			out_.Append(ret);
			
		 // line end
			ret.m_size = _in[1].m_size;
			ret.m_color = _in[1].m_color;
			ret.m_uv = float2(1.0, 1.0);
			ret.m_position = float4((pos1 - tng1) * _in[1].m_position.w, _in[1].m_position.zw);
			ret.m_edgeDistance = -_in[1].m_size;
			out_.Append(ret);
			ret.m_position = float4((pos1 + tng1) * _in[1].m_position.w, _in[1].m_position.zw);
			ret.m_edgeDistance = _in[1].m_size;
			out_.Append(ret);
		}
	
	#endif
#endif

#ifdef PIXEL_SHADER
	float4 main(VS_OUTPUT _in): SV_Target
	{
		float4 ret = _in.m_color;
		
		#if   defined(LINES)
			float d = abs(_in.m_edgeDistance) / _in.m_size;
			d = smoothstep(1.0, 1.0 - (kAntialiasing / _in.m_size), d);
			ret.a *= d;
			
		#elif defined(POINTS)
			float d = length(_in.m_uv - float2(0.5, 0.5));
			d = smoothstep(0.5, 0.5 - (kAntialiasing / _in.m_size), d);
			ret.a *= d;
			
		#endif
		
		return ret;
	}
#endif
