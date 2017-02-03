
const uint light_type_directional_bit = 1 << 0;
const uint light_type_shape_bit = 1 << 1;
const uint light_type_two_sided_bit = 1 << 2;
const uint light_type_textured_bit = 1 << 3;

const uint light_shape_sphere = (0 & 7) << 4;
const uint light_shape_quad = (1 & 7) << 4;
const uint light_shape_polygon = (2 & 7) << 4;
const uint light_shape_convex_polyhedron = (3 & 7) << 4;

// Virtual lights
const uint LightTypePoint = 0x0;
const uint LightTypeDirection = light_type_directional_bit;

// Shape lights
const uint LightTypeSphere = light_type_shape_bit | light_shape_sphere | light_type_two_sided_bit;
const uint LightTypeQuadOnesided = light_type_shape_bit | light_shape_quad;
const uint LightTypeQuadTwosided = light_type_shape_bit | light_shape_quad | light_type_two_sided_bit;
const uint LightTypeQuadTexturedOnesided = light_type_shape_bit | light_shape_quad | light_type_textured_bit;
const uint LightTypeQuadTexturedTwosided = light_type_shape_bit | light_shape_quad | light_type_textured_bit | light_type_two_sided_bit;
const uint LightTypePolygonOnesided = light_type_shape_bit | light_shape_polygon;
const uint LightTypePolygonTwosided = light_type_shape_bit | light_shape_polygon | light_type_two_sided_bit;
const uint LightTypeConvexPolyhedron = light_type_shape_bit | light_shape_convex_polyhedron;

// Light type queries
bool light_type_is_point(uint type) {
	return type == LightTypePoint;
}

bool light_type_is_directional(uint type) {
	return type == LightTypeDirection;
}

bool light_type_is_shaped(uint type) {
	return (type & light_type_shape_bit) != 0;
}

bool light_type_is_two_sided(uint type) {
	return (type & light_type_two_sided_bit) != 0;
}

bool light_type_is_textured(uint type) {
	return (type & light_type_textured_bit) != 0;
}

bool light_shape_is_sphere(uint type) {
	return (type & light_shape_sphere) == light_shape_sphere;
}

bool light_shape_is_quad(uint type) {
	return (type & light_shape_quad) == light_shape_quad;
}

bool light_shape_is_polygon(uint type) {
	return (type & light_shape_polygon) == light_shape_polygon;
}

bool light_shape_is_convex_polyhedron(uint type) {
	return (type & light_shape_convex_polyhedron) == light_shape_convex_polyhedron;
}
