const char* lobby_shadowmapPS = R"(
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

static const float3 pointLightPositionLobby[58] =
{
    float3(4599.603027, -17126.146484, 17760.044922 + 100.0f),
    float3(7430.978516, -20678.500000, 17751.500000 + 100.0f),
    float3(3438.737549, -21313.423828, 17748.615234 + 100.0f),
    float3(4381.654297, -24431.451172, 17750.894531 + 100.0f),
    float3(1942.597900, -26004.628906, 17750.775391 + 100.0f),
    float3(4712.081055, -27615.705078, 17738.699219 + 100.0f),
    float3(6651.171387, -25468.330078, 17747.373047 + 100.0f),
    float3(9415.385742, -21994.339844, 17737.849609 + 100.0f),
    float3(10916.774414, -24449.775391, 17750.435547 + 100.0f),
    float3(13162.217773, -26191.349609, 17751.097656 + 100.0f),
    float3(17775.595703, -27018.378906, 17748.669922 + 100.0f),
    float3(22596.466797, -31026.160156, 17833.960938 + 100.0f),
    float3(21213.281250, -27952.957031, 18256.546875 + 100.0f),
    float3(17370.451172, -21125.531250, 18257.925781 + 100.0f),
    float3(21766.947266, -13792.586914, 18262.000000 + 100.0f),
    float3(24398.091797, -12785.338867, 18262.000000 + 100.0f),
    float3(32067.482422, -11593.994141, 18258.000000 + 100.0f),
    float3(33560.625000, -15502.521484, 18258.810547 + 100.0f),
    float3(34679.726562, -18100.246094, 18258.589844 + 100.0f),
    float3(33014.984375, -24310.093750, 18261.484375 + 100.0f),
    float3(30726.216797, -27894.417969, 18255.337891 + 100.0f),
    float3(26108.285156, -29796.980469, 18321.591797 + 100.0f),
    float3(29283.658203, -31470.113281, 17922.437500 + 100.0f),
    float3(35984.324219, -34262.367188, 17751.500000 + 100.0f),
    float3(36708.996094, -25498.898438, 17751.500000 + 100.0f),
    float3(38397.316406, -25517.166016, 17755.818359 + 100.0f),
    float3(40258.160156, -25416.595703, 17751.500000 + 100.0f),
    float3(41618.023438, -24188.832031, 17751.500000 + 100.0f),
    float3(41653.285156, -22228.701172, 17751.167969 + 100.0f),
    float3(41706.812500, -19855.302734, 17757.214844 + 100.0f),
    float3(41532.597656, -17117.337891, 17751.500000 + 100.0f),
    float3(39378.097656, -14581.868164, 17756.574219 + 100.0f),
    float3(38344.785156, -13483.138672, 17750.210938 + 100.0f),
    float3(39533.429688, -12115.005859, 17751.574219 + 100.0f),
    float3(43448.054688, -11921.946289, 17750.873047 + 100.0f),
    float3(46706.105469, -12455.190430, 17751.779297 + 100.0f),
    float3(45311.273438, -23971.099609, 17748.525391 + 100.0f),
    float3(45095.050781, -29883.931641, 17750.187500 + 100.0f),
    float3(42651.496094, -39310.863281, 18227.500000 + 100.0f),
    float3(39919.343750, -34403.769531, 17887.500000 + 100.0f),
    float3(26710.029297, -33993.070312, 17770.089844 + 100.0f),
    float3(22767.683594, -42266.218750, 17809.960938 + 100.0f),
    float3(27761.738281, -39817.953125, 17806.984375 + 100.0f),
    float3(35066.031250, -41150.187500, 17761.234375 + 100.0f),
    float3(18927.271484, -43828.000000, 19642.230469 + 100.0f),
    float3(19086.314453, -38804.750000, 20104.814453 + 100.0f),
    float3(17116.355469, -39065.199219, 20090.144531 + 100.0f),
    float3(10594.347656, -35856.707031, 20982.677734 + 100.0f),
    float3(14384.308594, -47258.136719, 19679.519531 + 100.0f),
    float3(16300.496094, -47234.082031, 19683.164062 + 100.0f),
    float3(18820.548828, -47207.785156, 19682.089844 + 100.0f),
    float3(20602.144531, -47199.726562, 19685.162109 + 100.0f),
    float3(22618.007812, -47177.621094, 19683.605469 + 100.0f),
    float3(24900.587891, -47117.597656, 19689.132812 + 100.0f),
    float3(26540.480469, -47123.945312, 19686.779297 + 100.0f),
    float3(6007.584473, -22254.117188, 17747.890625 + 20.0f),
    float3(3227.504150, -29173.126953, 17749.382812 + 20.0f),
    float3(5240.759766, -29621.611328, 17747.964844 + 20.0f)
};

static const float3 pointLightPositionLobbyBoat[4] =
{
    float3(6000.000000, -13100.000000, 16725.500000 + 2000.0f),
    float3(5400.000000, -10200.000000, 16725.500000 + 2000.0f),
    float3(10300.000000, -8800.000000, 16725.500000 + 2000.0f),
    float3(13300.000000, -9200.000000, 16725.500000 + 2000.0f)
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
        
            for (int i = 0; i < 58; ++i)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionLobby[i]);

                if (distanceToSpecular2 < 3000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 3000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionLobby[i] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 1.6f; // Multiplier to scale the intensity
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
        
            for (int j = 0; j < 4; ++j)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionLobbyBoat[j]);

                if (distanceToSpecular2 < 5000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 5000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionLobbyBoat[j] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 1.6f; // Multiplier to scale the intensity
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
