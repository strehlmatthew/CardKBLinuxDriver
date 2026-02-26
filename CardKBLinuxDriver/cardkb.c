#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_DEV "/dev/i2c-3"
#define CARDKB_ADDR 0x5F

int hid_to_linux(int hid) {
    switch(hid) {
        case 0x1E: return KEY_1; case 0x1F: return KEY_2; case 0x20: return KEY_3;
        case 0x21: return KEY_4; case 0x22: return KEY_5; case 0x23: return KEY_6;
        case 0x24: return KEY_7; case 0x25: return KEY_8; case 0x26: return KEY_9;
        case 0x27: return KEY_0;
        case 0x14: return KEY_Q; case 0x1A: return KEY_W; case 0x08: return KEY_E;
        case 0x15: return KEY_R; case 0x17: return KEY_T; case 0x1C: return KEY_Y;
        case 0x18: return KEY_U; case 0x0C: return KEY_I; case 0x12: return KEY_O;
        case 0x13: return KEY_P;
        case 0x04: return KEY_A; case 0x16: return KEY_S; case 0x07: return KEY_D;
        case 0x09: return KEY_F; case 0x0A: return KEY_G; case 0x0B: return KEY_H;
        case 0x0D: return KEY_J; case 0x0E: return KEY_K; case 0x0F: return KEY_L;
        case 0x1D: return KEY_Z; case 0x1B: return KEY_X; case 0x06: return KEY_C;
        case 0x19: return KEY_V; case 0x05: return KEY_B; case 0x11: return KEY_N;
        case 0x10: return KEY_M;
        case 0x28: return KEY_ENTER;     case 0x29: return KEY_ESC;
        case 0x2A: return KEY_BACKSPACE; case 0x2B: return KEY_TAB;
        case 0x2C: return KEY_SPACE;     case 0x36: return KEY_COMMA;
        case 0x37: return KEY_DOT;       case 0x50: return KEY_LEFT;
        case 0x52: return KEY_UP;        case 0x51: return KEY_DOWN;
        case 0x4F: return KEY_RIGHT;
        case 0x3A: return KEY_F1; case 0x3B: return KEY_F2; case 0x3C: return KEY_F3;
        case 0x3D: return KEY_F4; case 0x3E: return KEY_F5; case 0x3F: return KEY_F6;
        case 0x40: return KEY_F7; case 0x41: return KEY_F8; case 0x42: return KEY_F9;
        case 0x43: return KEY_F10; case 0x44: return KEY_F11; case 0x45: return KEY_F12;
        case 0x4A: return KEY_HOME; case 0x4B: return KEY_PAGEUP;
        case 0x4E: return KEY_PAGEDOWN; case 0x4D: return KEY_END;
        default: return 0;
    }
}

int main() {
    int i2c_fd, uinput_fd;
    struct uinput_setup usetup;
    unsigned char buf[8];
    int active_keys[6] = {0};
    int last_mods = 0;

    i2c_fd = open(I2C_DEV, O_RDWR);
    if (i2c_fd < 0) return 1;
    ioctl(i2c_fd, I2C_SLAVE, CARDKB_ADDR);
    
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) return 1;

    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_REP);
    for (int i = 0; i < 255; i++) ioctl(uinput_fd, UI_SET_KEYBIT, i);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    strcpy(usetup.name, "CardKB_Raw_Passthrough");
    ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
    ioctl(uinput_fd, UI_DEV_CREATE);

    while (1) {
        if (read(i2c_fd, buf, 8) == 8) {
            int sync_needed = 0;
            int current_mods = buf[0];
            int current_keys[6] = {0};
            for (int i = 0; i < 6; i++) current_keys[i] = hid_to_linux(buf[i + 2]);

            // 1. Modifiers
            int m_bits[] = {0x01, 0x02, 0x04, 0x08};
            int m_keys[] = {KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA};
            for(int i = 0; i < 4; i++) {
                if ((current_mods & m_bits[i]) && !(last_mods & m_bits[i])) {
                    struct input_event ev = {.type = EV_KEY, .code = m_keys[i], .value = 1};
                    write(uinput_fd, &ev, sizeof(ev)); sync_needed = 1;
                }
                if (!(current_mods & m_bits[i]) && (last_mods & m_bits[i])) {
                    struct input_event ev = {.type = EV_KEY, .code = m_keys[i], .value = 0};
                    write(uinput_fd, &ev, sizeof(ev)); sync_needed = 1;
                }
            }
            last_mods = current_mods;

            // 2. Keys (Release)
            for (int i = 0; i < 6; i++) {
                if (active_keys[i] != 0) {
                    int found = 0;
                    for(int j = 0; j < 6; j++) if (current_keys[j] == active_keys[i]) found = 1;
                    if (!found) {
                        struct input_event ev = {.type = EV_KEY, .code = active_keys[i], .value = 0};
                        write(uinput_fd, &ev, sizeof(ev));
                        active_keys[i] = 0; sync_needed = 1;
                    }
                }
            }
            
            // 3. Keys (Press)
            for (int i = 0; i < 6; i++) {
                if (current_keys[i] != 0) {
                    int is_active = 0;
                    for(int j = 0; j < 6; j++) if (active_keys[j] == current_keys[i]) is_active = 1;
                    if (!is_active) {
                        for(int j = 0; j < 6; j++) {
                            if (active_keys[j] == 0) {
                                active_keys[j] = current_keys[i];
                                struct input_event ev = {.type = EV_KEY, .code = current_keys[i], .value = 1};
                                write(uinput_fd, &ev, sizeof(ev));
                                sync_needed = 1; break;
                            }
                        }
                    }
                }
            }
            
            // 4. Sync Report
            if (sync_needed) {
                struct input_event syn = {.type = EV_SYN, .code = SYN_REPORT, .value = 0};
                write(uinput_fd, &syn, sizeof(syn));
            }
        }
        usleep(10000); // Efficient 10ms poll
    }
    return 0;
}
