#include <string>
#include <cstring>
#include <scssdk_value.h>
#include <scssdk_telemetry_event.h>

#include <eurotrucks2/scssdk_eut2.h>
#include <amtrucks/scssdk_ats.h>

#include <scssdk_telemetry.h>
#include <eurotrucks2/scssdk_telemetry_eut2.h>
#include <amtrucks/scssdk_telemetry_ats.h>

#include <scssdk_input.h>

#include "globals.h"

using namespace std;

namespace {

SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void *const event_info, const scs_context_t)
{
    const struct scs_telemetry_configuration_t *const info = static_cast<const scs_telemetry_configuration_t *>(event_info);

    for (auto const* p = info->attributes; p->name; ++p) {
        if (0 == strcmp(p->name, SCS_TELEMETRY_CONFIG_ATTRIBUTE_shifter_type)) {
            if (0 == strcmp(p->value.value_string.value, SCS_SHIFTER_TYPE_arcade)) {
                active_shifter = SM_ARCADE;
            } else if (0 == strcmp(p->value.value_string.value, SCS_SHIFTER_TYPE_automatic)) {
                active_shifter = SM_AUTOMATIC;
            } else if (0 == strcmp(p->value.value_string.value, SCS_SHIFTER_TYPE_manual)) {
                active_shifter = SM_MANUAL;
            } else if (0 == strcmp(p->value.value_string.value, SCS_SHIFTER_TYPE_hshifter)) {
                active_shifter = SM_HSHIFTER;
            } else {
                game_log(SCS_LOG_TYPE_error, ("unknown shifter type [" + string(p->value.value_string.value) + "]").c_str());
                active_shifter = SM_UNKNOWN;
            }
            break;
        }
    }
    active_shifter_manual = (active_shifter == SM_MANUAL);
}

SCSAPI_VOID telemetry_channel_parking(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t)
{
    active_brake = value->value_bool.value;
    //    game_log(SCS_LOG_TYPE_message, ("brake: " + to_string(active_brake)).c_str());
}

SCSAPI_VOID telemetry_channel_cc(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t)
{
    active_cc = value->value_float.value;
    // game_log(SCS_LOG_TYPE_message, ("cc="+to_string(active_cc*3.6)).c_str());
}

}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t *const params)
{
    // We currently support only one version.

    if (version != SCS_TELEMETRY_VERSION_1_01) {
        return SCS_RESULT_unsupported;
    }

    const scs_telemetry_init_params_v101_t *const version_params = static_cast<const scs_telemetry_init_params_v101_t *>(params);

    if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0) {
        // Below the minimum version there might be some missing features (only minor change) or
        // incompatible values (major change).
        const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_1_00;
        if (version_params->common.game_version < MINIMAL_VERSION) {
            //            log_line("WARNING: Too old version of the game, some features might behave incorrectly");
        }
        // Future versions are fine as long the major version is not changed.
        const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT;
        if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
            //            log_line("WARNING: Too new major version of the game, some features might behave incorrectly");
        }
    } else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {
        // Below the minimum version there might be some missing features (only minor change) or
        // incompatible values (major change).
        const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_1_00;
        if (version_params->common.game_version < MINIMAL_VERSION) {
            //            log_line("WARNING: Too old version of the game, some features might behave incorrectly");
        }

        // Future versions are fine as long the major version is not changed.
        const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_CURRENT;
        if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
            //            log_line("WARNING: Too new major version of the game, some features might behave incorrectly");
        }
    } else {
        //    log_line("WARNING: Unsupported game, some features or values might behave incorrectly");
    }

    // Remember the function we will use for logging.
    game_log = version_params->common.log;
    game_log(SCS_LOG_TYPE_message, "Initializing simtoggle plugin");

    // Register for the configuration info. As this example only prints the retrieved
    // data, it can operate even if that fails.
    bool events_registered =
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_parking_brake,
                                              SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none,
                                              telemetry_channel_parking, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control,
                                              SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none,
                                              telemetry_channel_cc, NULL) == SCS_RESULT_ok);

    if (! events_registered) {
        // Registrations created by unsuccessfull initialization are
        // cleared automatically so we can simply exit.
        game_log(SCS_LOG_TYPE_error, "Unable to register event callbacks");
        return SCS_RESULT_generic_error;
    }

    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void)
{
    // Any cleanup needed. The registrations will be removed automatically
    // so there is no need to do that manually.
}
