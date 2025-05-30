#pragma once
#include <Resources/StreamUniversalImpl/StreamUniversalImpl.h>
#include <Core/PathToStream/PathToStream.h>

namespace cyanvne
{
	namespace resources
	{
        class UniversalPathToStream : public core::IPathToStream
        {
        public:
            std::shared_ptr<core::stream::InStreamInterface> getInStream(const std::string& path) override
            {
                return InStreamUniversalImpl::createFromBinaryFile(path);
            }
            std::shared_ptr<core::stream::OutStreamInterface> getOutStream(const std::string& path) override
            {
                return OutStreamUniversalImpl::createFromBinaryFile(path);
            }
        };
	}
}