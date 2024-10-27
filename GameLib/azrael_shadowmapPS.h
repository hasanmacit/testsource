const char* azrael_shadowmapPS = R"(
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
float Time : register(c12); // Add this to the constants at the bottom
float enableEffects : register(c13);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1); // big shadowmap
sampler2D ShadowMap2 : register(s2); // local (small) shadowmap

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

static const float3 pointLightPositionGalbeni[17] = 
{
    float3(6563.013184, -6570.580566, 21920.000000 + 100.0f),
    float3(11537.906250, -7753.419922, 21919.314453 + 100.0f),
    float3(15886.503906, -4803.235352, 21827.792969 + 100.0f),
    float3(33381.636719, -27281.521484, 21877.000000 + 100.0f),
    float3(31313.623047, -28896.279297, 21843.794922 + 100.0f),
    float3(42318.492188, -38209.644531, 20978.500000 + 100.0f),
    float3(41996.722656, -39745.921875, 20978.500000 + 100.0f),
    float3(40185.468750, -41099.273438, 20978.500000 + 100.0f),
    float3(35397.671875, -38862.660156, 20978.500000 + 100.0f),
    float3(41094.769531, -35588.355469, 20978.500000 + 100.0f),
    float3(44911.015625, -25544.224609, 21877.000000 + 100.0f),
    float3(40785.019531, -20491.716797, 21870.212891 + 100.0f),
    float3(26991.152344, -12226.507812, 21893.000000 + 100.0f),
    float3(8100.000000, -19000.000000, 22104.000000 + 100.0f),
    float3(23658.312500, -5831.427246, 21875.539062 + 100.0f),
    float3(21717.156250, -9502.573242, 21867.167969 + 100.0f),
    float3(31011.748047, -22446.476562, 21878.480469 + 100.0f)
};

static const float3 portalGalbeni[1] =
{
    float3(6667.594727, -5030.292480, 21918.919922 + 25.0f)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
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
    const float ShadowBias = 0.000009f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;
    const float texelScale1 = 1.0f / 2500.0f;
    const float texelScale2 = 1.0f / 6000.0f;
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
		float3 V = normalize(CameraPos - input.WorldPos.xyz);
		float3 R = reflect(-L, N);

        if (finalShadow < 0.25f)
        {
            // Compute specular reflection based on alpha intensity
            // Add Phong Specular Reflection only if alpha is greater than 0

            float alphaFactor = (Color.a != 1.0f) ? 3.0f : 0.45f;
            float shinessAlpha = (Color.a != 1.0f) ? Shininess : 7.00f;

            float SpecularIntensity = alphaFactor * Color.a * pow(saturate(dot(V, R)), shinessAlpha);

            LightResult += SpecularIntensity * MaterialSpecular * LightSpecular.rgb * alphaFactor;
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
				SpecularIntensity = alphaFactor * Color.a * pow(saturate(dotVR), shinessAlpha * 2) / 2;
			}
			else
			{
				// Light is coming from the opposite side, reduce specular intensity
				// You can adjust the factor to reduce the intensity as desired
				SpecularIntensity = alphaFactor * Color.a * pow(saturate(abs(dotVR)), shinessAlpha / 2) / 10;
			}
			LightResult += SpecularIntensity * MaterialSpecular * LightSpecular.rgb * alphaFactor;
		}
		
        float flicker = sin(Time / 2.0f) * 0.5f + 0.5f;
        flicker += sin(Time * 12.34f) * 0.25f;
        flicker = saturate(flicker);
        flicker = 1.0f + (flicker - 1.0f) / 25.0f; // This line will reduce the intensity of the flickering
		
		float flicker2 = sin(Time / 150.0f) * 0.25f + 0.75f;  // Slower base flicker
		flicker2 += sin(Time / 10.0f) * 0.1f;                 // Slower and smaller magnitude secondary fluctuation
		flicker2 = saturate(flicker2);                        // Ensure it's clamped between 0 and 1
		flicker2 *= 0.85f;                                    // Reduce overall intensity to make it darker
        
        if (enableEffects < 0.5)
        {
        
            for (int i = 0; i < 17; ++i)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionGalbeni[i]);

                if (distanceToSpecular2 < 3000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 3000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionGalbeni[i] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 1.6f; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(1.0f, 0.7f, 0.3f);

        // If there's no alpha channel, reduce the intensity
                    if (Color.a == 0.0f)
                    {
                        blueSpecIntensity2 *= 0.9f;
                    }
                    else // If there's an alpha channel, add the second specular with very high shininess
                    {
                        float highShininess = 20.0f; // Very high shininess for tighter highlight
                        float blueSpecIntensityHigh = blueSpecMultiplier2 * 5.0f * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), highShininess) * flicker;
                        LightResult += blueSpecIntensityHigh * blueSpecColor2;
                    }

                    LightResult += blueSpecIntensity2 * blueSpecColor2;
                }
            }
        
            for (int j = 0; j < 1; ++j)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - portalGalbeni[j]);

                if (distanceToSpecular2 < 2000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 2000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(portalGalbeni[j] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 0.9f; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker2; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(1.0f, 0.7f, 0.3f);

        // If there's no alpha channel, reduce the intensity
                    if (Color.a == 0.0f)
                    {
                        blueSpecIntensity2 *= 0.9f;
                    }
                    else // If there's an alpha channel, add the second specular with very high shininess
                    {
                        float highShininess = 20.0f; // Very high shininess for tighter highlight
                        float blueSpecIntensityHigh = blueSpecMultiplier2 * 5.0f * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), highShininess) * flicker2;
                        LightResult += blueSpecIntensityHigh * blueSpecColor2;
                    }

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
