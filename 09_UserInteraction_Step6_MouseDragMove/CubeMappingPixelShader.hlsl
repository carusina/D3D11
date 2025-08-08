TextureCube g_textureCube0 : register(t0);
SamplerState g_sampler : register(s0);

struct CubeMappingPixelShaderInput {
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
    float4 indexColor : SV_Target1;
};

PixelShaderOutput main(CubeMappingPixelShaderInput input)
{
    PixelShaderOutput output;
    output.pixelColor = g_textureCube0.Sample(g_sampler, input.posModel.xyz);
    output.indexColor = float4(0.0, 0.0, 1.0, 0.0);
    
    return output;
}