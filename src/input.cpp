#include <thread>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <map>
#include <queue>
#include <array>
#include <tuple>
#include <functional>
#include <fcntl.h>
#include <libevdev/libevdev.h>
#include <scssdk_input.h>
#include <scssdk_input_event.h>
#include "globals.h"

using namespace std;

namespace {

struct libevdev *input_dev = nullptr;

const auto inputs = to_array<scs_input_device_input_t>(
    {{.name = "transemi",       .display_name = "Auto_Seq toggle",          .value_type = SCS_VALUE_TYPE_bool},
     {.name = "parkingbrake",   .display_name = "Parking brake toggle",     .value_type = SCS_VALUE_TYPE_bool},
     {.name = "gear0",          .display_name = "Neutral gear position",    .value_type = SCS_VALUE_TYPE_bool},
     {.name = "geardrive",      .display_name = "Drive gear position",      .value_type = SCS_VALUE_TYPE_bool},
     {.name = "gearreverse",    .display_name = "Reverse gear position",    .value_type = SCS_VALUE_TYPE_bool},
     {.name = "cruiectrl",      .display_name = "CC toggle",                .value_type = SCS_VALUE_TYPE_bool},
     {.name = "cruiectrlres",   .display_name = "CC resume toggle",         .value_type = SCS_VALUE_TYPE_bool},
     {.name = "cruiectrlinc",   .display_name = "CC increment toggle",      .value_type = SCS_VALUE_TYPE_bool},
     {.name = "cruiectrldec",   .display_name = "CC decrement toggle",      .value_type = SCS_VALUE_TYPE_bool}});

enum input_n_t : uint8_t {
    IN_AUTOSEQ  = 0, 
    IN_PBRAKE   = 1,
    IN_NEUTRAL  = 2,
    IN_DRIVE    = 3,
    IN_REVERSE  = 4,
    IN_CC       = 5,
    IN_CCRES    = 6,
    IN_CCINC    = 7,
    IN_CCDEC    = 8
};

jthread *thr = nullptr;
jthread *timer = nullptr;

struct input_item_t {
    unsigned int        index;
    bool                state;
};

queue<input_item_t> input_queue;

// bool prev_cc = false;

void btn_click(input_n_t btn)
{
    input_queue.emplace(btn, true);
    input_queue.emplace(btn, false);
}

unsigned int time_cntr = 0;

void stop_thread(jthread* & thr)
{
    if (thr != nullptr) {
        thr->request_stop();
        thr->join();
        delete thr;
        thr = nullptr;
    }
}

void timer_thr(stop_token stoken, input_n_t btn)
{
    using namespace chrono_literals;

    time_cntr = 0;
    const auto start_delay  = 500ms,
               repeat_delay = 500ms;

    this_thread::sleep_for(start_delay);
    while (!stoken.stop_requested()) {
        ++time_cntr;
        btn_click(btn);
        this_thread::sleep_for(repeat_delay);
    }
}

void delay_thr(input_n_t btn)
{
    while(active_cc > 0.0) this_thread::sleep_for(10ms);
    // game_log(SCS_LOG_TYPE_message, "click2");
    btn_click(btn);
}

__attribute__((unused))
auto ie_less = [](const input_event& e1, const input_event& e2)
{
    return tie(e1.type, e1.code, e1.value) < tie(e2.type, e2.code, e2.value);
};

const map<input_event, function<void()>, decltype(ie_less)> evt_map {
#ifdef MERCEDES
    {input_event({.type=EV_KEY, .code=301, .value=1}), [](){
        input_queue.emplace(IN_REVERSE, false);
        input_queue.emplace(IN_NEUTRAL, true);
        btn_click(IN_PBRAKE);
    }},
    {{.type=EV_KEY, .code=302, .value=1}, [](){
        input_queue.emplace(IN_REVERSE, true);
        if (active_brake) btn_click(IN_PBRAKE);
    }},
    {{.type=EV_KEY, .code=303, .value=1}, [](){
        input_queue.emplace(IN_REVERSE, false);
        input_queue.emplace(IN_DRIVE, false);
        input_queue.emplace(IN_NEUTRAL, true);
    }},
    {{.type=EV_KEY, .code=704, .value=1}, [](){
        input_queue.emplace(IN_NEUTRAL, false);
        input_queue.emplace(IN_DRIVE, true);
        if (active_shifter_manual) btn_click(IN_AUTOSEQ);
    }},
    {{.type=EV_KEY, .code=705, .value=1}, [](){
        btn_click(IN_AUTOSEQ);
    }},
#endif
    // {{.type=EV_KEY, .code=715, .value=0}, [](){
    //      if (prev_cc) btn_click(IN_CCRES);
    // }},
    {{.type=EV_KEY, .code=715, .value=1}, [](){
//         prev_cc = (active_cc > 0.0);
         if (active_cc != 0.0) btn_click(IN_CC);     // cancel only, don't turn on if off
    }},
    {{.type=EV_KEY, .code=713, .value=1}, [](){ // down
        stop_thread(timer);
        timer = new jthread(timer_thr, IN_CCDEC);
    }},
    {{.type=EV_KEY, .code=713, .value=0}, [](){ // down
         if (time_cntr == 0) {
             if (active_cc != 0.0) {
                 //delay for the 2nd press
                 thread t(delay_thr, IN_CC);
                 t.detach();
             }
             // game_log(SCS_LOG_TYPE_message, "click");
             btn_click(IN_CC);
         }
        stop_thread(timer);
    }},
    {{.type=EV_KEY, .code=714, .value=1}, [](){ // up
        stop_thread(timer);
        timer = new jthread(timer_thr, IN_CCINC);
    }},
    {{.type=EV_KEY, .code=714, .value=0}, [](){ // up
         if (time_cntr == 0) btn_click(IN_CCRES);
        stop_thread(timer);
    }},
};

void evdev_thr(stop_token stop)
{
    using namespace chrono_literals;
    unsigned int flags = LIBEVDEV_READ_FLAG_NORMAL;

    while (!stop.stop_requested()) {
        if (libevdev_has_event_pending(input_dev) != 0) {
            input_event ev;
            int const res = libevdev_next_event(input_dev, flags, &ev);

            if (res != -EAGAIN) {
                auto p = evt_map.find(ev);

                if (p != evt_map.end()) {
                    p->second();
                }
            }
            if (res == LIBEVDEV_READ_STATUS_SYNC) flags = LIBEVDEV_READ_FLAG_SYNC;
        } else {
            flags = LIBEVDEV_READ_FLAG_NORMAL;
            this_thread::sleep_for(50ms);
        }
    }
}

SCSAPI_RESULT input_event_callback(scs_input_event_t *const event_info,
                                   const scs_u32_t /*flags*/,
                                   const scs_context_t /*unused NOLINT(misc-misplaced-const) */)
{
    SCSAPI_RESULT res = SCS_RESULT_not_found;

    if (!input_queue.empty()) {
        const auto &r = input_queue.front();

        event_info->input_index         = r.index;
        event_info->value_bool.value    = r.state;
        input_queue.pop();
        res = SCS_RESULT_ok;
    }
    return res;
}

int init_input_dev()
{
    using namespace filesystem;

    directory_entry const dir("/dev/input/by-id");
    const string evdev_name = "Gudsen_MOZA_Multi-function_Stalk";

    auto const& p = find_if(directory_iterator(dir), directory_iterator(), [&evdev_name](auto const &f)
    {
        return f.path().filename().string().find(evdev_name) != string::npos;
    });

    if (p == directory_iterator()) {
        game_log(SCS_LOG_TYPE_error, (evdev_name + " not found.").c_str());
        return -1;
    }

    int const fd = open(p->path().c_str(), O_RDONLY|O_NONBLOCK);

    if (fd < 0) {
        game_log(SCS_LOG_TYPE_error, strerror(errno));
        return fd;
    }

    return libevdev_new_from_fd(fd, &input_dev);
}

} // namespace

