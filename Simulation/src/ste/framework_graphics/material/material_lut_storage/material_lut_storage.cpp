
#include <stdafx.hpp>
#include <material_lut_storage.hpp>

using namespace ste::graphics;

const char *material_lut_storage::refraction_fit_path = R"(Data/microfacet_ggx_refraction_fit.bin)";
const char *material_lut_storage::transmission_fit_path = R"(Data/microfacet_ggx_transmission_fit.bin)";

const char *material_lut_storage::ggx_tab_path = R"(Data/ltc_ggx_fit.dds)";
const char *material_lut_storage::ggx_amp_path = R"(Data/ltc_ggx_amplitude.dds)";
