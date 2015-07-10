
#pragma once

class RenderControl
{
public:
	RenderControl();
	RenderControl(const RenderControl &) = delete;
	RenderControl(RenderControl &&) = delete;
	void operator=(const RenderControl &) = delete;
	void operator=(RenderControl &&) = delete;

	static const RenderControl &instance() {
		static RenderControl inst;
		return inst;
	}

	int init_rendering_context(float w, float h, bool fs);
	int init_rendering_context(float w, float h) { return this->init_rendering_context(w, h, false); }
};
