//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Provides facilities to predict the next frame time.
 *			Uses an Extended Kalman filter.
 */
class presentation_frame_time_predictor {
private:
	void *pimpl;

	float current{ .0f };
	float predicted{ .0f };

public:
	/**
	*	@brief	Predictor ctor.
	*	
	*	@param	covariance	Frame time noise covariance
	*/
	presentation_frame_time_predictor(float covariance = .25f);
	~presentation_frame_time_predictor() noexcept;

	/**
	 *	@brief	Update with next frame time, and computes the prediction.
	 *			Should be called each frame.
	 */
	void update(std::uint64_t ns);

	/**
	 *	@brief	Returns the predicted next frame time in milliseconds.
	 */
	auto predicted_value() const { return predicted; }
};

}
}
