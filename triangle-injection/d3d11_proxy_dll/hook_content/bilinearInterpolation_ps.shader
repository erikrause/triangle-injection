Texture2D BackBuffer : register(t0);
SamplerState Sampler : register(s0);

struct VS_OUT
{
    float2 tex : TEXCOORD0;
    float4 pos : SV_POSITION;
};

// Shader for bilinear upsampling with factor 2.
float4 main(VS_OUT input) : SV_TARGET
{
    float4 color = BackBuffer.Sample(Sampler, input.tex / 2);
    return color;
}