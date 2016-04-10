
#include "stdafx.hpp"
#include "Scene.hpp"

using namespace StE::Graphics;

Scene::Scene(Base::AccessToken, StEngineControl &ctx) : Base(Base::AccessToken(), "scene", &objects),
														objects(ctx, &scene_props) {
}
