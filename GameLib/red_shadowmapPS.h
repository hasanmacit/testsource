const char* red_shadowmapPS = R"(
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
float reduceLightIntensity : register(c15);
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

static const float3 pointLightPositionRed[45] = 
{
    float3(59904.207031, -69007.156250, 19851.500000 + 100.0f),
    float3(59086.726562, -68210.609375, 19851.500000 + 100.0f),
    float3(59913.460938, -66832.906250, 19851.500000 + 100.0f),
    float3(58036.195312, -65320.097656, 19850.699219 + 100.0f),
    float3(56760.632812, -63942.218750, 19848.500000 + 100.0f),
    float3(55364.449219, -63593.480469, 19848.000000 + 100.0f),
    float3(55965.179688, -61395.375000, 19847.500000 + 100.0f),
    float3(50225.968750, -57133.539062, 18146.820312 + 100.0f),
    float3(49441.070312, -58756.074219, 18136.500000 + 100.0f),
    float3(48771.917969, -58040.152344, 18137.384766 + 100.0f),
    float3(49258.175781, -55886.367188, 18130.392578 + 100.0f),
    float3(58409.285156, -55469.964844, 19821.265625 + 100.0f),
    float3(67788.867188, -52645.410156, 19947.537109 + 100.0f),
    float3(65710.960938, -54625.472656, 19862.500000 + 100.0f),
    float3(68453.343750, -59912.242188, 19872.185547 + 100.0f),
    float3(69324.757812, -60106.371094, 19906.902344 + 100.0f),
    float3(70943.640625, -58654.792969, 20007.744141 + 100.0f),
    float3(66324.609375, -62656.597656, 19862.500000 + 100.0f),
    float3(60212.226562, -62767.753906, 19862.500000 + 100.0f),
    float3(61111.105469, -63208.078125, 19862.500000 + 100.0f),
    float3(60844.238281, -60906.105469, 19862.500000 + 100.0f),
    float3(63420.210938, -63875.281250, 19862.500000 + 100.0f),
    float3(67611.000000, -61272.789062, 19864.609375 + 100.0f),
    float3(68459.140625, -57390.687500, 19871.822266 + 100.0f),
    float3(67568.546875, -55491.835938, 19875.765625 + 100.0f),
    float3(73299.929688, -65628.625000, 20398.597656 + 100.0f),
    float3(70338.617188, -45447.914062, 20354.500000 + 100.0f),
    float3(71813.234375, -39815.250000, 20356.509766 + 100.0f),
    float3(67861.523438, -37753.929688, 20485.320312 + 100.0f),
    float3(65518.492188, -48587.628906, 19863.900391 + 100.0f),
    float3(63935.410156, -46340.988281, 19862.500000 + 100.0f),
    float3(61878.402344, -48192.171875, 19862.500000 + 100.0f),
    float3(60498.425781, -50068.250000, 19868.000000 + 100.0f),
    float3(64904.945312, -52523.628906, 19900.394531 + 100.0f),
    float3(60638.898438, -54671.625000, 19860.662109 + 100.0f),
    float3(50425.316406, -50867.527344, 17592.460938 + 100.0f),
    float3(60175.128906, -28379.259766, 19354.869141 + 100.0f),
    float3(59907.171875, -35075.804688, 19402.500000 + 100.0f),
    float3(59032.718750, -39366.027344, 19407.910156 + 100.0f),
    float3(56118.066406, -33633.675781, 19398.216797 + 100.0f),
    float3(55953.476562, -32612.285156, 19399.355469 + 100.0f),
    float3(56676.796875, -30748.783203, 19397.410156 + 100.0f),
    float3(36629.328125, -116224.414062, 12762.500000 + 100.0f),
    float3(62907.722656, -55114.359375, 19862.500000 + 100.0f),
    float3(29453.349609, -64261.742188, 18006.000000 + 100.0f)
};

static const float3 portalRed[2] =
{
    float3(93952.523438, -104458.125000, 18911.000000 + 25.0f),
    float3(40605.292969, -7091.672852, 18205.830078 + 25.0f)
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
        
        if (enableEffects < 0.5)
        {
        
            for (int i = 0; i < 45; ++i)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionRed[i]);

                if (distanceToSpecular2 < 2000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 2000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionRed[i] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 1.6f / reduceLightIntensity; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(0.9f, 0.7f, 0.3f);

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
		
            for (int j = 0; j < 2; ++j)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - portalRed[j]);

                if (distanceToSpecular2 < 8000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 8000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(portalRed[j] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 0.9f; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker * 3; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(1.0f, 0.0f, 0.0f);

        // If there's no alpha channel, reduce the intensity
                    if (Color.a == 0.0f)
                    {
                        blueSpecIntensity2 *= 0.9f;
                    }
                    else // If there's an alpha channel, add the second specular with very high shininess
                    {
                        float highShininess = 20.0f; // Very high shininess for tighter highlight
                        float blueSpecIntensityHigh = blueSpecMultiplier2 * 5.0f * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), highShininess) * flicker * 3;
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
