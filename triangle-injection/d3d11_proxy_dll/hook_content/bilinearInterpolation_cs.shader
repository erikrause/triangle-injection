#ifndef NUMX
#define NUMX 8
#endif

#ifndef NUMY
#define NUMY 8
#endif

#ifndef NUMZ
#define NUMZ 1
#endif


#define TexDimX 1024
#define TexDimY 768
#define Factor 2


Texture2D<float4> OriginTexture : register(t0);
RWTexture2D<float4> UpsampledTexture : register(u0);
SamplerState Sampler : register(s0);

//float4 SampleTexture(uint2 coord)
//{
//    uint data = UpsampledTexture[coord];
    
//    float ch1, ch2, ch3, ch4;
//    ch1 = data >> 24;
//    ch2 = (data & 0x00ff0000) >> 16;
//    ch3 = (data & 0x0000ff00) >> 8;
//    ch4 = (data & 0x000000ff);
    
//    return float4(ch1, ch2, ch3, ch4);
//}

float4 SampleTexture(uint2 coords)
{
    float2 uv = float2(coords.x / 1024.f, coords.y / 768.f);
    return OriginTexture.SampleLevel(Sampler, uv, 0);
}

// discreteXFloor & discreteXCeil is just precalculated floor & ceil values respectively (for perfomance).
float4 R(float x, uint discreteY, uint discreteXFloor, uint discreteXCeil) //, float xFloorDif, float xCeilDif)
{
    return (discreteXCeil - x) * SampleTexture(uint2(discreteXFloor, discreteY)) +
           (x - discreteXFloor) * SampleTexture(uint2(discreteXCeil, discreteY)); 

}

[numthreads(NUMX, NUMY, NUMZ)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 upsampledCoord = DTid.xy;
    float2 interpolatedCoord = float2(DTid.xy) / float2(Factor, Factor);
    
    uint2 floorCoord = floor(interpolatedCoord);
    uint2 ceilCoord = interpolatedCoord + uint2(1, 1); //ceil(interpolatedCoord);
    
    //// precalculated coord diff (for perfomance): TODO
    //float2 floorCoordDiff = interpolatedCoord - float2(floorCoord);
    //float2 ceilCoordDiff = float2(ceilCoord) - interpolatedCoord;
    
    float4 r1 = R(interpolatedCoord.x, floorCoord.y, floorCoord.x, ceilCoord.x);
    float4 r2 = R(interpolatedCoord.x, ceilCoord.y, floorCoord.x, ceilCoord.x);
    
    // Final interpolation between r1 and r2:
    float4 result = (ceilCoord.y - interpolatedCoord.y) * r1 +
                    (interpolatedCoord.y - floorCoord.y) * r2;
    
    //float2 texCoord = float2(upsampledCoord.x / 2048.f, upsampledCoord.y / 1536.f);
    //result = OriginTexture.SampleLevel(Sampler, texCoord, 0);

    UpsampledTexture[upsampledCoord] = result;
}