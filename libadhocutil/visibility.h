#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#ifndef DLL_PRIVATE
#define DLL_PRIVATE __attribute__ ((visibility ("hidden")))
#endif

