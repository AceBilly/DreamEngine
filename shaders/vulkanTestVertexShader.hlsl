
static float2 pos[3] = { {0.0, -0.5}, {0.5, 0.5}, {-0.5, 0.5} };
static float3 col[3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} };

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


PixelShaderInput main()
{
	static int currentVertexIndex = 0;
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.color = float4(col[currentVertexIndex], 1.0);
	vertexShaderOutput.pos = float4(pos[currentVertexIndex], 0.0, 1.0);
    ++currentVertexIndex;
	if(currentVertexIndex > 2)
	{
		currentVertexIndex = 0;
	}
	return vertexShaderOutput;
}