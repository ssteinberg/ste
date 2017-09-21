This contains detailed information about StE's primary renderer, Sep 2017.

**<h1><font color="blue">Pipline</font></h1>**<br>

**light_preprocess**

Compute stage (light_preprocess_cull_lights.comp):

 - *<font color="gray">(push_constant)</font> clip_planes*
 - <font color="red">(buffer readwrite)</font> ll_counter
 - <font color="red">(buffer writeonly)</font> ll
 - <font color="red">(buffer readwrite)</font> light_buffer
 - <font color="green">(buffer readonly)</font> view_transform_buffer
 - <font color="green">(buffer readonly)</font> atmospherics_descriptor_data

**<h3>⇩</h3>** 

**scene_geo_cull**

Compute stage (scene_geo_cull.comp):

 - <font color="red">(local buffer readwrite)</font> counter
 - <font color="red">(local buffer writeonly)</font> idb
 - <font color="red">(local buffer writeonly)</font> sidb
 - <font color="red">(local buffer writeonly)</font> dsidb
 - <font color="red">(local buffer writeonly)</font> ttl
 - <font color="red">(local buffer writeonly)</font> d_ttl
 - <font color="green">(buffer readonly)</font> ll_counter
 - <font color="green">(buffer readonly)</font> ll
 - <font color="green">(buffer readonly)</font> light_buffer
 - <font color="green">(buffer readonly)</font> light_cascades
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer
 - <font color="green">(buffer readonly)</font> mesh_draw_params_buffer

**<h3>⇩</h3>**  

**shadow_projector**

Vertex stage (shadow_cubemap.vert):

 - <font color="green">(indirect buffer)</font> sidb
 - <font color="green">(vertex buffer)</font> vertex_buffer
 - <font color="green">(index buffer)</font> index_buffer
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer

Geometry stage (shadow_cubemap.geom):

 - <font color="green">(local buffer readonly)</font> ttl
 - <font color="green">(buffer readonly)</font> light_buffer

Fragment stage (fixed):

 - <font color="red">(framebuffer depth attachment write)</font> shadow_depth_cube_maps

**<h3>⇩</h3>** 

**directional_shadow_projector**

Vertex stage (shadow_directional.vert):

 - <font color="green">(indirect buffer)</font> dsidb
 - <font color="green">(vertex buffer)</font> vertex_buffer
 - <font color="green">(index buffer)</font> index_buffer
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer

Geometry stage (shadow_directional.geom):

 - <font color="green">(local buffer readonly)</font> d_ttl
 - <font color="green">(buffer readonly)</font> light_buffer
 - <font color="green">(buffer readonly)</font> light_cascades

Fragment stage (fixed):

 - <font color="red">(framebuffer depth attachment write)</font> directional_shadow_maps

**<h3>⇩</h3>**  

**prepopulate_depth**

Vertex stage (scene_transform.vert):

 - <font color="green">(indirect buffer)</font> idb
 - <font color="green">(vertex buffer)</font> vertex_buffer
 - <font color="green">(index buffer)</font> index_buffer
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer
 - <font color="green">(buffer readonly)</font> view_transform_buffer

Fragment stage (scene_prepopulate_depth.frag):

 - <font color="green">(sampler2D array readonly)</font> material_textures
 - <font color="green">(sampler readonly)</font> material_sampler
 - <font color="green">(buffer readonly)</font> mat_descriptor
 - <font color="red">(framebuffer depth attachment write)</font> gbuffer depth

**<h3>⇩</h3>** 

**downsample_depth**

Compute stage (gbuffer_downsample_depth.comp):

 - <font color="green">(sampler2D readonly)</font> gbuffer depth
 - <font color="red">(local storage image writeonly)</font> gbuffer downsampled-depth

**<h3>⇩</h3>** 

**linked_light_lists_generator**

Compute stage (linked_light_lists_gen.comp):

 - <font color="green">(sampler2D readonly)</font> gbuffer downsampled-depth
 - <font color="green">(buffer readonly)</font> ll_counter
 - <font color="green">(buffer readonly)</font> ll
 - <font color="green">(buffer readonly)</font> light_buffer
 - <font color="red">(local buffer readwrite)</font> lll_counter
 - <font color="red">(local buffer writeonly)</font> lll_buffer
 - <font color="red">(storage image writeonly)</font> linked_light_list_heads
 - <font color="red">(storage image writeonly)</font> linked_light_list_low_detail_heads
 - <font color="red">(storage image writeonly)</font> linked_light_list_size
 - <font color="red">(storage image writeonly)</font> linked_light_list_low_detail_size

**<h3>⇩</h3>** 

**scene**

Vertex stage (scene_transform.vert):

 - <font color="green">(indirect buffer)</font> idb
 - <font color="green">(vertex buffer)</font> vertex_buffer
 - <font color="green">(index buffer)</font> index_buffer
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer
 - <font color="green">(buffer readonly)</font> view_transform_buffer

