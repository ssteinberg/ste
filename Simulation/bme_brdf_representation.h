// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "common_brdf_representation.h"
#include "BRDF.h"

#include "StEngineControl.h"
#include "lru_cache.h"

#include "task.h"

#include "Log.h"
#include "AttributedString.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <map>
#include <vector>
#include <algorithm>

#include <exception>

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

namespace StE {
namespace Graphics {

class bme_brdf_representation {
public:
	static constexpr float resolution = .2f;
	static constexpr float bucket_size = 1.f;

	struct bme_brdf_descriptor_entry {
		float theta, phi;
		float brdf;
		glm::vec3 v;

		friend std::istream& operator>>(std::istream& istream, bme_brdf_descriptor_entry& entry) {
			std::string l;
			std::getline(istream, l, '\t');
			entry.theta = std::stof(l, nullptr);
			std::getline(istream, l, '\t');
			entry.phi = std::stof(l, nullptr);
			std::getline(istream, l);
			entry.brdf = std::stof(l, nullptr);

			entry.v = BxDF::omega(entry.theta, entry.phi);

			return istream;
		}

	};

private:
	using exitant_db = std::map<int, std::map<int, std::vector<bme_brdf_descriptor_entry>>>;
	using database_type = std::map<int, exitant_db>;

	struct exitant_db_descriptor {
		int in_theta;
		exitant_db db;
	};

	database_type database;

private:
	void append_entry(bme_brdf_descriptor_entry &&entry, exitant_db &db) const {
		int theta_bucket = static_cast<int>(entry.theta / bucket_size);
		int phi_bucket = static_cast<int>(entry.phi / bucket_size);

		db[theta_bucket][phi_bucket].push_back(std::move(entry));
	}

	task<exitant_db_descriptor> load_bme_brdf_task(const boost::filesystem::path &bme_data) const {
		return [=](optional<task_scheduler*> sched) {
			std::ifstream f(bme_data.string(), std::ifstream::in);

			exitant_db db;

			std::string l;
			float in_theta = -1;
			int in_phi = -1;
			while (std::getline(f, l)) {
				if (l[0] == '#') {
					if (l.find("intheta") == 1) in_theta = std::stof(l.substr(1 + sizeof("intheta")), nullptr);
					if (l.find("inphi") == 1) in_phi = std::stol(l.substr(1 + sizeof("inphi")), nullptr);
					continue;
				}

				assert(in_theta >= 0 && in_phi >= 0);

				std::stringstream ss(l);
				bme_brdf_descriptor_entry entry;
				ss >> entry;

				if (entry.brdf > .0f) {
					entry.phi -= in_phi;
					while (entry.phi > BRDF::phi_max) entry.phi -= 360.f;
					while (entry.phi < BRDF::phi_min) entry.phi += 360.f;
					append_entry(std::move(entry), db);
				}
			}

			exitant_db_descriptor desc;
			desc.db = std::move(db);
			desc.in_theta = glm::round(in_theta);
			return desc;
		};
	}

	void append_exitant_db(exitant_db_descriptor &&desc) {
		database[desc.in_theta] = std::move(desc.db);
	}

	void load(const StEngineControl &context, const boost::filesystem::path &bme_data_dir) {
		std::vector<std::future<exitant_db_descriptor>> futures;
		for (boost::filesystem::directory_iterator it(bme_data_dir); it != boost::filesystem::directory_iterator(); ++it)
			if (boost::filesystem::is_regular_file(it->path()))
				futures.push_back(context.scheduler().schedule_now(load_bme_brdf_task(it->path())));

		for (auto &f : futures) {
			auto db = std::move(f.get());
			if (db.db.size())
				append_exitant_db(std::move(db));
		}
	}

