struct PS_INPUT
{
	float4 m_position : SV_POSITION;
	float4 m_color    : COLOR0;
	float2 m_uv       : TEXCOORD0;
};
	
#ifdef VERTEX_SHADER
	cbuffer cbContextData : register(b0)
	{
		float4x4 uProjMatrix;
	};
	struct VS_INPUT
	{
		float2 m_position : POSITION;
		float4 m_color    : COLOR0;
		float2 m_uv       : TEXCOORD0;
	};

	PS_INPUT main(VS_INPUT _in)
	{
		PS_INPUT ret;
		ret.m_position = mul(uProjMatrix, float4(_in.m_position.xy, 0.0f, 1.0f));
		ret.m_color    = _in.m_color;
		ret.m_uv       = _in.m_uv;
		return ret;
	}
#endif

#ifdef PIXEL_SHADER
	Texture2D texture0;
	sampler   sampler0;

	float4 main(PS_INPUT _in) : SV_Target
	{
		float4 ret = _in.m_color;
		ret.a *= texture0.Sample(sampler0, _in.m_uv).r;
		return ret;
	}
#endif
