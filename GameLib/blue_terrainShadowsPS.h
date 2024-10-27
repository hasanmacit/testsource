const char* blue_terrainShadowsPS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
};

#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 1
#define SHADOW_PCF_NUM_SAMPLES 30 // 7

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
float Time : register(c12); // Add this to the constants at the bottom
float reduceLightIntensity : register(c15);
float enableEffects : register(c13);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1);   // big shadowmap
sampler2D ShadowMap2 : register(s2);   // local (small) shadowmap

static const float PI = 3.1416f;
static const float2 pcfOffsets[30] = {
    float2(0.0000, 0.0000),
    float2(0.5000, 0.0000),
    float2(0.3536, 0.3536),
    float2(0.0000, 0.5000),
    float2(-0.3536, 0.3536),
    float2(-0.5000, 0.0000),
    float2(-0.3536, -0.3536),
    float2(-0.0000, -0.5000),
    float2(0.3536, -0.3536),
    float2(1.0000, 0.0000),
    float2(0.8660, 0.5000),
    float2(0.5000, 0.8660),
    float2(0.0000, 1.0000),
    float2(-0.5000, 0.8660),
    float2(-0.8660, 0.5000),
    float2(-1.0000, 0.0000),
    float2(-0.8660, -0.5000),
    float2(-0.5000, -0.8660),
    float2(-0.0000, -1.0000),
    float2(0.5000, -0.8660),
    float2(0.8660, -0.5000),
    float2(1.5000, 0.0000),
    float2(1.1491, 0.9642),
    float2(0.2605, 1.4772),
    float2(-0.7500, 1.2990),
    float2(-1.4095, 0.5130),
    float2(-1.4095, -0.5130),
    float2(-0.7500, -1.2990),
    float2(0.2605, -1.4772),
    float2(1.1491, -0.9642)
};


static const float3 pointLightPositionBlue[58] = 
{
    float3(49694.875000, -74093.718750, 17767.000000 + 100.0f),
    float3(48154.949219, -74961.500000, 17767.000000 + 100.0f),
    float3(46531.449219, -73880.171875, 17767.000000 + 100.0f),
    float3(47382.554688, -71861.476562, 17767.000000 + 100.0f),
    float3(46554.773438, -72180.742188, 17767.000000 + 100.0f),
    float3(44920.710938, -71454.257812, 17767.000000 + 100.0f),
    float3(44127.281250, -70073.843750, 17767.000000 + 100.0f),
    float3(42851.984375, -69853.554688, 17767.000000 + 100.0f),
    float3(40844.171875, -68294.789062, 17767.000000 + 100.0f),
    float3(40196.929688, -70088.859375, 17767.000000 + 100.0f),
    float3(40978.429688, -72532.976562, 17766.000000 + 100.0f),
    float3(42471.230469, -74995.187500, 17563.000000 + 100.0f),
    float3(41811.683594, -80739.484375, 17563.000000 + 100.0f),
    float3(39606.382812, -73663.898438, 17765.000000 + 100.0f),
    float3(37084.011719, -72738.359375, 17766.000000 + 100.0f),
    float3(33518.511719, -77410.937500, 18035.490234 + 100.0f),
    float3(33108.031250, -79953.523438, 18220.000000 + 100.0f),
    float3(33011.386719, -82318.367188, 18220.000000 + 100.0f),
    float3(32525.859375, -84239.484375, 18220.000000 + 100.0f),
    float3(30184.703125, -84209.765625, 18220.000000 + 100.0f),
    float3(28290.154297, -86654.203125, 18220.000000 + 100.0f),
    float3(30215.263672, -80937.117188, 18220.000000 + 100.0f),
    float3(29863.039062, -70893.523438, 17849.384766 + 100.0f),
    float3(37406.804688, -69565.726562, 17767.000000 + 100.0f),
    float3(40433.726562, -67348.250000, 17770.449219 + 100.0f),
    float3(41266.632812, -65165.851562, 17787.367188 + 100.0f),
    float3(42684.578125, -64394.738281, 17767.000000 + 100.0f),
    float3(42753.921875, -63414.730469, 17767.000000 + 100.0f),
    float3(44777.164062, -64319.281250, 17767.000000 + 100.0f),
    float3(48742.773438, -64264.109375, 17767.000000 + 100.0f),
    float3(54630.683594, -63577.847656, 17767.000000 + 100.0f),
    float3(43588.859375, -59180.578125, 17767.000000 + 100.0f),
    float3(44305.410156, -54429.167969, 17767.000000 + 100.0f),
    float3(39593.914062, -56788.003906, 17767.000000 + 100.0f),
    float3(39637.132812, -58938.804688, 17767.000000 + 100.0f),
    float3(37762.468750, -60052.136719, 17768.687500 + 100.0f),
    float3(36664.589844, -60245.949219, 17771.375000 + 100.0f),
    float3(35631.972656, -62456.882812, 17846.070312 + 100.0f),
    float3(35279.132812, -63282.304688, 17867.349609 + 100.0f),
    float3(35073.113281, -61242.238281, 17814.990234 + 100.0f),
    float3(34392.796875, -59952.015625, 17800.962891 + 100.0f),
    float3(33286.894531, -59347.484375, 17812.351562 + 100.0f),
    float3(30645.830078, -57556.316406, 17767.000000 + 100.0f),
    float3(31425.935547, -54970.007812, 17767.000000 + 100.0f),
    float3(35213.296875, -54016.539062, 17563.000000 + 100.0f),
    float3(36221.929688, -53983.914062, 17563.000000 + 100.0f),
    float3(35169.695312, -48232.847656, 17563.000000 + 100.0f),
    float3(35305.648438, -45535.343750, 17663.169922 + 100.0f),
    float3(41014.500000, -47202.445312, 17050.884766 + 100.0f),
    float3(42963.980469, -48102.480469, 16915.800781 + 100.0f),
    float3(44132.785156, -48097.597656, 16902.562500 + 100.0f),
    float3(31108.089844, -26463.583984, 19298.000000 + 100.0f),
    float3(29023.417969, -27068.917969, 19297.814453 + 100.0f),
    float3(25769.238281, -60721.085938, 18006.000000 + 100.0f),
    float3(35751.519531, -57299.601562, 17734.695312 + 100.0f),
    float3(36616.343750, -57047.312500, 17724.757812 + 100.0f),
    float3(29450.041016, -63667.343750, 18006.000000),
    float3(29453.349609, -64261.742188, 18006.000000)
};

