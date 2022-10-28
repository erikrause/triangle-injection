#ifndef NUMX
#define NUMX 8
#endif

#ifndef NUMY
#define NUMY 8
#endif

#ifndef NUMZ
#define NUMZ 1
#endif


Texture2D<float4> OriginTexture : register(t0);
RWTexture2D<float4> UpsampledTexture : register(u0);
//SamplerState Sampler : register(s0);

[numthreads(NUMX, NUMY, NUMZ)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float x = DTid.x;
    float y = DTid.y;
    
    float4 test = OriginTexture[float2(x, y)];
    
    UpsampledTexture[float2(x, y)] = test; //OriginTexture[float2(x, y)];

}