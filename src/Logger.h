
#if defined(LOGGER)
    #define LOG(...) LOGGER.print(__VA_ARGS__)
    #define LOGLN(...) LOGGER.println(__VA_ARGS__)
    #define LOGFMT(...) LOGGER.printf(__VA_ARGS__)
    #define LOGWRITE(n) LOGGER.write(n)
    #define LOGWRITES(n, size) LOGGER.write(n, size)
#else
    #define LOG(...)
    #define LOGLN(...)
    #define LOGFMT(...)
    #define LOGWRITE(n)
    #define LOGWRITES(n, size)
#endif  // defined(LOGGER)
