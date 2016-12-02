#ifndef NWE_LOGGER_H
#define NWE_LOGGER_H

namespace nw
{
    class Logger
    {
        public:
            enum Level
            {
                Info,
                Error,
                Warning,
                Debug
            };

        public:
            virtual void log(
                Level _level,
                const char* _msg,
                const char* _function,
                const char* _file,
                int _line) = 0;
    };
}

#endif
