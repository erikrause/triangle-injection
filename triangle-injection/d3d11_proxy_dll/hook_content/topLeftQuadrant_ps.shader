Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

struct VS_OUT
{
    float2 tex : TEXCOORD0;
    float4 pos : SV_POSITION;
};

// Shader that takes top-left quadrant of texture.
float4 main(VS_OUT input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.tex / 2);

}