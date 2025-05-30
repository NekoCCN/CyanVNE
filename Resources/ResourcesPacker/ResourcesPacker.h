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
        class IResourcesPacker
        {
        protected:
            IResourcesPacker() = default;
        public:
            IResourcesPacker(const IResourcesPacker&) = delete;
            IResourcesPacker& operator=(const IResourcesPacker&) = delete;
            IResourcesPacker(IResourcesPacker&&) = delete;
            IResourcesPacker& operator=(IResourcesPacker&&) = delete;

            virtual void finalizePack() = 0;

            virtual ~IResourcesPacker() = default;
        };

        class ResourcesPacker : public IResourcesPacker
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
            explicit ResourcesPacker(const std::shared_ptr<core::stream::OutStreamInterface>& stream) : header_(),
	            out_stream_(stream), finalized_(false)
            {
	            if (!out_stream_ || !out_stream_->is_open())
	            {
		            throw exception::resourcesexception::ResourcePackerIOException(
			            "Output stream is not valid or not open for ResourcesPacker.");
	            }
	            std::ptrdiff_t bytes_written = serialize_object(*out_stream_, header_);
	            if (bytes_written < 0)
	            {
		            throw exception::resourcesexception::ResourcePackerIOException(
			            "Failed to write placeholder header to output stream.");
	            }
            }

            ~ResourcesPacker() override
            {
                if (!finalized_)
                {
                    try
                    {
	                    ResourcesPacker::finalizePack();
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

            uint64_t addResourceByStream(const std::shared_ptr<core::stream::InStreamInterface>& resource_stream,
                                         const ResourceType& type,
                                         const std::string& optional_alias = "");
            uint64_t addResourceByString(const std::string& content,
                                         const ResourceType& type,
                                         const std::string& optional_alias = "");
            uint64_t addResourceByData(const std::vector<uint8_t>& data,
                                       const ResourceType& type,
                                       const std::string& optional_alias = "");
            void finalizePack() override;
        };
    }
}