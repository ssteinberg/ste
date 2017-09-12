1. **primary_renderer**
     - <strike>Fragments must conform to ste_resource deferred</strike> 
     - Switch back to atomic counters (GLSL 4.6)
     - <strike>Enumerate resources used for each primary renderer fragment</strike>
     - <strike>Primary renderer pipeline barriers</strike>
     - <strike>Decouple renderer and presentation engine</strike>
     - <strike>Light cascades</strike>
2. <strike>Camera: Allow projection change and signal changes</strike>
3. Pipeline layouts' exceptions: Mention offending variable names
4. <strike><font color="lightgray">ste_resource: uninitialized resource.</font></strike>
5. <strike>Non-templated resolutions for shadowmap storage</strike>
6. <strike>External binding sets: Specializeable constants</strike>
7. <strike>Correct constness for pipeline_layout and external binding sets</strike>
8. VK_KHR_dedicated_allocation
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
     - Accelerated operations /w AVX
     - Mipmap generation
     - Compressed surfaces
     - Operations: Resize, reduce etc.
     - Advanced operations: Convolutions
14. <strike>Concurrency support for GPU data structures</strike>
15. Incorporate optimizing GLSL/SPIR-v compiler (LunarGLASS?)
16. SPIR-v refection: OpSpecConstantOp
17. <strike>Integrate glslang into StE SPIR-v compiler. Compile all modules in a single go.</strike>
18. <strike>Device resources' debug names via VK_EXT_debug_marker</strike>
19. Sponza Vulkan demo UI and ImGUI
20. Profiler
21. Units, meters, km, kelvin, bytes, kb, etc.
22. Task scheduler and thread pool. Deal with blocking calls on workers better.
23. Queue transfer, correctly, in time.
24. Get rid of host commands altogether.
25. Fix bind sparse mess.