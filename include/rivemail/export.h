#ifndef RIVEMAIL_EXPORT_H
#define RIVEMAIL_EXPORT_H
#if defined(_WIN32) && defined(RIVEMAIL_SHARED)
#  if defined(RIVEMAIL_BUILD)
#    define RIVEMAIL_API __declspec(dllexport)
#  else
#    define RIVEMAIL_API __declspec(dllimport)
#  endif
#else
#  define RIVEMAIL_API
#endif
#endif