Fragment stage (object.frag):

 - <font color="green">(sampler2D array readonly)</font> material_textures
 - <font color="green">(sampler readonly)</font> material_sampler
 - <font color="green">(buffer readonly)</font> mat_descriptor
 - <font color="red">(framebuffer color attachment write)</font> gbuffer[0]
 - <font color="red">(framebuffer color attachment write)</font> gbuffer[1]

**<h3>⇩</h3>** 

**prepopulate_depth_backface**

Vertex stage (scene_transform.vert):

 - <font color="green">(indirect buffer)</font> idb
 - <font color="green">(vertex buffer)</font> vertex_buffer
 - <font color="green">(index buffer)</font> index_buffer
 - <font color="green">(buffer readonly)</font> mesh_descriptor_buffer
 - <font color="green">(buffer readonly)</font> view_transform_buffer

Fragment stage (scene_prepopulate_depth.frag):

 - <font color="green">(sampler2D array readonly)</font> material_textures
 - <font color="green">(sampler readonly)</font> material_sampler
 - <font color="green">(buffer readonly)</font> mat_descriptor
 - <font color="red">(framebuffer depth attachment write)</font> gbuffer back-face depth

**<h3>⇩</h3>** 

**volumetric_scattering**

Compute stage (volumetric_scattering_scatter.comp):

 - <font color="red">(local storage image writeonly)</font> volumetric scattering volume
 - <font color="green">(buffer readonly)</font> ltc_points
 - <font color="green">(samplerCubeArrayShadow readonly)</font> shadow_depth_maps
 - <font color="green">(samplerCubeArray readonly)</font> shadow_maps
 - <font color="green">(sampler2DArrayShadow readonly)</font> directional_shadow_depth_maps
 - <font color="green">(sampler2DArray readonly)</font> directional_shadow_maps
 - <font color="green">(sampler2D readonly)</font> gbuffer depth
 - <font color="green">(sampler2D readonly)</font> gbuffer downsampled-depth
 - <font color="green">(local buffer readwrite)</font> lll_buffer
 - <font color="green">(storage image readonly)</font> linked_light_list_low_detail_heads
 - <font color="green">(storage image readonly)</font> linked_light_list_low_detail_size
 - <font color="green">(buffer readonly)</font> light_buffer
 - <font color="green">(buffer readonly)</font> light_cascades
 - <font color="green">(buffer readonly)</font> view_transform_buffer

**<h3>⇩</h3>** 

**composer**

Fragment stage (deferred_compose.frag):

 - <font color="green">(local sampler3D readonly)</font> volumetric scattering volume
 - <font color="green">(local sampler2D readonly)</font> microfacet_refraction_fit_lut
 - <font color="green">(local sampler2DArray readonly)</font> microfacet_transmission_fit_lut
 - <font color="green">(local sampler2D readonly)</font> ltc_ggx_fit
 - <font color="green">(local sampler2D readonly)</font> ltc_ggx_amplitude
 - <font color="green">(sampler2DArray readonly)</font> gbuffer
 - <font color="green">(sampler2D readonly)</font> gbuffer depth
 - <font color="green">(sampler2D readonly)</font> gbuffer back-face depth
 - <font color="green">(buffer readonly)</font> view_transform_buffer
 - <font color="green">(buffer readonly)</font> ltc_points
 - <font color="green">(samplerCubeArrayShadow readonly)</font> shadow_depth_maps
 - <font color="green">(samplerCubeArray readonly)</font> shadow_maps
 - <font color="green">(sampler2DArrayShadow readonly)</font> directional_shadow_depth_maps
 - <font color="green">(sampler2DArray readonly)</font> directional_shadow_maps
 - <font color="green">(local buffer readwrite)</font> lll_buffer
 - <font color="green">(storage image readonly)</font> linked_light_list_heads
 - <font color="green">(storage image readonly)</font> linked_light_list_size
 - <font color="green">(buffer readonly)</font> light_buffer
 - <font color="green">(buffer readonly)</font> light_cascades
 - <font color="green">(sampler2D array readonly)</font> material_textures
 - <font color="green">(sampler readonly)</font> material_sampler
 - <font color="green">(buffer readonly)</font> mat_descriptor
 - <font color="green">(buffer readonly)</font> mat_layer_descriptor

**<h3>⇩</h3>** 

**hdr**

Self sufficient

**<h3>⇩</h3>** 

**fxaa**

Self sufficient


**<h1><font color="blue">Pipeline Resource Barriers</font></h1>**<br>

**light_buffer**: <font color="red">light_preprocess (compute rw)</font> ➧ <font color="green">scene_geo_cull (compute r)</font> ➧ <font color="green">shadow_projector (geometry r)</font> ➧ <font color="green">directional_shadow_projector (geometry r)</font> ➧ <font color="green">linked_light_lists_generator (compute r)</font> ➧ <font color="green">volumetric_scattering (compute r)</font> ➧ <font color="green">composer (frag r)</font>

