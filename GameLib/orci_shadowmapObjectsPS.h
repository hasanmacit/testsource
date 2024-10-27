const char* orci_shadowmapObjectsPS = R"(
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
#define SHADOW_PCF_RADIUS 2
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
float Time : register(c12);
float reduceLightIntensity : register(c15);
float enableEffects : register(c13);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s2); // big shadowmap
sampler2D ShadowMap2 : register(s3); // local (small) shadowmap

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

static const float3 pointLightPositionRed[7] = 
{
    float3(11774.305664, -9889.907227, 17633.500000 + 100.0f),
    float3(13037.123047, -8809.815430, 17633.500000 + 100.0f),
    float3(12618.804688, -2669.528076, 17655.275391 + 100.0f),
    float3(78418.671875, -73614.078125, 16125.000000 + 100.0f),
    float3(78100.820312, -77778.437500, 16125.000000 + 100.0f),
    float3(77483.437500, -79870.406250, 16125.000000 + 100.0f),
    float3(73007.750000, -76392.132812, 16125.000000 + 100.0f)
};

static const float3 portalRed[5] =
{
    float3(7794.366211, -1554.875610, 18151.900391 + 50.0f),
    float3(13111.487305, -74616.609375, 16875.437500 + 50.0f),
    float3(11598.202148, -149237.953125, 17184.648438 + 50.0f),
    float3(63969.058594, -143611.812500, 16412.500000 + 50.0f),
    float3(147262.312500, -7419.125000, 19268.500000 + 50.0f)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float ComputePointLightIntensity(float3 position, float3 lightPosition, float radius)
{
    float3 toLightPosition = position - lightPosition;
    float distanceToLight = length(toLightPosition);
    return pow(saturate(1.0f - distanceToLight / radius), 2);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // Fetch and transform normal from normal map
    float3 NM = normalize((tex2D(DiffuseTexture, input.UV).rgb * 10.0f) - 1.0f);
    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.65f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.0002f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;
    const float texelScale1 = 1.0f / 1500.0f;
    const float texelScale2 = 1.0f / 2500.0f;
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

	if (SpecularLight > 0.5f) 
	{
		// Compute view direction
		float3 V = normalize(CameraPos - input.WorldPos.xyz);
		
		// Compute reflection vector
		float3 R = reflect(-L, N);
	
		// If the surface is not in shadow...
		if (finalShadow < 0.25)
		{
			// Compute standard specular reflection based on alpha intensity
			float SpecularIntensity = pow(saturate(dot(V, R)), Shininess);
	
			if (Color.a < 0.5f) // Increase specular light power by 10 only where alpha channel is present
			{
				SpecularIntensity /= 4.0f;
			}
			
			LightResult += SpecularIntensity * MaterialSpecular * LightSpecular.rgb;
		}
		else
		{
			// Compute specular reflection based on alpha intensity
			// Amplify Phong Specular Reflection when light is coming from the correct side
		
			float alphaFactor = (Color.a != 1.0f) ? 3.0f : 0.45f;
			float shinessAlpha = (Color.a != 1.0f) ? Shininess : 7.00f;
		
			float3 V = normalize(CameraPos - input.WorldPos.xyz);
			float3 R = reflect(-L, N);
			// Check the dot product, amplify if the light is coming from the correct side
			float dotVR = dot(V, R);
			float SpecularIntensity;
			
			if (dotVR > 0)
			{
				// Light is coming from the correct side, calculate traditional specular intensity
				SpecularIntensity = alphaFactor * Color.a * pow(saturate(dotVR), shinessAlpha * 10) / 10;
			}
			else
			{
				// Light is coming from the opposite side, reduce specular intensity
				// You can adjust the factor to reduce the intensity as desired
				SpecularIntensity = alphaFactor * Color.a * pow(saturate(abs(dotVR)), shinessAlpha * 12) / 26;
			}
		
			LightResult += SpecularIntensity * MaterialSpecular * LightSpecular.rgb * alphaFactor;
		}		
        
        float flicker = sin(Time / 2.0f) * 0.5f + 0.5f;
        flicker += sin(Time * 12.34f) * 0.25f;
        flicker = saturate(flicker);
        flicker = 1.0f + (flicker - 1.0f) / 25.0f; // This line will reduce the intensity of the flickering
		
        if (enableEffects < 0.5)
        {
        
            for (int i = 0; i < 7; ++i)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionRed[i]);

                if (distanceToSpecular2 < 2000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 2000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionRed[i] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 3.6f; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(0.9f, 0.7f, 0.3f);

        // If there's no alpha channel, reduce the intensity

                    float highShininess = 80.0f; // Very high shininess for tighter highlight
                    float blueSpecIntensityHigh = blueSpecMultiplier2 * 1.5f * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), highShininess) * flicker;
                    LightResult += blueSpecIntensityHigh * blueSpecColor2;
                

                    LightResult += blueSpecIntensity2 * blueSpecColor2;
                }
            }
		
            for (int j = 0; j < 5; ++j)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - portalRed[j]);

                if (distanceToSpecular2 < 8000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 8000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(portalRed[j] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 5.0f / reduceLightIntensity; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 1.4f) * flicker; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(0.35f, 0.169f, 0.839f);
                    LightResult += blueSpecIntensity2 * blueSpecColor2;
                }
            }
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
