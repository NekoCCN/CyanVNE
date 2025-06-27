#pragma once
#include <cstddef>

namespace cyanvne
{
    namespace resources
    {
        class ICachedResource
        {
        public:
            virtual ~ICachedResource() = default;
            virtual size_t getSizeInBytes() const = 0;
        };
    }
}
