import bindings;

[[vk::binding(STORAGE_IMAGE_BINDING, 0)]]
RWTexture2D<float4> g_storage_images[];

[[vk::binding(STORAGE_BUFFER_BINDING, 0)]]
RWStructuredBuffer<uint> g_storage_buffers[];

[[vk::push_constant]]
cbuffer PushConstants {
    uint storage_image_index;
    uint chunk_buffer_index;
    float3 camera_position;
    float3 camera_forward;
    float3 camera_right;
    float3 camera_up;
    float  camera_fov;
    float  camera_aspect;
};

static const uint MAX_DDA_STEPS = 256;
static const uint CHUNK_SIZE = 32;

struct Ray {
    float3 origin;
    float3 direction;
};

struct StepResult {
    int3 normal;
    int  axis;
};

struct HitResult {
    bool hit;
    int3 voxel;
    int3 normal;
    float t;
    int voxel_type;
};

struct DDAState {
    int3 voxel;
    int3 step;
    float3 t_max;
    float3 t_delta;
};

Ray ray(float2 ndc);
DDAState dda_init(Ray ray);
StepResult dda_step(inout DDAState state);
uint voxel_index(int3 voxel);
int voxel_type(int3 voxel);
HitResult dda_traverse(Ray ray, int max_steps);

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
    uint width, height;
    g_storage_images[storage_image_index].GetDimensions(width, height);

    if (dispatch_id.x >= width || dispatch_id.y >= height) {
        return;
    }

    float scale = tan(camera_fov * 0.5);

    float2 uv = float2(dispatch_id.xy) / float2(width, height);
    float2 ndc = uv * 2.0 - 1.0;

    ndc.x *= camera_aspect * scale;
    ndc.y *= scale;
    ndc.y *= -1;

    Ray r = ray(ndc);
    HitResult hit = dda_traverse(r, MAX_DDA_STEPS);

    float4 color;
    if (hit.hit) {
        color = float4(float3(hit.normal) * 0.5 + 0.5, 1.0);
    } else {
        color = float4(0.0, 0.0, 0.0, 0.0);
    }

    g_storage_images[storage_image_index][dispatch_id.xy] = color;
}

Ray ray(float2 ndc) {
    Ray ray;
    ray.direction = normalize(
        camera_forward +
        ndc.x * camera_right +
        ndc.y * camera_up);

    ray.origin = camera_position;

    return ray;
}

DDAState dda_init(Ray ray) {
    DDAState state;

    state.voxel = int3(floor(ray.origin));
    state.step = int3(sign(ray.direction));
    state.t_delta = abs(1.0 / ray.direction);
    state.t_max = (float3(state.voxel + max(state.step, 0)) - ray.origin)
        / ray.direction;

    return state;
}

StepResult dda_step(inout DDAState state) {
    int axis;
    if (state.t_max.x < state.t_max.y && state.t_max.x < state.t_max.z) {
        axis = 0; // x
    } else if (state.t_max.y < state.t_max.z) {
        axis = 1; // y
    } else {
        axis = 2; // z
    }

    state.voxel[axis] += state.step[axis];
    state.t_max[axis] += state.t_delta[axis];

    int3 normal = int3(0);
    normal[axis] = -state.step[axis];
    return {normal, axis};
}

uint voxel_index(int3 voxel) {
    return voxel.z * CHUNK_SIZE * CHUNK_SIZE + voxel.y * CHUNK_SIZE + voxel.x;
}

int voxel_type(int3 voxel) {
    if (any(voxel < 0) || any(voxel >= int(CHUNK_SIZE)))
        return 0;
    return int(g_storage_buffers[chunk_buffer_index][voxel_index(voxel)]);
}

HitResult dda_traverse(Ray ray, int max_steps) {
    DDAState state = dda_init(ray);
    for (int i = 0; i < max_steps; i++) {
        StepResult step = dda_step(state);
        int type = voxel_type(state.voxel);
        if (type != 0) {
            float t = state.t_max[step.axis] - state.t_delta[step.axis];
            return { true, state.voxel, step.normal, t, type };
        }
    }
    return { false, int3(0), int3(0), 0.0, 0 };
}