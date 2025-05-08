#pragma once
#include <exception>
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