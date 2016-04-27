
#include "stdafx.hpp"
#include "Scene.hpp"

using namespace StE::Graphics;

Scene::Scene(Base::AccessToken,
			 const StEngineControl &ctx) : Base(Base::AccessToken(), "scene", &objects),
					  					   objects(ctx, &scene_props) {}
