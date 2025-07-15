#ifndef ERROREVENT_H
#define ERROREVENT_H
#include <string>

namespace cyanvne::platform
{
    namespace events
    {
        struct ErrorEvent
        {
            std::string error_message;
            std::string error_source;
            bool is_fatal = false;

            ErrorEvent(std::string message, std::string source, bool fatal = false)
                : error_message(std::move(message)), error_source(std::move(source)), is_fatal(fatal)
            {  }
        };
    }
}


#endif //ERROREVENT_H
