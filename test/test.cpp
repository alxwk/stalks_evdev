#include <iostream>
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <libevdev/libevdev-uinput.h>

#include <scssdk_input.h>

using namespace std;

namespace {

scs_input_device_t indev;

void proc(scs_input_event_t &event_info)
{
    for(;;) {
        SCSAPI_RESULT res = indev.input_event_callback(&event_info, scs_u32_t(), scs_context_t());
        if (res == SCS_RESULT_not_found) {
            cout << "-- end -- " << endl;
            break;
        }
        cout << indev.inputs[event_info.input_index].display_name << ' ' << bool(event_info.value_bool.value) << endl;
    }
}

}

int main()
{
    int err;
    struct libevdev *evdev;
    struct libevdev_uinput *uidev;

    evdev = libevdev_new();
    const string vendor = "Gudsen";
    const string product = "MOZA Multi-function Stalk";
    const string name = vendor + ' ' + product;

    libevdev_set_id_bustype(evdev, BUS_USB);
    libevdev_set_id_vendor(evdev, 0x346e);
    libevdev_set_id_product(evdev, 0x0024);

    libevdev_set_name(evdev, name.c_str());
    libevdev_enable_event_type(evdev, EV_KEY);
    libevdev_enable_event_code(evdev, EV_KEY, 713, NULL);
    libevdev_enable_event_code(evdev, EV_KEY, 714, NULL);
    libevdev_enable_event_code(evdev, EV_KEY, 715, NULL);

    err = libevdev_uinput_create_from_device(evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);

    if (err != 0) {
        cerr << "can't create uinput device: " << err << endl;
        return -1;
    }

    sleep(1);

    scs_input_init_params_v100_t parms{};

    parms.common.log = [](const scs_log_type_t type, const scs_string_t message) {
        cout << type << ": " << message << endl;
    };

    parms.register_device = [](const scs_input_device_t *device_info) -> SCSAPI_RESULT {
        indev = *device_info;
        return SCS_RESULT_ok;
    };

    if (scs_input_init(0, &parms) != SCS_RESULT_ok) {
        cerr << "error at plugin init" << endl;
        return -1;
    }

    cout << unitbuf;

    cout << indev.display_name << endl;

    scs_input_event_t event_info;

    cout << "-- Short press down" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "-- Long press down (0.3 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(300000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "-- Long press down (0.6 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(600000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "-- Long press down (1.6 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(1600000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "-- Short press up" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 714, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);
    libevdev_uinput_write_event(uidev, EV_KEY, 714, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "-- Long press up" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 714, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(600000);
    libevdev_uinput_write_event(uidev, EV_KEY, 714, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    libevdev_uinput_destroy(uidev);

    return 0;
}
