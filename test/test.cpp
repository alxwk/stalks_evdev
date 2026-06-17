#include <iostream>
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <libevdev/libevdev-uinput.h>

#include <scssdk_input.h>

using namespace std;

SCSAPI_RESULT scs_input_init(const scs_u32_t /*version*/, const scs_input_init_params_t *const params);

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

int main()
{
    int err;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;

    dev = libevdev_new();
    const string name = "Gudsen_MOZA_Multi-function_Stalk";
    libevdev_set_name(dev, name.c_str());
    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, 713, NULL);
    libevdev_enable_event_code(dev, EV_KEY, 714, NULL);
    libevdev_enable_event_code(dev, EV_KEY, 715, NULL);

    err = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);

    if (err != 0) {
        cerr << "can't create uinput device: " << err << endl;
    }

    const string devnode = libevdev_uinput_get_devnode(uidev);

    namespace fs = std::filesystem;

    const fs::path link(string("/dev/input/by-id/usb-")+name);

    if (!fs::exists(link)) {
        fs::create_symlink(devnode, link);
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

    SCSAPI_RESULT res = scs_input_init(0, &parms);

    cout << unitbuf;

    cout << indev.display_name << endl;

    scs_input_event_t event_info;

   /* Key press, report the event, send key release, and report again */

    cout << "Short press down" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "Long press down (0.3 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(300000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "Long press down (0.6 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(600000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "Long press down (1.6 s)" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 713, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(1600000);
    libevdev_uinput_write_event(uidev, EV_KEY, 713, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "Short press up" << endl;

    libevdev_uinput_write_event(uidev, EV_KEY, 714, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);
    libevdev_uinput_write_event(uidev, EV_KEY, 714, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    usleep(50000L);

    proc(event_info);
    sleep(1);

    cout << "Long press up" << endl;

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
