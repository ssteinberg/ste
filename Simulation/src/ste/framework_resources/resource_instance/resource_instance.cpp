
#include "stdafx.hpp"
#include "resource_instance.hpp"

using namespace StE::Resource;

thread_local std::stack<const resource_instance_base*> resource_instance_base::resources_being_ctored_stack;
