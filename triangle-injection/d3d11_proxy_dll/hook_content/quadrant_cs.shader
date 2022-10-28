#ifndef NUMX
#define NUMX 8
#endif

#ifndef NUMY
#define NUMY 8
#endif

#ifndef NUMZ
#define NUMZ 1
#endif

RWTexture2D<float4> UpsampledTexture : register(u0);
RWTexture2D<float4> OutputTexture : register(u1);


[numthreads(NUMX, NUMY, NUMZ)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float x = DTid.x;
    float y = DTid.y;
    
    OutputTexture[float2(x, y)] = UpsampledTexture[float2(x, y)];

}