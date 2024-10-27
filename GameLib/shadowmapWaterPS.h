const char* shadowmapWaterPS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
    float4 Diffuse : COLOR0; // Add this line
};


#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 2
#define SHADOW_PCF_NUM_SAMPLES 7 // 7

float3 MaterialDiffuse : register(c0);
float3 LightDirection : register(c1);
float4 LightDiffuse : register(c2);
float4 AmbientColor : register(c3);
float3 CameraPos : register(c4);
float4 FogColor : register(c5);
float2 FogRange : register(c6);
float3 MaterialSpecular : register(c7);
float Shininess : register(c8);
float3 LightSpecular : register(c9);
int ShadowOpacity : register(c10); // 0..100?
float SpecularLight : register(c11);
float3 OrangeTint : register(c12);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1);
sampler2D ShadowMap2 : register(s2);

static const float PI = 3.1416f;
static const float2 pcfOffsets[7] =
{
    float2(0.0, 0.0), // Center
    float2(1.0, 0.0), // 0 degrees
    float2(0.5, 0.87) * 0.6f, // 60 degrees
    float2(-0.5, 0.87), // 120 degrees
    float2(-1.0, 0.0) * 0.6f, // 180 degrees
    float2(-0.5, -0.87), // 240 degrees
    float2(0.5, -0.87) * 0.6f // 300 degrees
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 foamUV;
	foamUV.x = 0.01 + 0.98 * (input.WorldPos.x * 0.001);
	foamUV.y = 0.01 + 0.98 * (input.WorldPos.y * 0.001);


    
    float4 Color = tex2D(DiffuseTexture, foamUV);
    Color.a *= input.Diffuse.a;

    // Fetch and transform normal from normal map
    float3 NM = 2.0f * tex2D(DiffuseTexture, input.UV).rgb - 1.0f; // Convert from [0, 1] to [-1, 1]
    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 1.0f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias;
    float SampledDepth1 = tex2D(ShadowMap1, LightTexCoord1).r;
    float SampledDepth2 = tex2D(ShadowMap2, LightTexCoord2).r;
    const float squareSize = 0.2f; // Half of the full size
    float2 RedOffset = float2(0.2, 0.2);
    float2 GreenOffset = float2(0.0, 0.0);
    float2 BlueOffset = float2(-0.2, -0.2);
    float4 ChromaticAberrationColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

   
    float bigShadow = 0.0f;
    float localShadow = 0.0f;
    const float texelScale1 = 1.0f / 500.0f;
    const float texelScale2 = 1.0f / 800.0f;
    const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i)
    {
        float2 offset = pcfOffsets[i];
        bigShadow += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
        localShadow += (fCurrDepth2 > tex2D(ShadowMap2, LightTexCoord2 + offset * texelScale2).r ? increment : 0.0f);
    }

	 // Distance to the inner shadow border (0.0 .. 0.5)
    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.2f, borderDist);
	 
	 /*
	 // Visualize map grid (for debug)	 
	 float2 pix = LightTexCoord1 * 2048.0f;
	 pix = pix - floor(pix);
	 pix = 0.5f - abs(pix - 0.5f);
 	 float gridVal = max(pix.x, pix.y);
	 gridVal = smoothstep(0.45f, 0.5f, gridVal);
	 */

    float finalShadow = lerp(bigShadow, localShadow, fade); //(shadow2 + (shadow1 * fade)) / 2.0f;
	 //finalShadow = bigShadow + gridVal * 0.8f;
    float InvertedOpacity = 1.0f - (ShadowOpacity / 120.0f);

	finalShadow = lerp(bigShadow, localShadow, fade); 
	finalShadow *= InvertedOpacity;  // Here, if ShadowOpacity is 100 (full), InvertedOpacity will be 0, hence transparent.

	LightResult *= (1.0f - finalShadow);

    if (SpecularLight > 0.5f)  // Use c11 bool to control specular light
    {

        // Compute specular reflection based on alpha intensity
// Compute specular reflection based on alpha intensity
            float3 V = normalize(CameraPos - input.WorldPos.xyz);
            float3 R = reflect(L, N);
            float SpecularIntensity = Color.a * pow(saturate(dot(V, R)), Shininess);
    
// Modify the chromatic aberration offsets with the SpecularIntensity
            RedOffset *= SpecularIntensity;
            GreenOffset *= SpecularIntensity;
            BlueOffset *= SpecularIntensity;

// Fetch colors with modified UVs
            float4 DiffuseColorR = tex2D(DiffuseTexture, foamUV + RedOffset);
            float4 DiffuseColorG = tex2D(DiffuseTexture, foamUV + GreenOffset);
            float4 DiffuseColorB = tex2D(DiffuseTexture, foamUV + BlueOffset);

// Compute chromatic aberration color
            float4 ChromaticAberrationColor = float4(DiffuseColorR.r, DiffuseColorG.g, DiffuseColorB.b, 1.0f);

// Continue with your lighting and shading logic...

           

        
// Fresnel effect
            float f0 = 0.06;
            float fresnel = f0 + (1.0 - f0) * pow(1.0 - dot(V, N), 5);

// Original white specular
            float3 SpecularWhite = SpecularIntensity * MaterialSpecular * LightSpecular.rgb / 8;

// Orange-tinted specular
            float3 SpecularOrange = SpecularIntensity * MaterialSpecular * LightSpecular.rgb * OrangeTint * 6.2;

// Blend the two specular colors based on the Fresnel term
            float3 SpecularCombined = lerp(SpecularWhite, SpecularOrange, fresnel);
         
            
            if (finalShadow < 0.75f)
            {
                LightResult += SpecularCombined;
                LightResult.rgb *= ChromaticAberrationColor.rgb * 5;
            }
    }
#endif

    if (finalShadow < 0.75f)
        Color.rgb = LightResult;
    else
        Color.rgb*= LightResult;

  // Fog
    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));

    Color.a *= input.Diffuse.a; // Use computed transparency
    return lerp(Color, FogColor, fogFactor);
}
)";