SCSAPI_RESULT scs_input_init(const scs_u32_t /*version*/, const scs_input_init_params_t *const params)
{
    const auto *const version_params = static_cast<const scs_input_init_params_v100_t *>(params);

    if (init_input_dev() != 0) {
        game_log(SCS_LOG_TYPE_error, "Unable to open an input device");
        return SCS_RESULT_not_found;
    }

    // Setup the device information. The name of the input matches the name of the
    // mix as seen in controls.sii. Note that only some inputs are supported this way.
    // See documentation of SCS_INPUT_DEVICE_TYPE_semantical

    scs_input_device_t device_info;
    memset(&device_info, 0, sizeof(device_info));
    device_info.name = "virt_toggles";
    device_info.display_name = "VirtualToggles";
    device_info.type = SCS_INPUT_DEVICE_TYPE_semantical;
    device_info.input_count = inputs.size();
    device_info.inputs = inputs.data();
    device_info.input_event_callback = input_event_callback;
    device_info.callback_context = nullptr;

    if (version_params->register_device(&device_info) != SCS_RESULT_ok) {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to register device");
        return SCS_RESULT_generic_error;
    }

    thr = new jthread(evdev_thr);

    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_input_shutdown()
{
    stop_thread(thr);

    if (input_dev) {
        int const fd = libevdev_get_fd(input_dev);

        libevdev_free(input_dev);
        close(fd);

        input_dev = nullptr;
    }
}
