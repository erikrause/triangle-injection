struct VS_IN
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct VS_OUT
{
    float2 tex : TEXCOORD0;
    float4 pos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
    VS_OUT OUT;
    OUT.pos = float4(input.pos, 1.0);
    OUT.tex = input.tex;
    return OUT;
}