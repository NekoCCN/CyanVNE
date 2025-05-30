#pragma once
#include <cstdint>
#include <Core/Stream/Stream.h>
#include <string>
#include <memory>

namespace cyanvne
{
	namespace core
	{
		class IPathToStream
		{
		protected:
            IPathToStream() = default;
		public:
			IPathToStream(const IPathToStream&) = delete;
            IPathToStream& operator=(const IPathToStream&) = delete;
            IPathToStream(IPathToStream&&) = delete;
            IPathToStream& operator=(IPathToStream&&) = delete;

			virtual std::shared_ptr<stream::InStreamInterface> getInStream(const std::string& path) = 0;
			virtual std::shared_ptr<stream::OutStreamInterface> getOutStream(const std::string& path) = 0;

            virtual ~IPathToStream() = default;
		};
	}
}