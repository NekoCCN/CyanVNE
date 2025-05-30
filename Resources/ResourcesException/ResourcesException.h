#pragma once
#include <Core/CoreException/CoreException.h>
#include <stdexcept>

namespace cyanvne
{
    namespace exception
    {
        namespace resourcesexception
        {
            class PathNotFoundException : public CyanVNEIOException
            {
            public:
                PathNotFoundException(const std::string& message) : CyanVNEIOException(message)
                {  }
            };

            class ResourcePackerBeenFinalizedException : public exception::IllegalStateException
            {
            public:
                ResourcePackerBeenFinalizedException(const std::string& message) : IllegalStateException(message)
                {  }
            };

            class ResourcePackerIOException : public CyanVNEIOException
            {
            public:
                ResourcePackerIOException(const std::string& message) : CyanVNEIOException(message)
                {  }
            };
        }
    }
}