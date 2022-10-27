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
    //return input.col;
    //return float4(input.tex.x, input.tex.y, 0.0f, 1.0f);
    float4 color = BackBuffer.Sample(Sampler, input.tex / 2);
    color.z = 1.0f;
    return color;
}