import logging

class log:

    loggerName = "lite3d_logger"

    @staticmethod
    def debug(message):
        logging.getLogger(log.loggerName).debug(str(message))

    @staticmethod
    def info(message):
        logging.getLogger(log.loggerName).info(str(message))

    @staticmethod
    def warn(message):
        logging.getLogger(log.loggerName).warning(str(message))

    @staticmethod
    def error(message):
        logging.getLogger(log.loggerName).error(str(message))
    
    @staticmethod
    def init():
        logHandler = logging.StreamHandler()
        logHandler.setLevel(logging.DEBUG)
        logFormatter = logging.Formatter("%(asctime)s:%(levelname)s:%(message)s")
        logHandler.setFormatter(logFormatter)

        logger = logging.getLogger(log.loggerName)
        logger.setLevel(logging.DEBUG)
        logger.addHandler(logHandler)

