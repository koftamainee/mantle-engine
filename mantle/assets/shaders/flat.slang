struct PushConstants {
    float4x4 mvp;
}

[[vk::push_constant]]
PushConstants pc;

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

[shader("vertex")]
VSOutput vert_main(VSInput input) {
    VSOutput output;
    output.position = mul(pc.mvp, float4(input.position, 1.0));
    output.normal = input.normal;
    return output;
}

[shader("fragment")]
float4 frag_main(VSOutput input) : SV_Target {
    float3 color = input.normal * 0.5 + 0.5;
    return float4(color, 1.0);
}