**ll_counter**: <font color="magenta">clear (transfer w)</font> ➧ <font color="red">light_preprocess (compute rw)</font> ➧ <font color="green">scene_geo_cull (compute r)</font> ➧ <font color="green">linked_light_lists_generator (compute r)</font>

**ll**: <font color="red">light_preprocess (compute rw)</font> ➧ <font color="green">scene_geo_cull (compute r)</font> ➧ <font color="green">linked_light_lists_generator (compute r)</font>

**lll_counter**: <font color="magenta">clear (transfer w)</font> ➧ <font color="red">linked_light_lists_generator (compute rw)</font>

**lll_buffer**: <font color="red">linked_light_lists_generator (compute rw)</font> ➧ <font color="green">volumetric_scattering (compute r)</font> ➧ <font color="green">composer (frag r)</font>

**linked_light_list_heads**: <font color="red">linked_light_lists_generator (compute rw)</font> ➧ <font color="green">composer (frag r)</font>

**linked_light_list_size**: <font color="red">linked_light_lists_generator (compute rw)</font> ➧ <font color="green">composer (frag r)</font>

**linked_light_list_low_detail_heads**: <font color="red">linked_light_lists_generator (compute rw)</font> ➧ <font color="green">volumetric_scattering (compute r)</font>

**linked_light_list_low_detail_size**: <font color="red">linked_light_lists_generator (compute rw)</font> ➧ <font color="green">volumetric_scattering (compute r)</font>

**gbuffer downsampled-depth**: <font color="orange">downsample_depth (depth attachment output w)</font> ➧ <font color="green">linked_light_lists_generator (compute r)</font> ➧ <font color="green">volumetric_scattering (compute r)</font>

**gbuffer depth**: <font color="orange">prepopulate_depth (depth attachment output w)</font> ➧ <font color="green">downsample_depth (compute r)</font> ➧ <font color="green">volumetric_scattering (compute r)</font> ➧ <font color="green">composer (frag r)</font>

**gbuffer back-face depth**: <font color="orange">prepopulate_depth_backface (depth attachment output w)</font> ➧ <font color="green">composer (frag r)</font>

**gbuffer**: <font color="orange">scene (color attachment output w)</font> ➧ <font color="green">composer (frag r)</font>

**shadow_depth_cube_maps**: <font color="orange">shadow_projector (depth attachment output w)</font> ➧ <font color="green">volumetric_scattering (compute r)</font> ➧ <font color="green">composer (frag r)</font>

**directional_shadow_maps**: <font color="orange">directional_shadow_projector (depth attachment output w)</font> ➧ <font color="green">volumetric_scattering (compute r)</font> ➧ <font color="green">composer (frag r)</font>

**<h1><font color="blue">Common Descriptor Set</font></h1>**<br>

**renderer_transform_buffers**

uniform buffer 0 --- view transform<br/>
uniform buffer 1 --- projection transform<br/>

****
**mesh**

buffer 2 --- mesh decriptors<br/>
buffer 3 --- mesh draw parameters<br/>

****
**material**

buffer 4 --- material descriptors<br/>
buffer 5 --- material layers descriptors<br/>
uniform 13  -- material textures (length-specializeable dynamic array)<br/>
uniform 14  -- material sampler<br/>

****
**light**

buffer 6 --- lights buffer<br/>
buffer 7 --- active-light-list atomic counter<br/>
buffer 8 --- active-light-list buffer<br/>
buffer 11 --- shaped lights' points<br/>

buffer 9 --- linked-light-list atomic counter<br/>
buffer 10 --- linked-light-list buffer<br/>

uniform 15 -- per-tile linked-light-lists size map (storage image)<br/>
uniform 16 -- per-tile linked-light-lists heads map (storage image)<br/>

(inactive)<br/>
uniform - -- per-tile linked-light-lists low-detail size map (storage image)<br/>
uniform - -- per-tile linked-light-lists low-detail heads map (storage image)<br/>

buffer - --- Directional light cascades<br/>
uniform buffer - -- Directional light cascade depths<br/>

****
**gbuffer**

uniform 17 -- gbuffer (sampler2DArray)<br/>
uniform 18 -- scene depth map (sampler2D)<br/>
uniform 19 -- downsampled scene depth map (sampler2D)<br/>
uniform 20 -- scene back-face depth map (sampler2D)<br/>

****
**shadows (inactive)**

uniform - -- shadow depth cubemaps (samplerCubeArrayShadow)<br/>
uniform - -- shadow cubemaps (samplerCubeArray)<br/>
uniform - -- directional shadow depth cascades (sampler2DArrayShadow)<br/>
uniform - -- directional shadow cascades (sampler2DArray)<br/>

****
**Atmospherics**

uniform buffer 12 --- atmospherics descriptor<br/>
uniform 21 -- atmospheric optical length lut (sampler2DArray)<br/>
uniform 22 -- atmospheric mie0 scattering lut (sampler3D)<br/>
uniform 23 -- atmospheric ambient lut (sampler3D)<br/>
uniform 24 -- atmospheric scattering lut (sampler3D)<br/>
