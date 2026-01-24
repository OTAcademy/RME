//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "lua_api_noise.h"
#include "../fast_noise_lite.h"

#include <unordered_map>
#include <mutex>

namespace LuaAPI {

	// Thread-local noise generator cache for better performance
	// Each seed gets its own generator instance
	class NoiseGeneratorCache {
	public:
		FastNoiseLite& getGenerator(int seed) {
			std::lock_guard<std::mutex> lock(mutex_);
			auto it = generators_.find(seed);
			if (it == generators_.end()) {
				auto& gen = generators_[seed];
				gen.SetSeed(seed);
				return gen;
			}
			return it->second;
		}

		void clear() {
			std::lock_guard<std::mutex> lock(mutex_);
			generators_.clear();
		}

	private:
		std::unordered_map<int, FastNoiseLite> generators_;
		std::mutex mutex_;
	};

	static NoiseGeneratorCache g_noiseCache;

	// Helper to create configured noise generator
	static FastNoiseLite createNoiseGenerator(int seed, FastNoiseLite::NoiseType type, float frequency = 0.01f) {
		FastNoiseLite noise;
		noise.SetSeed(seed);
		noise.SetNoiseType(type);
		noise.SetFrequency(frequency);
		return noise;
	}

	void registerNoise(sol::state& lua) {
		sol::table noiseTable = lua.create_table();

		// ========================================
		// PERLIN NOISE
		// ========================================

		// noise.perlin(x, y, seed, frequency) -> number [-1, 1]
		// Simple 2D perlin noise
		noiseTable.set_function("perlin", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_Perlin, freq);
			return noise.GetNoise(x, y);
		});

		// noise.perlin3d(x, y, z, seed, frequency) -> number [-1, 1]
		// 3D perlin noise
		noiseTable.set_function("perlin3d", [](float x, float y, float z, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_Perlin, freq);
			return noise.GetNoise(x, y, z);
		});

		// ========================================
		// SIMPLEX / OPENSIMPLEX2 NOISE
		// ========================================

		// noise.simplex(x, y, seed, frequency) -> number [-1, 1]
		// OpenSimplex2 noise - better quality than perlin
		noiseTable.set_function("simplex", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_OpenSimplex2, freq);
			return noise.GetNoise(x, y);
		});

		// noise.simplex3d(x, y, z, seed, frequency) -> number [-1, 1]
		noiseTable.set_function("simplex3d", [](float x, float y, float z, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_OpenSimplex2, freq);
			return noise.GetNoise(x, y, z);
		});

		// noise.simplexSmooth(x, y, seed, frequency) -> number [-1, 1]
		// OpenSimplex2S - smoother variant
		noiseTable.set_function("simplexSmooth", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_OpenSimplex2S, freq);
			return noise.GetNoise(x, y);
		});

		// ========================================
		// CELLULAR / VORONOI NOISE
		// ========================================

		// noise.cellular(x, y, seed, frequency, distanceFunc, returnType) -> number
		// Cellular/Voronoi noise for cave-like structures, organic patterns
		noiseTable.set_function("cellular", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency, sol::optional<std::string> distanceFunc, sol::optional<std::string> returnType) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);
			std::string distFn = distanceFunc.value_or("euclidean");
			std::string retType = returnType.value_or("distance");

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_Cellular, freq);

			// Set distance function
			if (distFn == "euclidean" || distFn == "euclideanSq") {
				noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
			} else if (distFn == "manhattan") {
				noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Manhattan);
			} else if (distFn == "hybrid") {
				noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
			}

			// Set return type
			if (retType == "cellValue") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
			} else if (retType == "distance") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
			} else if (retType == "distance2") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
			} else if (retType == "distance2Add") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add);
			} else if (retType == "distance2Sub") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Sub);
			} else if (retType == "distance2Mul") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Mul);
			} else if (retType == "distance2Div") {
				noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
			}

			return noise.GetNoise(x, y);
		});

		// noise.cellular3d(x, y, z, seed, frequency) -> number
		noiseTable.set_function("cellular3d", [](float x, float y, float z, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_Cellular, freq);
			return noise.GetNoise(x, y, z);
		});

		// ========================================
		// VALUE NOISE
		// ========================================

		// noise.value(x, y, seed, frequency) -> number [-1, 1]
		// Value noise - blocky, good for heightmaps
		noiseTable.set_function("value", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_Value, freq);
			return noise.GetNoise(x, y);
		});

		// noise.valueCubic(x, y, seed, frequency) -> number [-1, 1]
		// Cubic interpolated value noise - smoother than value
		noiseTable.set_function("valueCubic", [](float x, float y, sol::optional<int> seed, sol::optional<float> frequency) -> float {
			int s = seed.value_or(1337);
			float freq = frequency.value_or(0.01f);

			FastNoiseLite noise = createNoiseGenerator(s, FastNoiseLite::NoiseType_ValueCubic, freq);
			return noise.GetNoise(x, y);
		});

		// ========================================
		// FBM (Fractional Brownian Motion)
		// ========================================

		// noise.fbm(x, y, seed, options) -> number
		// Fractal Brownian Motion - layered noise for natural terrain
		// options: { frequency, octaves, lacunarity, gain, noiseType }
		noiseTable.set_function("fbm", [](float x, float y, sol::optional<int> seed, sol::optional<sol::table> options) -> float {
			int s = seed.value_or(1337);

			FastNoiseLite noise;
			noise.SetSeed(s);
			noise.SetFractalType(FastNoiseLite::FractalType_FBm);

			if (options) {
				sol::table opts = *options;

				// Frequency
				float freq = opts.get_or(std::string("frequency"), 0.01f);
				noise.SetFrequency(freq);

				// Octaves (layers of noise)
				int octaves = opts.get_or(std::string("octaves"), 4);
				noise.SetFractalOctaves(octaves);

				// Lacunarity (frequency multiplier per octave)
				float lacunarity = opts.get_or(std::string("lacunarity"), 2.0f);
				noise.SetFractalLacunarity(lacunarity);

				// Gain (amplitude multiplier per octave)
				float gain = opts.get_or(std::string("gain"), 0.5f);
				noise.SetFractalGain(gain);

				// Noise type for fractal
				std::string noiseType = opts.get_or<std::string>(std::string("noiseType"), "simplex");
				if (noiseType == "perlin") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
				} else if (noiseType == "simplex" || noiseType == "opensimplex") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
				} else if (noiseType == "value") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
				} else if (noiseType == "cellular") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
				}
			} else {
				noise.SetFrequency(0.01f);
				noise.SetFractalOctaves(4);
				noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
			}

			return noise.GetNoise(x, y);
		});

		// noise.fbm3d(x, y, z, seed, options) -> number
		noiseTable.set_function("fbm3d", [](float x, float y, float z, sol::optional<int> seed, sol::optional<sol::table> options) -> float {
			int s = seed.value_or(1337);

			FastNoiseLite noise;
			noise.SetSeed(s);
			noise.SetFractalType(FastNoiseLite::FractalType_FBm);

			if (options) {
				sol::table opts = *options;
				noise.SetFrequency(opts.get_or(std::string("frequency"), 0.01f));
				noise.SetFractalOctaves(opts.get_or(std::string("octaves"), 4));
				noise.SetFractalLacunarity(opts.get_or(std::string("lacunarity"), 2.0f));
				noise.SetFractalGain(opts.get_or(std::string("gain"), 0.5f));

				std::string noiseType = opts.get_or<std::string>(std::string("noiseType"), "simplex");
				if (noiseType == "perlin") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
				} else {
					noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
				}
			} else {
				noise.SetFrequency(0.01f);
				noise.SetFractalOctaves(4);
				noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
			}

			return noise.GetNoise(x, y, z);
		});

		// ========================================
		// RIDGED NOISE
		// ========================================

		// noise.ridged(x, y, seed, options) -> number
		// Ridged fractal noise - good for mountains, veins
		noiseTable.set_function("ridged", [](float x, float y, sol::optional<int> seed, sol::optional<sol::table> options) -> float {
			int s = seed.value_or(1337);

			FastNoiseLite noise;
			noise.SetSeed(s);
			noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
			noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

			if (options) {
				sol::table opts = *options;
				noise.SetFrequency(opts.get_or(std::string("frequency"), 0.01f));
				noise.SetFractalOctaves(opts.get_or(std::string("octaves"), 4));
				noise.SetFractalLacunarity(opts.get_or(std::string("lacunarity"), 2.0f));
				noise.SetFractalGain(opts.get_or(std::string("gain"), 0.5f));
			} else {
				noise.SetFrequency(0.01f);
				noise.SetFractalOctaves(4);
			}

			return noise.GetNoise(x, y);
		});

		// ========================================
		// DOMAIN WARP
		// ========================================

		// noise.warp(x, y, seed, options) -> x, y (warped coordinates)
		// Domain warping - distorts input coordinates for organic effects
		noiseTable.set_function("warp", [](float x, float y, sol::optional<int> seed, sol::optional<sol::table> options, sol::this_state s) -> sol::object {
			int sd = seed.value_or(1337);

			FastNoiseLite noise;
			noise.SetSeed(sd);
			noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);

			float amplitude = 30.0f;
			float frequency = 0.01f;

			if (options) {
				sol::table opts = *options;
				amplitude = opts.get_or(std::string("amplitude"), 30.0f);
				frequency = opts.get_or(std::string("frequency"), 0.01f);

				std::string warpType = opts.get_or<std::string>(std::string("type"), "simplex");
				if (warpType == "simplex" || warpType == "opensimplex") {
					noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
				} else if (warpType == "simplexReduced") {
					noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2Reduced);
				} else if (warpType == "basic") {
					noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
				}
			}

			noise.SetDomainWarpAmp(amplitude);
			noise.SetFrequency(frequency);

			noise.DomainWarp(x, y);

			sol::state_view lua(s);
			sol::table result = lua.create_table();
			result["x"] = x;
			result["y"] = y;
			return result;
		});

		// ========================================
		// UTILITY FUNCTIONS
		// ========================================

		// noise.normalize(value, min, max) -> number [0, 1]
		// Normalize noise value from [-1,1] to [min, max]
		noiseTable.set_function("normalize", [](float value, sol::optional<float> minVal, sol::optional<float> maxVal) -> float {
			float min = minVal.value_or(0.0f);
			float max = maxVal.value_or(1.0f);
			// value is in [-1, 1], normalize to [0, 1] first, then scale
			float normalized = (value + 1.0f) * 0.5f;
			return min + normalized * (max - min);
		});

		// noise.threshold(value, threshold) -> boolean
		// Returns true if value is above threshold
		noiseTable.set_function("threshold", [](float value, float threshold) -> bool {
			return value >= threshold;
		});

		// noise.map(value, inMin, inMax, outMin, outMax) -> number
		// Map value from one range to another
		noiseTable.set_function("map", [](float value, float inMin, float inMax, float outMin, float outMax) -> float {
			float t = (value - inMin) / (inMax - inMin);
			return outMin + t * (outMax - outMin);
		});

		// noise.clamp(value, min, max) -> number
		noiseTable.set_function("clamp", [](float value, float min, float max) -> float {
			if (value < min) {
				return min;
			}
			if (value > max) {
				return max;
			}
			return value;
		});

		// noise.lerp(a, b, t) -> number
		// Linear interpolation
		noiseTable.set_function("lerp", [](float a, float b, float t) -> float {
			return a + t * (b - a);
		});

		// noise.smoothstep(edge0, edge1, x) -> number
		// Smooth interpolation
		noiseTable.set_function("smoothstep", [](float edge0, float edge1, float x) -> float {
			float t = (x - edge0) / (edge1 - edge0);
			if (t < 0.0f) {
				t = 0.0f;
			}
			if (t > 1.0f) {
				t = 1.0f;
			}
			return t * t * (3.0f - 2.0f * t);
		});

		// noise.clearCache() - clear noise generator cache
		noiseTable.set_function("clearCache", []() {
			g_noiseCache.clear();
		});

		// ========================================
		// BATCH GENERATION (for performance)
		// ========================================

		// noise.generateGrid(x1, y1, x2, y2, options) -> table of values
		// Generate noise values for a grid area (faster than individual calls)
		noiseTable.set_function("generateGrid", [](int x1, int y1, int x2, int y2, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);
			sol::table result = lua.create_table();

			FastNoiseLite noise;

			if (options) {
				sol::table opts = *options;
				noise.SetSeed(opts.get_or(std::string("seed"), 1337));
				noise.SetFrequency(opts.get_or(std::string("frequency"), 0.01f));

				std::string noiseType = opts.get_or<std::string>(std::string("noiseType"), "simplex");
				if (noiseType == "perlin") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
				} else if (noiseType == "simplex") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
				} else if (noiseType == "cellular") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
				} else if (noiseType == "value") {
					noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
				}

				// Fractal settings
				std::string fractal = opts.get_or<std::string>(std::string("fractal"), "none");
				if (fractal == "fbm") {
					noise.SetFractalType(FastNoiseLite::FractalType_FBm);
					noise.SetFractalOctaves(opts.get_or(std::string("octaves"), 4));
					noise.SetFractalLacunarity(opts.get_or(std::string("lacunarity"), 2.0f));
					noise.SetFractalGain(opts.get_or(std::string("gain"), 0.5f));
				} else if (fractal == "ridged") {
					noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
					noise.SetFractalOctaves(opts.get_or(std::string("octaves"), 4));
				}
			} else {
				noise.SetSeed(1337);
				noise.SetFrequency(0.01f);
				noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
			}

			// Generate values
			for (int y = y1; y <= y2; ++y) {
				sol::table row = lua.create_table();
				for (int x = x1; x <= x2; ++x) {
					row[x - x1 + 1] = noise.GetNoise((float)x, (float)y);
				}
				result[y - y1 + 1] = row;
			}

			return result;
		});

		lua["noise"] = noiseTable;
	}

} // namespace LuaAPI
