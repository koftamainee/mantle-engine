import bindings;

struct MeshVertex {
    float3 position : POSITION;
    float3 color : TEXCOORD0;
};

struct VSOut {
    float4 pos : SV_Position;
    float3 color : TEXCOORD1;
};

[[vk::push_constant]]
cbuffer PushConstants {
    float4x4 view_proj;
    float4x4 model;
};

[shader("vertex")]
VSOut vert_main(MeshVertex vertex) {
    VSOut output;
    float4 world_pos = mul(model, float4(vertex.position, 1.0));
    output.pos = mul(view_proj, world_pos);
    output.color = vertex.color;
    return output;
}

[shader("fragment")]
float4 frag_main(VSOut input) : SV_Target {
    return float4(input.color, 1.0);
}
