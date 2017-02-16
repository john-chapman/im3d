struct VS_OUTPUT
{
	linear        float4 m_position     : SV_POSITION;
	linear        float3 m_normal       : NORMAL;
};

#ifdef VERTEX_SHADER
	cbuffer cbContextData : register(b0)
	{
		float4x4 uWorldMatrix;
		float4x4 uViewProjMatrix;
	};
	
	struct VS_INPUT
	{
		float3 m_position : POSITION;
		float3 m_normal   : NORMAL;
	};
	
	VS_OUTPUT main(VS_INPUT _in) 
	{
		VS_OUTPUT ret;
		ret.m_normal = mul(uWorldMatrix, float4(_in.m_normal, 0.0));
		ret.m_position = mul(uViewProjMatrix, mul(uWorldMatrix, float4(_in.m_position, 1.0)));
		return ret;
	}
#endif

#ifdef PIXEL_SHADER
	float4 main(VS_OUTPUT _in): SV_Target
	{
		float3 nrm = normalize(_in.m_normal);
		float3 ldir = normalize(float3(1.0, 1.0, 1.0));
		float ndotl = max(dot(nrm, ldir), 0.0) * 0.5;
		return float4(ndotl, ndotl, ndotl, 1.0);
	}
#endif
