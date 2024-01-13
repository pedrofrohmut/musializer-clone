#ifndef LOGGER_H_
#define LOGGER_H_

void log_info(const char *format, ...);
void log_warn(const char *format, ...);
void log_error(const char *format, ...);
void log_debug(const char *format, ...);

#endif  // LOGGER_H_
