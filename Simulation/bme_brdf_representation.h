// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

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

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

namespace StE {
namespace Graphics {

class bme_brdf_representation {
public:
	static constexpr float resolution = .5f;

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
			float len = glm::length(entry.v);

			return istream;
		}

	};

private:
	using exitant_db = std::vector<bme_brdf_descriptor_entry>;
	using database_type = std::map<int, exitant_db>;

	struct exitant_db_descriptor {
		int in_theta;
		exitant_db db;
	};

	struct brdf_data {
		int in_min, in_max;
		gli::texture3D tex;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
			ar << in_min;
			ar << in_max;

			ar << tex.dimensions().x;
			ar << tex.dimensions().y;
			ar << tex.dimensions().z;

			std::string data;
			auto size = tex.dimensions().x * tex.dimensions().y * tex.dimensions().z * sizeof(float);
			data.resize(size);
			memcpy(&data[0], tex.data(), size);
			ar << data;
		}
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
			ar >> in_min;
			ar >> in_max;

			unsigned x, y, z;
			ar >> x;
			ar >> y;
			ar >> z;
			tex = gli::texture3D(1, gli::format::FORMAT_R32_SFLOAT, { x,y,z });
			auto size = x*y*z*sizeof(float);

			std::string data;
			ar >> data;
			if (data.size() != size)
				throw new std::exception("Failed to deserialize brdf_data");
			memcpy(tex.data(), &data[0], size);
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
	};

	database_type database;

private:
	void append_entry(const bme_brdf_descriptor_entry &entry, exitant_db &db) const {
		db.push_back(entry);
	}

	task<exitant_db_descriptor> load_bme_brdf_task(const boost::filesystem::path &bme_data) const {
		return [=](optional<task_scheduler*> sched) {
			std::ifstream f(bme_data.string(), std::ifstream::in);

			exitant_db db;

			std::string l;
			float in_theta = -1;
			while (std::getline(f, l)) {
				if (l[0] == '#') {
					if (l.find("intheta") == 1) in_theta = std::stof(l.substr(1 + sizeof("intheta")), nullptr);
					if (l.find("inphi") == 1) {
						int inphi = std::stol(l.substr(1 + sizeof("inphi")), nullptr);
						if (inphi != 0)
							return exitant_db_descriptor();
					}
					continue;
				}

				std::stringstream ss(l);
				bme_brdf_descriptor_entry entry;
				ss >> entry;
				append_entry(entry, db);
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
		return task<brdf_data> ([=](optional<task_scheduler*> sched) {
			std::string cache_key = std::string("bme_brdf_representation_") + bme_data_dir.string();
			brdf_data brdfdata;
			try {
				auto cache_get_task = ctx->cache().get<brdf_data>(cache_key);
				optional<brdf_data> opt = cache_get_task();
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

			glm::ivec3 dims;
			dims.x = glm::ceil((BRDF::phi_max - BRDF::phi_min) / resolution + 1);
			dims.y = glm::ceil((BRDF::theta_max - BRDF::theta_min) / resolution + 1);
			dims.z = bme.database.size();

			brdfdata.tex = gli::texture3D(1, gli::format::FORMAT_R32_SFLOAT, dims);
			brdfdata.in_min = bme.database.begin()->first;
			brdfdata.in_max = (--bme.database.end())->first;

			std::vector<std::future<void>> futures;

			auto it = bme.database.begin();
			for (int i = 0; i < dims.z; ++i, ++it) {
				futures.push_back(ctx->scheduler().schedule_now([&, i=i, it=it](optional<task_scheduler*> sched) {
					float *data = reinterpret_cast<float*>(brdfdata.tex.data()) + i * (dims.x * dims.y);
					exitant_db &db = it->second;

					for (float out_theta = BRDF::theta_min; out_theta <= BRDF::theta_max; out_theta += resolution) {
						for (float out_phi = BRDF::phi_min; out_phi <= BRDF::phi_max; out_phi += resolution, ++data) {
							glm::vec3 w = BxDF::omega(out_theta, out_phi);

							using closest_vector_type = std::vector<std::pair<float, bme_brdf_descriptor_entry*>>;
							closest_vector_type closest;
							for (auto &entry : db) {
								float d = glm::dot(entry.v, w);
								auto pair = std::make_pair(d, &entry);
								if (closest.size() > 10 && closest[closest.size() - 1].first > d) continue;
								closest.insert(std::lower_bound(closest.begin(), closest.end(), pair, [](const closest_vector_type::value_type &lhs, const closest_vector_type::value_type &rhs) {
									return lhs.first > rhs.first;
								}), pair);
								if (closest.size() > 10) closest.pop_back();
							}

							std::pair<float, bme_brdf_descriptor_entry*> *pp = nullptr, *pm = nullptr, *mp = nullptr, *mm = nullptr;
							for (auto &pair : closest) {
								if (pair.second->v.x > w.x && pair.second->v.y > w.y && !pp) pp = &pair;
								if (pair.second->v.x <=w.x && pair.second->v.y > w.y && !mp) mp = &pair;
								if (pair.second->v.x > w.x && pair.second->v.y <=w.y && !pm) pm = &pair;
								if (pair.second->v.x <=w.x && pair.second->v.y <=w.y && !mm) mm = &pair;
							}

							float weight = .0f;
							if (pp) { weight += pp->first; }
							if (mp) { weight += mp->first; }
							if (pm) { weight += pm->first; }
							if (mm) { weight += mm->first; }
							float brdf = (pp ? pp->second->brdf * pp->first : .0f) +
								(mp ? mp->second->brdf * mp->first : .0f) +
								(pm ? pm->second->brdf * pm->first : .0f) +
								(mm ? mm->second->brdf * mm->first : .0f);
							brdf /= weight;

							*data = brdf;
						}
					}
				}));
			}

			for (auto &f : futures)
				f.wait();

			brdf_data data_to_save = brdfdata;
			ctx->cache().insert(cache_key, std::move(data_to_save));

			return std::move(brdfdata);
		}).then_on_main_thread([](optional<task_scheduler*> sched, brdf_data &&data) {
			return std::move(std::make_unique<BRDF>(data.in_min, data.in_max, std::move(data.tex)));
		});
	}
};

}
}
