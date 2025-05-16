#pragma once
#include <memory>
#include <Core/Stream/Stream.h>

namespace cyanvne
{
	namespace parser
	{
		class ParserYamlInterface
		{
		protected:
			ParserYamlInterface() = default;
		public:
			ParserYamlInterface(const ParserYamlInterface&) = delete;
			ParserYamlInterface(ParserYamlInterface&&) = delete;
			ParserYamlInterface& operator=(const ParserYamlInterface&) = delete;
			ParserYamlInterface& operator=(ParserYamlInterface&&) = delete;

			virtual void parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_config_path) = 0;

			virtual ~ParserYamlInterface() = default;
		};
	}
}