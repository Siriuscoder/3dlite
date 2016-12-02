#ifndef NWE_LOGGABLE_H
#define NWE_LOGGABLE_H

#include <sstream>

#include "log/logger.h"

#define NW_LOG(_Level, _Message)                      \
    log(                                              \
        _Level,                                       \
        static_cast<std::ostringstream&>(             \
            std::ostringstream().flush() << _Message  \
        ).str().c_str(),                              \
        __FUNCTION__,                                 \
        __FILE__,                                     \
        __LINE__                                      \
    );

#define NW_INFO(_Message) \
    NW_LOG(nw::Logger::Info, _Message)

#define NW_ERROR(_Message) \
    NW_LOG(nw::Logger::Error, _Message)

#define NW_WARNING(_Message) \
    NW_LOG(nw::Logger::Warning, _Message)

#define NW_DEBUG(_Message) \
    NW_LOG(nw::Logger::Debug, _Message)

namespace nw
{
    class Loggable
    {
        private:
            Logger* m_pLogger;

        public:
            Loggable() :
                m_pLogger(NULL)
            {
            }

            Loggable(Logger* _pLogger) :
                m_pLogger(_pLogger)
            {
            }

            virtual ~Loggable()
            {
            }

            Logger* logger()
            {
                return m_pLogger;
            }

            Logger* logger() const
            {
                return m_pLogger;
            }

            void setLogger(Logger* _pLogger)
            {
                m_pLogger = _pLogger;
            }

        protected:
            void log(
                Logger::Level _level,
                const char* _msg,
                const char* _function,
                const char* _file,
                int _line) const
            {
                if (m_pLogger)
                {
                    m_pLogger->log(_level, _msg, _function, _file, _line);
                }
            }
    };
}

#endif
