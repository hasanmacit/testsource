const char* yellow_shadowmapObjectsPS = R"(
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

static const float3 pointLightPositionGalbeni[67] = 
{
    float3(54118.625000, -56074.523438, 20025.000000 + 100.0f),
    float3(64849.179688, -67658.078125, 19182.117188 + 100.0f),
    float3(60261.425781, -64519.871094, 19210.742188 + 100.0f),
    float3(57890.449219, -68241.320313, 19433.500000 + 100.0f),
    float3(54893.867188, -61220.050781, 19307.500000 + 100.0f),
    float3(59028.601563, -62238.882813, 19308.294922 + 100.0f),
    float3(73665.851563, -66404.210938, 19405.009766 + 100.0f),
    float3(73322.429688, -69002.562500, 19408.505859 + 100.0f),
    float3(72076.617188, -66306.343750, 19408.150391 + 100.0f),
    float3(52414.234375, -68321.062500, 19441.500000 + 100.0f),
    float3(60971.636719, -62580.386719, 19217.095703 + 100.0f),
    float3(63184.070313, -59617.746094, 19175.000000 + 100.0f),
    float3(57148.054688, -55780.378906, 20025.000000 + 100.0f),
    float3(51546.964844, -51912.945313, 20934.500000 + 100.0f),
    float3(60207.500000, -59887.601563, 19186.500000 + 100.0f),
    float3(66089.867188, -68198.171875, 19175.000000 + 100.0f),
    float3(67498.171875, -66832.195313, 19175.000000 + 100.0f),
    float3(64651.031250, -59702.773438, 19175.255859 + 100.0f),
    float3(66773.843750, -72356.671875, 19175.000000 + 100.0f),
    float3(64724.867188, -56062.234375, 19175.000000 + 100.0f),
    float3(56413.406250, -51740.265625, 20214.000000 + 100.0f),
    float3(55973.984375, -51496.871094, 20214.000000 + 100.0f),
    float3(56523.195313, -52119.640625, 20214.000000 + 100.0f),
    float3(63349.167969, -70990.390625, 19182.351563 + 100.0f),
    float3(67132.687500, -60984.257813, 19266.320313 + 100.0f),
    float3(64868.835938, -71040.890625, 19175.000000 + 100.0f),
    float3(54242.027344, -55315.039063, 20025.000000 + 100.0f),
    float3(67800.140625, -62219.574219, 19267.500000 + 100.0f),
    float3(63338.152344, -56039.433594, 19175.000000 + 100.0f),
    float3(56522.957031, -52642.890625, 20203.230469 + 100.0f),
    float3(57156.730469, -62259.710938, 19309.390625 + 100.0f),
    float3(59652.640625, -70136.515625, 19433.500000 + 100.0f),
    float3(49579.847656, -65056.410156, 19136.000000 + 100.0f),
    float3(49688.453125, -63596.886719, 19136.230469 + 100.0f),
    float3(46988.519531, -65014.031250, 19128.824219 + 100.0f),
    float3(54957.417969, -63346.265625, 19278.000000 + 100.0f),
    float3(77214.703125, -64516.074219, 19136.050781 + 100.0f),
    float3(77287.070312, -63397.015625, 19135.074219 + 100.0f),
    float3(79355.460938, -63015.773438, 18921.929688 + 100.0f),
    float3(64918.222656, -81376.671875, 18042.169922 + 100.0f),
    float3(63304.476562, -81184.195312, 17965.000000 + 100.0f),
    float3(63312.214844, -78676.382812, 18628.242188 + 100.0f),
    float3(60465.246094, -56853.414062, 19186.500000 + 100.0f),
    float3(57475.996094, -59655.082031, 19263.250000 + 100.0f),
    float3(53090.882812, -59143.207031, 19310.880859 + 100.0f),
    float3(52466.230469, -74911.914062, 19758.322266 + 100.0f),
    float3(53879.535156, -78725.796875, 19672.500000 + 100.0f),
    float3(69333.953125, -58399.835938, 19267.000000 + 100.0f),
    float3(87290.468750, -57553.914062, 19475.500000 + 100.0f),
    float3(87731.796875, -82343.929688, 18940.500000 + 100.0f),
    float3(62190.203125, -89896.429688, 17955.453125 + 100.0f),
    float3(65120.871094, -92285.257812, 18038.283203 + 100.0f),
    float3(57658.757812, -89424.257812, 18156.369141 + 100.0f),
    float3(52520.347656, -89718.382812, 18455.365234 + 100.0f),
    float3(48617.523438, -93132.773438, 18454.500000 + 100.0f),
    float3(28594.697266, -41064.414062, 17133.500000 + 100.0f),
    float3(89363.390625, -104575.570312, 18462.039062 + 100.0f),
    float3(38830.957031, -83958.515625, 17666.408203 + 100.0f),
    float3(30684.011719, -81488.265625, 17246.000000 + 100.0f),
    float3(35128.378906, -76889.609375, 19008.294922 + 100.0f),
    float3(46534.378906, -69452.835938, 18936.884766 + 100.0f),
    float3(55259.457031, -67726.882812, 19439.779297 + 100.0f),
    float3(82275.265625, -55659.539062, 18689.509766 + 100.0f),
    float3(87296.484375, -53630.347656, 19475.500000 + 100.0f),
    float3(48431.386719, -37712.652344, 18862.000000 + 100.0f),
    float3(20415.257812, -54574.515625, 16412.822266 + 100.0f),
    float3(70306.640625, -69044.640625, 19410.765625 + 100.0f)
};

