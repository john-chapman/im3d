struct PS_INPUT
{
	float4 m_position : SV_POSITION;
	float4 m_color    : COLOR0;
	float2 m_uv       : TEXCOORD0;
};
	
#ifdef VERTEX_SHADER
	cbuffer vertexBuffer : register(b0)
	{
		float4x4 projMatrix;
	};
	struct VS_INPUT
	{
		float2 m_position : POSITION;
		float4 m_color    : COLOR0;
		float2 m_uv       : TEXCOORD0;
	};

	PS_INPUT main(VS_INPUT _vin)
	{
		PS_INPUT ret;
		ret.m_position = mul(projMatrix, float4(_vin.m_position.xy, 0.0f, 1.0f));
		ret.m_color    = _vin.m_color;
		ret.m_uv       = _vin.m_uv;
		return ret;
	}
#endif

#ifdef PIXEL_SHADER
	Texture2D texture0;
	sampler   sampler0;

	float4 main(PS_INPUT _pin) : SV_Target
	{
		float4 ret = _pin.m_color;
		ret.a *= texture0.Sample(sampler0, _pin.m_uv).r;
		return ret;
	}
#endif
