struct VS_OUT
{
    float2 tex : TEXCOORD0;
    float4 pos : SV_POSITION;
};

float4 main(VS_OUT input) : SV_TARGET
{
    //return input.col;
    return float4(input.tex.x, input.tex.y, 0.0f, 1.0f);
}