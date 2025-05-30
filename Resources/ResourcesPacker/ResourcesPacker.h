#pragma once
#include <Core/Stream/Stream.h>
#include <Resources/StreamUniversalImpl/StreamUniversalImpl.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <Resources/ResourcesDefination/ResourcesDefination.h>
#include <Core/Serialization/Serialization.h>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <memory>

namespace cyanvne
{
    namespace resources
    {
        class ResourcesPacker
        {
        private:
            std::map<uint64_t, uint64_t> id_to_definition_index_;
            std::map<std::string, uint64_t> alias_to_id_;

            std::vector<ResourceDefinition> definitions_;

            uint64_t next_available_id_ = 0;

            ResourcesFileHeader header_;

            std::shared_ptr<core::stream::OutStreamInterface> out_stream_;
            bool finalized_ = false;

        public:
            explicit ResourcesPacker(std::shared_ptr<core::stream::OutStreamInterface> stream) : header_(),
	            out_stream_(std::move(stream)), finalized_(false)
            {
	            if (!out_stream_ || !out_stream_->is_open())
	            {
		            throw exception::resourcesexception::ResourcePackerIOException(
			            "Output stream is not valid or not open for ResourcesPacker.");
	            }
	            std::ptrdiff_t bytes_written = core::binaryserializer::serialize_object(*out_stream_, header_);
	            if (bytes_written < 0)
	            {
		            throw exception::resourcesexception::ResourcePackerIOException(
			            "Failed to write placeholder header to output stream.");
	            }
            }

            ~ResourcesPacker()
            {
                if (!finalized_)
                {
                    try
                    {
                        finalizePack();
                    }
                    catch (const std::exception&)
                    {
                        core::GlobalLogger::getCoreLogger()->error(
                            "Failed to write placeholder header to output stream.");
                    }
                    catch (...)
                    {
                        core::GlobalLogger::getCoreLogger()->error(
                            "Failed to write placeholder header to output stream.");
                    }
                }
            }

            ResourcesPacker(const ResourcesPacker&) = delete;
            ResourcesPacker& operator=(const ResourcesPacker&) = delete;
            ResourcesPacker(ResourcesPacker&&) = delete;
            ResourcesPacker& operator=(ResourcesPacker&&) = delete;

            uint64_t addResourceByStream(std::shared_ptr<core::stream::InStreamInterface> resource_stream,
                                         const ResourceType& type,
                                         const std::string& optional_alias = "");
            uint64_t addResourceByString(const std::string& content,
                                         const ResourceType& type,
                                         const std::string& optional_alias = "");
            uint64_t addResourceByData(const std::vector<uint8_t>& data,
                                       const ResourceType& type,
                                       const std::string& optional_alias = "");
            void finalizePack();
        };
    }
}