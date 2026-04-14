static const float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);
static const float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

struct VSInput {
    uint vertex_id : SV_VertexID;
};
struct VSOutput {
    float3 color : COLOR;
    float4 position : SV_Position;
};

[shader("vertex")]
VSOutput vert_main(VSInput input) {
    VSOutput output;
    output.position = float4(positions[input.vertex_id], 0.0, 1.0);
    output.color = colors[input.vertex_id];
    return output;
}

[shader("fragment")]
float4 frag_main(VSOutput input) : SV_Target {
    return float4(input.color, 1.0);
}