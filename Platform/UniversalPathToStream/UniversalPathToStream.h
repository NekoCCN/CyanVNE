#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <bx/platform.h>
#include "Core/PathToStream/PathToStream.h"
#include "Platform/StreamUniversalImpl/StreamUniversalImpl.h"

namespace cyanvne
{
    namespace platform
    {
        class UniversalPathToStream : public core::IPathToStream
        {
        public:
            UniversalPathToStream()
            {
                #if BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX || BX_PLATFORM_OSX
                    const char* path = SDL_GetBasePath();
                    if (path)
                    {
                        base_path_ = path;
                        SDL_free((void*)path);
                    }
                #endif
            }

            std::shared_ptr<core::stream::InStreamInterface> getInStream(const std::string& path) override
            {
                std::string full_path = getFullPath(path);
                return resources::InStreamUniversalImpl::createFromBinaryFile(full_path);
            }

            std::shared_ptr<core::stream::OutStreamInterface> getOutStream(const std::string& path) override
            {
                std::string full_path = getFullPath(path);
                return resources::OutStreamUniversalImpl::createFromBinaryFile(full_path);
            }

        private:
            std::string getFullPath(const std::string& path) const
            {
                #if BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX || BX_PLATFORM_OSX
                    if (isAbsolutePath(path))
                    {
                        return path;
                    }
                    else
                    {
                        return base_path_ + path;
                    }
                #else
                    return path;
                #endif
            }

            #if BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX || BX_PLATFORM_OSX
            bool isAbsolutePath(const std::string& path) const
            {
                if (path.empty()) return false;

                #if BX_PLATFORM_WINDOWS
                    if (path.length() > 2 && isalpha(path[0]) && path[1] == ':') {
                        if (path[2] == '\\' || path[2] == '/') return true;
                    }
                #else
                    if (path[0] == '/') return true;
                #endif

                return false;
            }

            std::string base_path_;
            #endif
        };
    }
}
