#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}