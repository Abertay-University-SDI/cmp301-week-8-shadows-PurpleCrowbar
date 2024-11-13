
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix[2];
	matrix lightProjectionMatrix[2];
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos[2] : TEXCOORD1;
};


OutputType main(InputType input)
{
    OutputType output;

    // Transform vertex to world, view, and projection space
    float4 worldPos = mul(input.position, worldMatrix);
    output.position = mul(worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate light view positions for both lights
    for (int i = 0; i < 2; i++)
    {
        float4 lightViewPosition = mul(worldPos, lightViewMatrix[i]);
        output.lightViewPos[i] = mul(lightViewPosition, lightProjectionMatrix[i]);
    }

    // Pass through texture coordinates
    output.tex = input.tex;

    // Transform and normalie the normal vector
    float3 worldNormal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(worldNormal);

    return output;
}