static const float3 portalBlue[3] =
{
    float3(12461.169922, -111888.468750, 20881.730469 + 25.0f),
    float3(39576.746094, -114656.703125, 19964.044922 + 25.0f),
    float3(13605.537109, -12478.478516, 20503.302734 + 25.0f)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

// Utility function to compute flickering effect
float ComputeFlicker(float Time)
{
    float flicker = sin(Time * 10.0f) * 0.5f + 0.5f;
    flicker += sin(Time * 12.34f) * 0.25f;
    flicker = saturate(flicker);
    return 1.0f + (flicker - 1.0f) / 8.0f;
}

// Utility function to compute point light intensity
float ComputePointLightIntensity(float3 position, float3 lightPosition, float radius)
{
    float3 toLightPosition = position - lightPosition;
    float distanceToLight = length(toLightPosition);
    return pow(saturate(1.0f - distanceToLight / radius), 2);
}



float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // Fetch and transform normal from the diffuse map (pretending it's a normal map)
    float3 NM = normalize((tex2D(DiffuseTexture, input.UV).rgb * 2.0f) - 1.0f);
    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float OriginalLightIntensity = saturate(dot(N, L));
    float LightIntensity = 0.3f + 0.2f * OriginalLightIntensity;
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity);

#ifdef USE_SHADOWS

    float totalPointLightIntensity = 0.0f;
    if (SpecularLight > 0.5f)
    {
        float flicker = sin(Time / 2.0f) * 0.5f + 0.5f;
        flicker += sin(Time * 12.34f) * 0.25f;
        flicker = saturate(flicker);
        flicker = 1.0f + (flicker - 1.0f) / 35.0f; // This line will reduce the intensity of the flickering
    
        if(enableEffects < 0.5)
        {     
            float3 colorFactor = float3(1.0f, 0.6f, 0.3f);
            int lightCount = 58;
        
            for (int i = 0; i < lightCount; ++i)
            {
                float pointLightIntensity1 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionBlue[i], 4000.0f) * flicker;
                float pointLightIntensity2 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionBlue[i], 2000.0f) * flicker;
                totalPointLightIntensity += pointLightIntensity1;
                Color.rgb += pointLightIntensity2 * colorFactor * 6.0f / reduceLightIntensity;
            }

            float3 colorFactor2 = float3(1.0f, 0.0f, 0.0f);
            int lightCount2 = 3;
        
            for (int j = 0; j < lightCount2; ++j)
            {
                float pointLightIntensity1 = ComputePointLightIntensity(input.WorldPos.xyz, portalBlue[j], 8000.0f) * flicker;
                float pointLightIntensity2 = ComputePointLightIntensity(input.WorldPos.xyz, portalBlue[j], 4000.0f) * flicker;
                totalPointLightIntensity += pointLightIntensity1;
                Color.rgb += pointLightIntensity2 * colorFactor2 * 4.0f / reduceLightIntensity;
            }
            
        
            totalPointLightIntensity = saturate(totalPointLightIntensity); // clamp between [0, 1]
        }
    }

    const float ShadowBias = 0.0001f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;	 
    const float texelScale1 = 1.0f / 2500.0f;
    const float texelScale2 = 1.0f / 1200.0f;
	 const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i) {		 		
            float2 offset = pcfOffsets[i];				
				bigShadow   += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
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

    float finalShadow = lerp(bigShadow, localShadow, fade);//(shadow2 + (shadow1 * fade)) / 2.0f;
	 //finalShadow = bigShadow + gridVal * 0.8f;
	 float InvertedOpacity = 1.0f - (ShadowOpacity / 120.0f);

	finalShadow = lerp(bigShadow, localShadow, fade); 
	finalShadow *= InvertedOpacity;  // Here, if ShadowOpacity is 100 (full), InvertedOpacity will be 0, hence transparent.


	if(reduceLightIntensity < 1.1f)
		finalShadow *= (1.0f - totalPointLightIntensity);
	

	LightResult *= (1.0f - finalShadow);

	if (SpecularLight > 0.5f)
	{	
		if (finalShadow < 0.30)
		{
			float3 V = normalize(CameraPos - input.WorldPos.xyz);
			float3 R = reflect(-L, N);
			float SpecularIntensity = pow(saturate(dot(V, R)), Shininess);
			LightResult += (SpecularIntensity * MaterialSpecular * LightSpecular.rgb)/1.5f;
		}
	}
	

#endif

    Color.rgb *= LightResult;

    // Fog
    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(Color, FogColor, fogFactor);
}

)";
