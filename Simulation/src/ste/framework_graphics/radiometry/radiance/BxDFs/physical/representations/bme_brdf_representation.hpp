// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "common_brdf_representation.hpp"
#include "pBRDF.hpp"

#include "StEngineControl.hpp"
#include "lru_cache.hpp"

#include "Log.hpp"
#include "AttributedString.hpp"

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

			entry.v = BxDF<float>::omega(glm::radians(entry.theta), glm::radians(entry.phi));

			return istream;
		}

	};

private:
	using exitant_db = std::unordered_map<int, std::unordered_map<int, std::vector<bme_brdf_descriptor_entry>>>;
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

	auto load_bme_brdf_task(const boost::filesystem::path &bme_data) const {
		return [=]() -> exitant_db_descriptor {
			exitant_db db;
			float in_theta = -1;
			int in_phi = -1;

			std::ifstream fs(bme_data.string(), std::ios::in);
			if (!fs.good()) {
				using namespace Text::Attributes;
				ste_log_error() << Text::AttributedString("Error while reading BME database \"") + i(bme_data.string()) + "\": " + std::strerror(errno) << std::endl;
				assert(false);

				return exitant_db_descriptor();
			}

			std::string l;
			while (std::getline(fs, l)) {
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
					while (entry.phi > pBRDF::phi_max) entry.phi -= 360.f;
					while (entry.phi < pBRDF::phi_min) entry.phi += 360.f;
					append_entry(std::move(entry), db);
				}
			}

			fs.close();

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
		std::vector<std::task_future<exitant_db_descriptor>> futures;
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

protected:
	static void create_layer(int i, const database_type::iterator &it, common_brdf_representation &brdfdata, const glm::ivec3 &dims) {
		float *data = reinterpret_cast<float*>(brdfdata.get_data()->data()) + dims.x * dims.y * i;
		exitant_db &db = it->second;

		for (int j = 0; j < dims.y; ++j) {
			float theta = glm::mix<float>(pBRDF::theta_min, pBRDF::theta_max, static_cast<float>(j) / static_cast<float>(dims.y - 1));

			for (int k = 0; k < dims.x; ++k) {
				float phi = glm::mix<float>(pBRDF::phi_min, pBRDF::phi_max, static_cast<float>(k) / static_cast<float>(dims.x - 1));

				glm::vec3 w = BxDF<float>::omega(glm::radians(theta), glm::radians(phi));

				std::vector<bme_brdf_descriptor_entry*> entries;
				int theta_bucket = static_cast<int>(theta / bucket_size);
				int phi_bucket = static_cast<int>(phi / bucket_size);

				constexpr int samples = 15;
				constexpr auto p_range = pBRDF::phi_max - pBRDF::phi_min + 1;
				int bias = 1;
				while (entries.size() < samples) {
					for (int x = -bias; x <= bias; ++x) {
						int tm = glm::clamp(theta_bucket + x, pBRDF::theta_min, pBRDF::theta_max);
						int pm = phi_bucket + bias <= pBRDF::phi_max ? phi_bucket + bias : phi_bucket + bias - p_range;
						int pn = phi_bucket - bias >= pBRDF::phi_min ? phi_bucket - bias : phi_bucket - bias + p_range;
						for (auto &e : db[tm][pm])
							entries.push_back(&e);
						if (bias)
							for (auto &e : db[tm][pn])
								entries.push_back(&e);
					}
					for (int x = -bias + 1; x < bias; ++x) {
						int tm = glm::clamp(theta_bucket + bias, pBRDF::theta_min, pBRDF::theta_max);
						int tn = glm::clamp(theta_bucket - bias, pBRDF::theta_min, pBRDF::theta_max);
						int pm = phi_bucket + x;
						if (pm > pBRDF::phi_max) pm -= p_range;
						if (pm < pBRDF::phi_min) pm += p_range;
						for (auto &e : db[tm][pm])
							entries.push_back(&e);
						if (bias)
							for (auto &e : db[tn][pm])
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

				data[j * dims.x + k] = closest[0].second->brdf;
			}
		}
	}

public:
	static auto BRDF_from_bme_representation_task(const StEngineControl &context, const boost::filesystem::path &bme_data_dir) {
		std::string cache_key = std::string("bme_brdf_representation_") + bme_data_dir.string();
		return context.scheduler().schedule_now([=, ctx = &context]() {
			common_brdf_representation brdfdata;
			try {
				auto cache_get_task = ctx->cache().get<common_brdf_representation>(cache_key);
				optional<common_brdf_representation> opt = cache_get_task();
				if (opt) {
					using namespace Text::Attributes;
					ste_log() << b("bme_brdf_representation") + " - pBRDF loaded from cache: " + i(bme_data_dir.string()) << std::endl;
					return std::move(opt.get());
				}
			}
			catch (const std::exception &ex) {}

			{
				using namespace Text::Attributes;
				ste_log() << b("bme_brdf_representation") + " - Loading pBRDF: " + i(bme_data_dir.string()) << std::endl;
			}

			bme_brdf_representation bme;
			bme.load(*ctx, bme_data_dir);

			glm::ivec3 dims;
			dims.x = glm::ceil((pBRDF::phi_max - pBRDF::phi_min) / resolution + 1);
			dims.y = glm::ceil((pBRDF::theta_max - pBRDF::theta_min) / resolution + 1);
			dims.z = bme.database.size();

			brdfdata.get_data() = std::make_unique<gli::texture3d>(gli::format::FORMAT_R32_SFLOAT_PACK32, dims, 1);
			brdfdata.set_min_incident(bme.database.begin()->first);
			brdfdata.set_max_incident((--bme.database.end())->first);

			std::vector<std::task_future<void>> futures;

			auto it = bme.database.begin();
			for (int i = 0; i < dims.z; ++i, ++it) {
				futures.push_back(ctx->scheduler().schedule_now([&, i=i, it=it]() {
					create_layer(i, it, brdfdata, dims);
				}));
			}

			for (auto &f : futures)
				f.wait();

			ctx->cache().insert(cache_key, brdfdata);
			return std::move(brdfdata);
		}).then_on_main_thread([=](common_brdf_representation &&brdfdata) {
			auto ptr = std::make_unique<pBRDF>(std::move(brdfdata));
			return ptr;
		});
	}
};

}
}
