#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <optional>
#include "Core/Serialization/Serialization.h"

namespace cyanvne
{
	namespace parser
	{
		namespace theme
		{
			struct AnimationData : public core::binaryserializer::BinarySerialiable
			{
				int32_t ms_per_frame = -1;
				std::vector<std::vector<uint32_t>> sprite_sheet;

				std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const override;
				std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override;
			};

			struct AreaScaleData : public core::binaryserializer::BinarySerialiable
			{
				double x = 0.0, y = 0.0, w = -1.0, h = -1.0;

				std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const override;
				std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override;
			};

			struct ThemeResource : public core::binaryserializer::BinarySerialiable
			{
				std::string key;
				std::optional<AnimationData> animation;
				std::optional<AreaScaleData> area_scale;

				std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const override;
				std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override;
			};

			struct ThemeConfig : public core::binaryserializer::BinarySerialiable
			{
				std::string name;
				bool enable_built_in_font = false;
				std::map<std::string, ThemeResource> resources;

				std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const override;
				std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override;
			};

			struct ThemeGeneratorConfig : public core::binaryserializer::BinarySerialiable
			{
				std::map<std::string, std::string> resources;

				std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const override;
				std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override;
			};
		}
	}
}
