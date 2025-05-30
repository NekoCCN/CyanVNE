#pragma once
#include <stdexcept>

namespace cyanvne
{
	namespace exception
	{
		class CyanVNELogicException : public std::logic_error
		{
		public:
			CyanVNELogicException(const std::string& message) : std::logic_error(message)
			{  }
		};
        class CyanVNEIOException : public std::runtime_error
        {
        public:
            CyanVNEIOException(const std::string& message) : std::runtime_error(message)
            {  }
        };
        class CyanVNERuntimeException : public std::runtime_error
        {
        public:
            CyanVNERuntimeException(const std::string& message) : std::runtime_error(message)
            {  }
        };
        class IllegalArgumentException : public CyanVNELogicException
        {
        public:
            IllegalArgumentException(const std::string& message) : CyanVNELogicException(message)
            {  }
        };
        class IllegalStateException : public CyanVNELogicException
        {
        public:
            IllegalStateException(const std::string& message) : CyanVNELogicException(message)
            {  }
        };
        class NullPointerException : public CyanVNELogicException
        {
        public:
            NullPointerException(const std::string& message) : CyanVNELogicException(message)
            {  }
        };
        class MemoryAllocException : public CyanVNERuntimeException
        {
        public:
            MemoryAllocException(const std::string& message) : CyanVNERuntimeException(message)
            {  }
        };

        namespace coreexception
        {
            class LogSystemNotInitException : public CyanVNERuntimeException
            {
            public:
                LogSystemNotInitException(const std::string& message) : CyanVNERuntimeException(message)
                {  }
            };
        }
	}
}