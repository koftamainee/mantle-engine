### M0. Cleanup

- [x] Split up renderer into frame scheduler and render pass, also Renderer::Impl should not have any methods
- [ ] Design and implement bindless descriptor system: single global set, partially bound arrays, resource handles via push constants (`VK_EXT_descriptor_indexing`)
- [ ] Initialize ImGui in vulkan backend, carry out into separate class with its own descriptor pool and set (separate from global bindless set)
- [ ] Remove MVP from push constants, pass camera matrices via descriptor sets using bindless system
- [x] Add explicit compute-to-graphics pipeline barrier infrastructure (storage image write -> read synchronization, separate from frame-in-flight sync)
- [ ] Implement pool/slab allocator and its pmr version
- [ ] Replace all STL containers with pmr versions using custom allocators, or avoid STL in hot paths
- [ ] Add memory profiling stats to existing allocators
- [ ] Remove mangohud fps overlay, add new debug overlay in ImGui (F3, F3 + 1, 2, 3, 4)

### M1. Voxel rendering

The basic idea: cpu meshing is too expensive for processing huge amount of small voxels, gpu meshing will probably be ok, but struggle with destructions, we will need to rebuild entire mesh on each player interaction. This is quite expensive.

#### New approach

Instead of rasterizing voxels we will raytrace them using 3D DDA algorithm, all of it in a compute shader.

- [ ] Create compute pipeline class in vulkan abstractions
- [ ] Create a DOD style Chunk struct, voxels should be array of u32/u16/u8, bit packed with material palettes, some flags, params like temperature, pressure, etc. -- for later, dont mind them yet
- [ ] Decide on G-buffer layout: what data DDA writes per pixel (to be finalized before M3 lighting pass)
- [ ] Add API to renderer that allows to put chunk data into pipeline to render it
- [ ] Implement slang compute shader to 3D DDA raycast voxels on screen
- [ ] Render single chunk on screen

### M2. World && Streaming

We should have plain data, and systems to operate on it. DOD preferred.

- [ ] Create chunk storage struct that will store loaded chunks via flat hash map or some kind of associative container
- [ ] Create chunk generation system, inputs -- chunk coords, seed, maybe some params, output -- generated chunk
- [ ] Create chunk streaming system. Chunks should load in NxNxN cube around player, generate if they don't exist, and deload when out of render distance (for now just forget about them, later save to save file)
- [ ] Chunk generation is synchronous for now -- refactor to async worker pool in a future milestone

### M3. Rendering pt.2

This milestone focuses on enhancing visual quality and renderer architecture. Divided into 3 parts:

1. Adapting 3D DDA to work on multiple chunks
    - [ ] Introduce new world buffer on GPU
    - [ ] Rewrite main slang raycasting shader: two-level DDA, outer traverses chunks, inner traverses voxels within chunk
2. Basic lighting setup
    - [ ] Ambient -- constant factor
    - [ ] Point light -- position and radius. Examples: torch, lamp
3. Basic render graph
    - [/] Define resource lifetime tracking per pass (read/write access per resource) -- required for automatic barrier insertion
    - [/] Create render graph class: accepts passes, computes dependencies, inserts `vkCmdPipelineBarrier`s
    - [/] Using render graph, build the following pass sequence:
        - Geometry pass -- compute, DDA raycasting, writes G-buffer
        - Lighting pass -- compute, shadow ray raycasting, reads G-buffer, writes lit image
        - Debug UI pass -- graphics, draws ImGui overlay on lit image if enabled
        - Blit pass -- copies lit image into swapchain, transitions image layout

### M4. First playable

This milestone focuses on player interaction with the world.

- [ ] Create Player struct with position, velocity, and Camera
- [ ] Create basic physics system: AABB player vs AABB voxels collision
- [ ] Implement raypick -- DDA ray from camera, returns voxel coord and face
- [ ] Implement placing voxels on RMB
- [ ] Implement destroying voxels on LMB