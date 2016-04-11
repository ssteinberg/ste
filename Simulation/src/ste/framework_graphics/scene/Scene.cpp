
#include "stdafx.hpp"
#include "Scene.hpp"

using namespace StE::Graphics;

Scene::Scene(Base::AccessToken, StEngineControl &ctx) : Base(Base::AccessToken(), "scene", &objects),
														objects(ctx, &scene_props),
														shadows_projector(ctx, &objects, &scene_props.lights_storage(), &shadows_storage) {
	this->add_dependency(make_gpu_task("shadow_projector", &shadows_projector, shadows_storage.get_fbo()));
}
