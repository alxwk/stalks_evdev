#include "scssdk_input.h"
#include "scssdk_telemetry.h"

#ifdef __linux__
void __attribute__ ((destructor)) unload(void)
{
    scs_telemetry_shutdown();
    scs_input_shutdown();
}
#endif