static const float3 portalGalbeni[3] =
{
    float3(87608.976562, -112915.703125, 18334.544922 + 25.0f),
    float3(61424.457031, -11605.809570, 19905.804688 + 25.0f),
    float3(90872.867188, -8548.823242, 20097.580078 + 25.0f)
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
		
		float flicker2 = sin(Time / 250.0f) * 0.25f + 0.75f;  // Slower base flicker
		flicker2 += sin(Time / 50.0f) * 0.1f;                 // Slower and smaller magnitude secondary fluctuation
		flicker2 = saturate(flicker2);                        // Ensure it's clamped between 0 and 1
		flicker2 *= 0.85f;                                    // Reduce overall intensity to make it darker
        
        if (enableEffects < 0.5)
        {
        
            for (int i = 0; i < 67; ++i)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - pointLightPositionGalbeni[i]);

                if (distanceToSpecular2 < 2000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 2000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(pointLightPositionGalbeni[i] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 3.6f / reduceLightIntensity; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 0.8f) * flicker; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(1.0f, 0.6f, 0.3f);

        // If there's no alpha channel, reduce the intensity

                    float highShininess = 80.0f; // Very high shininess for tighter highlight
                    float blueSpecIntensityHigh = blueSpecMultiplier2 * 1.8f * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), highShininess) * flicker;
                    LightResult += blueSpecIntensityHigh * blueSpecColor2;
                

                    LightResult += blueSpecIntensity2 * blueSpecColor2;
                }
            }
        
            for (int j = 0; j < 3; ++j)
            {
                float distanceToSpecular2 = length(input.WorldPos.xyz - portalGalbeni[j]);

                if (distanceToSpecular2 < 8000.0f)
                {
        // Quadratic falloff for intensity (intense near the center, zero on the margins of 2000.0)
                    float falloff2 = (1.0f - distanceToSpecular2 / 8000.0f);
                    falloff2 = falloff2 * falloff2; // Square the falloff for a more pronounced effect

                    float3 toSpecular2 = normalize(portalGalbeni[j] - input.WorldPos.xyz);
                    float3 specularReflection2 = reflect(-V, N);
                    float blueSpecMultiplier2 = 1.0f; // Multiplier to scale the intensity
                    float blueSpecIntensity2 = blueSpecMultiplier2 * falloff2 * pow(saturate(dot(specularReflection2, toSpecular2)), 1.4f) * flicker2; // Adjusted power for tighter highlight
                    float3 blueSpecColor2 = float3(1.0f, 0.0f, 0.0f);
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
