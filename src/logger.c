#include <stdarg.h>
#include <stdio.h>

void _log(const char *text, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s", text);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void log_info(const char *format, ...)
{
    va_list args;
    _log("[INFO] ", format, args);
}

void log_warn(const char *format, ...)
{
    va_list args;
    _log("[WARN] ", format, args);
}

void log_error(const char *format, ...)
{
    va_list args;
    _log("[ERROR] ", format, args);
}

void log_debug(const char *format, ...)
{
    va_list args;
    _log("[DEBUG] ", format, args);
}
