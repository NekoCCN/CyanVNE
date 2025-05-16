#pragma once
#include <Core/CoreException/CoreException.h>

namespace cyanvne
{
	namespace exception
	{
		namespace parserexception
		{
			class ParserException : public exception::CyanVNERuntimeException
			{
			private:
				std::string format_error_message;
			public:
				ParserException(const std::string& message, const std::string& format_error_message)
					: CyanVNERuntimeException(message), format_error_message(format_error_message)
				{  }

				virtual const char* getFormatErrorMessage() const noexcept
				{
					return format_error_message.c_str();
				}
			};
		}
	}
}