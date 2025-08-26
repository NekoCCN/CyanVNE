
//
// Created by unirz on 2025/7/10.
//

#ifndef RUNTIMEEXCEPTION_H
#define RUNTIMEEXCEPTION_H
#include "Core/CoreException/CoreException.h"

namespace cyanvne::exception::runtimeexception
{
    class CyanvneCommandExecuteException : public CyanVNERuntimeException
    {
    public:
        CyanvneCommandExecuteException(const std::string& command, const std::string& message)
            : CyanVNERuntimeException(command + "Command execution failed: " + message)
        {  }
    };
}



#endif //RUNTIMEEXCEPTION_H
