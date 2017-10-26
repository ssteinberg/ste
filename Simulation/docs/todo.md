1. <strike>**primary_renderer**</strike>
     - <strike>Fragments must conform to ste_resource deferred</strike> 
     - <strike>Enumerate resources used for each primary renderer fragment</strike>
     - <strike>Primary renderer pipeline barriers</strike>
     - <strike>Decouple renderer and presentation engine</strike>
     - <strike>Light cascades</strike>
2. <strike>Camera: Allow projection change and signal changes</strike>
3. <strike>Pipeline layouts' exceptions: Mention offending variable names</strike>
4. <strike><font color="lightgray">ste_resource: uninitialized resource.</font></strike>
5. <strike>Non-templated resolutions for shadowmap storage</strike>
6. <strike>External binding sets: Specializeable constants</strike>
7. <strike>Correct constness for pipeline_layout and external binding sets</strike>
8. <strike>VK_KHR_dedicated_allocation</strike>
9. VK_KHR_16bit_storage
10. <strike>Atomic flags for pipeline layout and binding sets flags</strike>
11. <strike>Model factory adaptation to new engine</strike>
12. Text renderer: goto marker.
13. <font color="gray">Custom surface class</font>
     - <strike>Surfaces</strike>
     - <strike>Opaque surface</strike>
     - <strike>Load opaque surface + DDS</strike>
     - Samplers
     - <strike>Surface convert and copy</strike>
     - <strike>Accelerated operations /w AVX</strike>
     - Mipmap generation
     - Compressed blocks
     - Operations: Resize, reduce etc.
     - Advanced operations: Convolutions
     - Faster PNG loader
14. <strike>Concurrency support for GPU data structures</strike>
15. Incorporate optimizing GLSL/SPIR-v compiler (LunarGLASS?)
16. SPIR-v refection: OpSpecConstantOp
17. <strike>Integrate glslang into StE SPIR-v compiler. Compile all modules in a single go.</strike>
18. <strike>Device resources' debug names via VK_EXT_debug_marker</strike>
19. <strike>Sponza Vulkan demo UI and ImGUI</strike>
20. <strike>Profiler</strike>
21. <strike>Units, meters, km, kelvin, bytes, kb, etc.</strike>
22. Task scheduler and thread pool. Deal with blocking calls on workers better.
23. Queue transfer, correctly, in time.
24. <strike>Get rid of host commands altogether.</strike>
25. <strike>Fix bind sparse mess.</strike>
26. <strike>Host read-back, buffers and images.</strike>
27. Switch back to atomic counters (GLSL 4.6)
28. <strike>HDR: Temporal stablity of histogram tonemap.</strike>
29. HDR: Adaptation
30. Multi-submit command batch
31. Sparse images/opaque images
32. <strike>Semaphores enforce correct host-side happens-before relationship</strike>
33. Window resizing, correct resource deallocation and recreation.
34. Profiler capture and output
35. Types: Normalized types.
36. Voxel GI:
	- <strike>Sparse voxelizer</strike>
    - <strike>Voxel ray traversal</strike>
    - Voxel cone traversal
    - <strike>Voxel metadata</strike>
    - Voxel data interpolation
37. Polygonal lights: Beam focusing
38. Lights: Better surface area, fall-off and max distance calculation for non-spherical lights.
39. Geometry culling: Better bounding volume or low-poly geometry
40. <strike>std::hardware_destructive_interference_size</strike>
41. Make sense out of gl\::vector/gl\::stable_vector
42. <strike>VK_KHR_bind_memory2</strike>
43. LRUCache: Robustness