	bme_brdf_representation() = default;
	~bme_brdf_representation() = default;

public:
	static task<std::unique_ptr<BRDF>> BRDF_from_bme_representation_task(const StEngineControl &context, const boost::filesystem::path &bme_data_dir) {
		const StEngineControl *ctx = &context;
		std::string cache_key = std::string("bme_brdf_representation_") + bme_data_dir.string();
		return task<common_brdf_representation> ([=](optional<task_scheduler*> sched) {
			common_brdf_representation brdfdata;
			try {
				auto cache_get_task = ctx->cache().get<common_brdf_representation>(cache_key);
				optional<common_brdf_representation> opt = cache_get_task();
				if (opt) {
					using namespace StE::Text::Attributes;
					ste_log() << b("bme_brdf_representation") + " - BRDF loaded from cache: " + i(bme_data_dir.string()) << std::endl;
					return std::move(opt.get());
				}
			}
			catch (std::exception *ex) {}

			{
				using namespace StE::Text::Attributes;
				ste_log() << b("bme_brdf_representation") + " - Loading BRDF: " + i(bme_data_dir.string()) << std::endl;
			}

			bme_brdf_representation bme;
			bme.load(*ctx, bme_data_dir);

			glm::tvec3<std::size_t> dims;
			dims.x = glm::ceil((BRDF::phi_max - BRDF::phi_min) / resolution + 1);
			dims.y = glm::ceil((BRDF::theta_max - BRDF::theta_min) / resolution + 1);
			dims.z = bme.database.size();

			brdfdata.get_data() = gli::texture2DArray(dims.z, 1, gli::format::FORMAT_R32_SFLOAT, dims.xy);
			brdfdata.set_min_incident(bme.database.begin()->first);
			brdfdata.set_max_incident((--bme.database.end())->first);

			std::vector<std::future<void>> futures;

			auto it = bme.database.begin();
			for (unsigned i = 0; i < dims.z; ++i, ++it) {
				futures.push_back(ctx->scheduler().schedule_now([&, i=i, it=it](optional<task_scheduler*> sched) {
					float *data = reinterpret_cast<float*>(brdfdata.get_data()[i].data());
					exitant_db &db = it->second;

					for (unsigned j = 0; j < dims.y; ++j) {
						float theta = glm::mix<float>(BRDF::theta_min, BRDF::theta_max, static_cast<float>(j) / static_cast<float>(dims.y - 1));

						for (unsigned k = 0; k < dims.x; ++k, ++data) {
							float phi = glm::mix<float>(BRDF::phi_min, BRDF::phi_max, static_cast<float>(k) / static_cast<float>(dims.x - 1));

							glm::vec3 w = BxDF::omega(theta, phi);

							std::vector<bme_brdf_descriptor_entry*> entries;
							int theta_bucket = static_cast<int>(theta / bucket_size);
							int phi_bucket = static_cast<int>(phi / bucket_size);

							constexpr int samples = 15;
							int bias = 1;
							while (entries.size() < samples) {
								for (int x = -bias; x <= bias; ++x) {
									for (auto &e : db[theta_bucket + x][phi_bucket + bias])
										entries.push_back(&e);
									if (bias)
										for (auto &e : db[theta_bucket + x][phi_bucket - bias])
											entries.push_back(&e);
								}
								for (int x = -bias + 1; x < bias; ++x) {
									for (auto &e : db[theta_bucket + bias][phi_bucket + x])
										entries.push_back(&e);
									if (bias)
										for (auto &e : db[theta_bucket - bias][phi_bucket + x])
											entries.push_back(&e);
								}

								++bias;
							}
							for (auto &e : db[theta_bucket][phi_bucket])
								entries.push_back(&e);

							assert(entries.size());

							using closest_vector_type = std::vector<std::pair<float, bme_brdf_descriptor_entry*>>;
							closest_vector_type closest;
							float max_w = .0f, min_w = 1.f;
							for (auto &entry : entries) {
								float d = glm::dot(entry->v, w);
								max_w = glm::max(d, max_w);
								min_w = glm::min(d, min_w);

								auto pair = std::make_pair(d, entry);
								if (closest.size() > samples && closest[closest.size() - 1].first > d) continue;
								closest.insert(std::lower_bound(closest.begin(), closest.end(), pair, [](const closest_vector_type::value_type &lhs, const closest_vector_type::value_type &rhs) {
									return lhs.first > rhs.first;
								}), pair);

								if (closest.size() > samples) closest.pop_back();
							}

							float tweight = .0f;
							float brdf = .0f;
							for (auto &e : closest) {
								float weight = max_w - min_w > 0 ? glm::pow((e.first - min_w) / (max_w - min_w), 2.f) : 1.f;
								brdf += e.second->brdf * weight;
								tweight += weight;
							}
							brdf /= tweight;

							*data = closest[0].second->brdf;
						}
					}
				}));
			}

			for (auto &f : futures)
				f.wait();

			ctx->cache().insert(cache_key, brdfdata);
			return std::move(brdfdata);
		}).then_on_main_thread([=](optional<task_scheduler*> sched, common_brdf_representation &&data) {
			auto ptr = std::make_unique<BRDF>(std::move(data));
			return ptr;
		});
	}
};

}
}
