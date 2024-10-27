const char* shadowmapWaterVS = R"(
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
	float4 Diffuse : COLOR0; // Add this line
};

struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 Diffuse : COLOR0; // Add this line
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
};


float4x4 Transform : register(c20);
float4x4 ViewProj : register(c4);
float4x4 LightViewProj1 : register(c8);
float4x4 LightViewProj2 : register(c16);

// Parameters for the ripple effect
float Time : register(c30);
float RippleSpeed : register(c31);
float RippleHeight : register(c32);
float RippleFrequency : register(c33);
float3 RippleOrigin : register(c34);

// Noise function for more organic ripples
float noise(float2 p)
{
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

PS_INPUT main(VS_INPUT input)
{
    float dist = length(input.Position.xy - RippleOrigin.xy);

    float noiseFactor = noise(input.Position.xy * 0.1) * 2.0 - 1.0;
    float ripple1 = RippleHeight /10 * sin(dist * RippleFrequency - Time * RippleSpeed + noiseFactor);
    float ripple2 = RippleHeight / 10 * sin(dist * (RippleFrequency * 0.75) - Time * (RippleSpeed * 1.2) + noiseFactor);

    input.Position.y += ripple1 + ripple2;

    PS_INPUT output;
    output.WorldPos = mul(float4(input.Position, 1.0f), Transform);
    output.LightViewPosition1 = mul(output.WorldPos, LightViewProj1);
    output.LightViewPosition2 = mul(output.WorldPos, LightViewProj2);
    output.Position = mul(output.WorldPos, ViewProj);

    float delta1 = RippleFrequency * RippleHeight/10 * cos(dist * RippleFrequency - Time * RippleSpeed);
    float delta2 = RippleFrequency * 0.75 * RippleHeight / 10 * cos(dist * (RippleFrequency * 0.75) - Time * (RippleSpeed * 1.2));
    float3 disturbedNormal = float3(-delta1 - delta2, 1.0f, -delta1 - delta2);
    output.Normal = normalize(mul(disturbedNormal + input.Normal, (float3x3) Transform));

    output.UV = input.UV;
	output.Diffuse = input.Diffuse;
    return output;
}

)";
