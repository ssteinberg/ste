 ފ         #     �                GLSL.std.450              	       main    �   �     �                                              �   
 GL_GOOGLE_cpp_style_line_directive       main         max(vf2;vf2;vf2;      
   x        y        z        min(vf2;vf2;vf2;         x        y        z        bounding_triangle_vertex(vf2;vf2;vf2;        prev         v        next         voxel_tree_block_extent      voxel_P  
 !   voxel_tree_initial_block_extent   "   voxel_Pi      %   voxel_grid_resolution     &   voxel_world   (   voxel_leaf_level      @   U     F   V     K   W     P   a     T   b     X   p0    \   p1    t   t     �   U     �   gl_PerVertex      �       gl_Position   �      gl_PointSize      �      gl_ClipDistance   �      gl_CullDistance   �   gl_in     �   V     �   W     �   N     �   material_id   �   vs_out    �       st    �      material_id   �   vin   �   s     �   recp_s    �   absN      �   signN     �   T     �   invT      �   p0    �   p1    �   p2    �   minv      �   param     �   param       param       geo_out         P          N          st         material_id        max_aabb        vout      
  param       param       param       transformed_N       d     "  param     %  param     (  param     .  param     1  param     4  param     :  param     =  param     @  param     v  v0      v1    �  v2    �  gl_PerVertex      �      gl_Position   �     gl_PointSize      �     gl_ClipDistance   �     gl_CullDistance   �       	 �  material_texture_descriptor   �      sampler_idx   �  material_descriptor   �      cavity_map    �     normal_map    �     mask_map      �     texture   �     emission     	 �     packed_emission_color     �     head_layer    �     material_flags   
 �  material_descriptors_binding      �      mat_descriptor    �       	 �  material_layer_descriptor    	 �      roughness_sampler_idx    	 �     metallicity_sampler_idx  	 �     thickness_sampler_idx     �     next_layer_id    	 �     attenuation_coefficient   �     packed_albedo     �     ior_phase_pack    �     _unused1      �     _unused2      �  material_layer_descriptors_binding   	 �      mat_layer_descriptor      �        �  material_textures_count   �  material_textures     �  material_sampler      �  voxel_buffer_binding      �      voxel_buffer      �        �  voxel_counter_binding     �      voxel_buffer_size     �        �  voxel_list_element_t      �      x     �     y     �     z     �     normal_and_material   �     voxel_node    �  voxel_list_binding    �      voxel_list_buffer     �       	 �  voxel_list_counter_binding   	 �      voxel_list_buffer_size    �      G           G  "         G  &         G  (         H  �              H  �            H  �            H  �            G  �      H  �         G  �      G  �          H          H          G       G           H  �             H  �           H  �           H  �           G  �     H  �      #       H  �      #       H  �     #      H  �     #      H  �     #      H  �     #      H  �     #      H  �     #      H  �     #      G  �         H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      H  �      #       H  �     #      H  �     #      H  �     #      H  �     #      H  �     #       H  �     #   $   H  �     #   (   H  �     #   ,   G  �     0   H  �         H  �         H  �      #       G  �     G  �  "      G  �  !      G  �         G  �  "      G  �  !      G  �  "      G  �  !      G  �        H  �         H  �      #       G  �     G  �  "      G  �  !       H  �         H  �      #       G  �     G  �  "      G  �  !      H  �      #       H  �     #      H  �     #      H  �     #      H  �     #      G  �        H  �         H  �      #       G  �     G  �  "      G  �  !      H  �         H  �      #       G  �     G  �  "      G  �  !           !                                        !  	                           ;                       +                        2           4        �         ;     !      2     "      4     #   �      "   ;     %      2     &    �;E4     '   �      "   2     (      +     )      4     *   �   (   )   4     +   �      *   4     ,   �      +   4     -   �   '   ,     >            ?      >   +     B     �?+     `      ?,     a   `   `   +     f         g           �           �      )     �   �      �   �   +     �        �   �   �      �      �   ;  �   �      +     �          �      �   +     �         �           �           �   �   �      �      �   ;  �   �         �           �     �   �        �   >         �      �   +     �       +     �           >   >                      ;         +     	     ,       B   B              +     {     F+     �        �           �           �     >     �  �      �   �      �     �  ;  �  �        �     �   +     �  �I@+     �  ��?+     �  �
�?+     �  �I?+     �  |� ?+     �  ���>+     �  ��"?+     �  Evt?+     �  ���?+     �  ��?+     �  ��@+     �  ��A+     �  �IA+     �  �S{A+     �  T�-@+     �  �Z�>+     �  ���.+     �  �O
+     �    �+     �     +     �     +     �     �  �      
 �  �  �  �  �                �  �    �  �     �     �  ;  �  �       �              �                 �  �    �  �     �     �  ;  �  �     2     �      	 �                             �  �  �     �      �  ;  �  �        �     �      �  ;  �  �      +     �  ����+     �  ��L>+     �  ���=+     �  fff?+     �  �bJ@+     �  ��>+     �    �@+     �       �       �  �     �     �  ;  �  �       �        �     �  ;  �  �       �                   �  �    �  �     �     �  ;  �  �       �        �     �  ;  �  �     +     �    �F6               �     ;  ?   �      ;  ?   �      ;  ?   �      ;  ?   �      ;  �   �      ;  g   �      ;  g   �      ;  ?   �      ;  ?   �      ;  �   �      ;  �   �      ;  ?   �      ;  ?   �      ;  ?   �      ;     �      ;     �      ;     �      ;          ;     
     ;          ;          ;  ?        ;  g        ;     "     ;     %     ;     (     ;     .     ;     1     ;     4     ;     :     ;     =     ;     @     ;     v     ;          ;     �     o            >         o     $   #   >  !   $   o     .   -   �     /   &   .   >  %   /   A  �   �   �   �   �   =  �   �   �   O  >   �   �   �             >  �   �   A  �   �   �      �   =  �   �   �   O  >   �   �   �             >  �   �   A  �   �   �   �   �   =  �   �   �   O  >   �   �   �             >  �   �   =  >   �   �   =  >   �   �   �  >   �   �   �   =  >   �   �   =  >   �   �   �  >   �   �   �     >   �      D   �   �     >   �      E   �   >  �   �   A  �   �   �   �      =     �   �   >  �   �   =     �   %   >  �   �   =     �   �   �     �   B   �   >  �   �   =  >   �   �     >   �         �   >  �   �   =  >   �   �     >   �         �   >  �   �   =  >   �   �   O     �   �   �           =  >   �   �   O     �   �   �         �  �   �   �   �   �  �   �   �   �  �       �  �   �   �   �  �   A  g   �   �   �   =     �   �        �   �   A  g   �   �   �   =     �   �   P  >   �   �   �   �   P  >   �   �   B   �   P  >   �   �   �   �   P  �   �   �   �   �   >  �   �   �  �   �  �   =  >   �   �   O     �   �   �         =  >   �   �   O     �   �   �          �  �   �   �   �   �  �   �   �   �  �       �  �   �   �   �  �   A  g   �   �   )   =     �   �        �   �   A  g   �   �   )   =     �   �   P  >   �   B   �   �   P  >   �   �   �   �   P  >   �   �   �   �   P  �   �   �   �   �   >  �   �   �  �   �  �   A  g   �   �   f   =     �   �   A  g   �   �   f   =     �   �   P  >   �   �   �   �   P  >   �   �   B   �   P  >   �   �   �   �   P  �   �   �   �   �   >  �   �   �  �   �  �   �  �   �  �   =  �   �   �   T  �   �   �   >  �   �   =  �   �   �   =     �   �   =  >   �   �   �  >   �   �   �   �  >   �   �   �   >  �   �   =  �   �   �   =     �   �   =  >   �   �   �  >   �   �   �   �  >   �   �   �   >  �   �   =  �   �   �   =     �   �   =  >   �   �   �  >   �   �   �   �  >   �   �   �   >  �   �   =  >   �   �   O     �   �   �          >  �   �   =  >      �   O                    >  �     =  >     �   O                  >      9          �   �     >  �     =  >     �   O                  >  
    =  >     �   O                  >      =  >     �   O                  >      9          
      =       �   �           �           A        	  >      =  �     �   =  >     �   �  >         >      =  >     �   =  >        �     !       >    !  =  >   #  �   O     $  #  #         >  "  $  =  >   &  �   O     '  &  &         >  %  '  =  >   )  �   O     *  )  )         >  (  *  9     +     "  %  (  =  >   ,  �   O  >   -  ,  +           >  �   -  =  >   /  �   O     0  /  /         >  .  0  =  >   2  �   O     3  2  2         >  1  3  =  >   5  �   O     6  5  5         >  4  6  9     7     .  1  4  =  >   8  �   O  >   9  8  7           >  �   9  =  >   ;  �   O     <  ;  ;         >  :  <  =  >   >  �   O     ?  >  >         >  =  ?  =  >   A  �   O     B  A  A         >  @  B  9     C     :  =  @  =  >   D  �   O  >   E  D  C           >  �   E  =     F    =  >   G  �   O     H  G  G         =  >   I    O     J  I  I         �     K  H  J  �     L  F  K  A  g   M    f   =     N  M  �     O  L  N  A  g   P  �   f   >  P  O  =     Q    =  >   R  �   O     S  R  R         =  >   T    O     U  T  T         �     V  S  U  �     W  Q  V  A  g   X    f   =     Y  X  �     Z  W  Y  A  g   [  �   f   >  [  Z  =     \    =  >   ]  �   O     ^  ]  ]         =  >   _    O     `  _  _         �     a  ^  `  �     b  \  a  A  g   c    f   =     d  c  �     e  b  d  A  g   f  �   f   >  f  e  =  �   g  �   =     h  �   =  >   i  �   �  >   j  i  h  �  >   k  g  j  >  �   k  =  �   l  �   =     m  �   =  >   n  �   �  >   o  n  m  �  >   p  l  o  >  �   p  =  �   q  �   =     r  �   =  >   s  �   �  >   t  s  r  �  >   u  q  t  >  �   u  =  >   w  �   O     x  w  w         =     y  �   �     z  x  y  P     |  {  {  �     }  z  |  �     ~  }    >  v  ~  =  >   �  �   O     �  �  �         =     �  �   �     �  �  �  P     �  {  {  �     �  �  �  �     �  �    >    �  =  >   �  �   O     �  �  �         =     �  �   �     �  �  �  P     �  {  {  �     �  �  �  �     �  �    >  �  �  =     �  �   A  �  �    �  >  �  �  A  �  �  �   �   �   =     �  �  A    �    �   >  �  �  =  >   �  �   A  �  �    �   >  �  �  =  >   �  �   A  �  �       >  �  �  =     �  v  Q     �  �      Q     �  �     P  �   �  �  �  �   B   A  �  �  �  �   >  �  �  �  A  �  �  �      �   =     �  �  A    �    �   >  �  �  =  >   �  �   A  �  �    �   >  �  �  =  >   �  �   A  �  �       >  �  �  =     �    Q     �  �      Q     �  �     P  �   �  �  �  �   B   A  �  �  �  �   >  �  �  �  A  �  �  �   �   �   =     �  �  A    �    �   >  �  �  =  >   �  �   A  �  �    �   >  �  �  =  >   �  �   A  �  �       >  �  �  =     �  �  Q     �  �      Q     �  �     P  �   �  �  �  �   B   A  �  �  �  �   >  �  �  �  �  �  8  6            	   7     
   7        7        �     =     0   
   =     1           2      (   0   1   =     3           4      (   2   3   �  4   8  6            	   7        7        7        �     =     7      =     8           9      %   7   8   =     :           ;      %   9   :   �  ;   8  6            	   7        7        7        �     ;  ?   @      ;  ?   F      ;  ?   K      ;  ?   P      ;  ?   T      ;  ?   X      ;  ?   \      ;  ?   t      =     A      Q     C   A       Q     D   A      P  >   E   C   D   B   >  @   E   =     G      Q     H   G       Q     I   G      P  >   J   H   I   B   >  F   J   =     L      Q     M   L       Q     N   L      P  >   O   M   N   B   >  K   O   =  >   Q   F   =  >   R   @   �  >   S   Q   R   >  P   S   =  >   U   K   =  >   V   F   �  >   W   U   V   >  T   W   =  >   Y   P   =  >   Z   @     >   [      D   Y   Z   >  X   [   =  >   ]   T   =  >   ^   F     >   _      D   ]   ^   >  \   _   =  >   b   X   O     c   b   b               d         c   �     e   a   d   A  g   h   X   f   =     i   h   �     j   i   e   A  g   k   X   f   >  k   j   =  >   l   \   O     m   l   l               n         m   �     o   a   n   A  g   p   \   f   =     q   p   �     r   q   o   A  g   s   \   f   >  s   r   =  >   u   X   =  >   v   \     >   w      D   u   v   >  t   w   =  >   x   t   O     y   x   x          A  g   z   t   f   =     {   z   P     |   {   {   �     }   y   |   �  }   8  