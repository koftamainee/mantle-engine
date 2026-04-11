### M0. Cleanup

- [ ] Split up renderer into frame sheduler and render pass, also impl should not have any methods
- [ ] Create proper descriptor pools, sets, set layouts management, maybe separate vulkan object (bindless architecture ????)
- [ ] Remove mvp from push constants and pass them via separate 3 matrices via descriptor sets
- [ ] Implement pool/slab allocator and its pmr version
- [ ] replace all stl containers with pmr versions using custom allocators, or probably just avoid stl in hot paths
- [ ] Initialize imgui in vulkan backend, prob carry out into separate class
- [ ] Add memory profiling stats to existing allocators
- [ ] Remove mangohud fps overlay, add new debug overlay in imgui (F3, F3 + 1, 2, 3, 4)

### M1. Voxel rendering
The basic idea: cpu meshing is too expensive for processing huge amount of small voxels, gpu meshing will probably be ok, but struggle with destructions, we will need to rebuild entire mesh on each player interaction. this is quite expensive
#### New approach
Instead of rasterizing voxels we will path trace them using some kind of 3D DDA algorithms, all of it in compute shader

- [ ] Create compute pipeline class in vulkan abstractions
- [ ] Create a DOD style Chunk struct, voxels should be array of u32/u16/u8, bit packed with material palettes, some flags, params like temperature, pressure, etc, for later, dont mind them yet
- [ ] Add API to renderer that allows to put in chunk data into pipeline to render it
- [ ] Implement slang compute shader to 3D DDA ray cast voxel on a screen
- [ ] Render single chunk on a screen

### M2. World && Streaming
We should have plain data, and system to operate on it. DOD is preffered
- [ ] Create chunk storage struct that will store loaded chunks via flat hash map or some kind of associative container
- [ ] Create chunk generation system, inputs -- chunk coords, seed, maybe some params, output -- generated chunk
- [ ] Create chunk streaming system. Chunks should load in NxNxN cube around player, generate if they not exist and deload when out of render distance (for now just forget about them, later save in some save file)

### M3. Rendering pt.2
This milestone focuses on enchansing visual quality of image and also renderer architecture. It can be divided into 3 parts: 
1. Adapting 3D DDA to work on multiple chunks
	- [ ] Introduce new world buffer on gpu
	- [ ] Rewrite main slangc raycasting shader, now there should be 2 DDAs, one main in chunks, and another secondary on voxels in chunk
2. Basic lighting setup: implement this types of light
	- [ ] Ambient -- just constant factor
	- [ ] Point light -- light point and radius. Examples: torch, lamp
	- [ ] (opt) Spot light: light in some direction: Example: flashlight
3. Basic render graph. Now we have lighting it will be usefull to have structure that will auto arrange render passes, so we dont manually specify order
	- [ ] Create render graph class. it should accept our passes and compute dependencies for each other arrange them and insert needed vkCmdPipelineBarrier's
	- [ ] Using render graph build following sequence of passes:
	      - Geometry pass -- compute, DDA raycasting. Writes G-buffer
	      - Lighting pass -- compute, shadow rays raycasting, reads G-buffer, writes lit image
	      - Debug UI pass -- graphics, draw debug ImGui ui if needed on lit image
	      - Blit pass -- copy lit image into swapchain, optionally transitions image layout

### M4. First playable
This milestone focuses on player interaction with world.
- [ ] Create Player struct with position, velocity, and Camera on it
- [ ] Create basic physics system that will handle player collisions with voxels (AABB of voxels vs AABBs of player)
- [ ] Implement raypick -- DDA ray from camera, returns voxel coord and side
- [ ] Implement building voxels on RMB
- [ ] Implement destroying voxels on LMB