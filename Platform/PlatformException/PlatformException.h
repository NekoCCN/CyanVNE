#pragma once
#include <Core/CoreException/CoreException.h>
#include <stdexcept>

namespace cyanvne
{
    namespace exception
    {
        namespace platformexception
        {
            class CreateWindowContextException : public CyanVNERuntimeException
            {
            public:
                CreateWindowContextException(const std::string& message) : CyanVNERuntimeException(message)
                {  }
            };

            class InitWMSystemException : public CyanVNERuntimeException
            {
            public:
                InitWMSystemException(const std::string& message) : CyanVNERuntimeException(message)
                {  }
            };

            class InitEventCoreException : public CyanVNERuntimeException
            {
            public:
                InitEventCoreException(const std::string& message) : CyanVNERuntimeException(message)
                {  }
            };

            class MonitorNotFoundException : public CyanVNERuntimeException
            {
            public:
                MonitorNotFoundException() : CyanVNERuntimeException("Can not found any monitor")
                {  }
                MonitorNotFoundException(const std::string& message) : CyanVNERuntimeException(message)
                {  }
            };
        }
    }
}