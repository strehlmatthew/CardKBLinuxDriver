#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define PIN           13
#define NUMPIXELS      1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define shiftPressed ((PINB & 0x10) == 0)
#define symPressed   ((PINB & 0x80) == 0)
#define fnPressed    ((PINB & 0x40) == 0)

// --- 16-BIT HID KEYMAP (Normal, Sym, Fn) ---
uint16_t HID_KeyMap[48][3] =
{ 
  // Normal       Sym Layer       Fn Layer (Ctrl / Nav / F-Keys)
  { 0x0029,       0x0029,         0x004C }, // esc    (Fn = Delete)
  { 0x001E,       0x021E,         0x011E }, // 1      (Sym = !, Fn = Ctrl+1)
  { 0x001F,       0x021F,         0x011F }, // 2      (Sym = @, Fn = Ctrl+2)
  { 0x0020,       0x0220,         0x0120 }, // 3      (Sym = #, Fn = Ctrl+3)
  { 0x0021,       0x0221,         0x0121 }, // 4      (Sym = $, Fn = Ctrl+4)
  { 0x0022,       0x0222,         0x0122 }, // 5      (Sym = %, Fn = Ctrl+5)
  { 0x0023,       0x0223,         0x0123 }, // 6      (Sym = ^, Fn = Ctrl+6)
  { 0x0024,       0x0224,         0x0124 }, // 7      (Sym = &, Fn = Ctrl+7)
  { 0x0025,       0x0225,         0x0125 }, // 8      (Sym = *, Fn = Ctrl+8)
  { 0x0026,       0x0226,         0x0126 }, // 9      (Sym = (, Fn = Ctrl+9)
  { 0x0027,       0x0227,         0x0127 }, // 0      (Sym = ), Fn = Ctrl+0)
  { 0x002A,       0x002A,         0x012A }, // del    (Fn = Ctrl+Del)
  { 0x002B,       0x002B,         0x0049 }, // tab    (Fn = Pure Insert)
  { 0x0014,       0x022F,         0x0114 }, // q      (Sym = {, Fn = Ctrl+Q)
  { 0x001A,       0x0230,         0x011A }, // w      (Sym = }, Fn = Ctrl+W)
  { 0x0008,       0x002F,         0x0108 }, // e      (Sym = [, Fn = Ctrl+E)
  { 0x0015,       0x0030,         0x0115 }, // r      (Sym = ], Fn = Ctrl+R)
  { 0x0017,       0x0038,         0x0117 }, // t      (Sym = /, Fn = Ctrl+T)
  { 0x001C,       0x0031,         0x011C }, // y      (Sym = \, Fn = Ctrl+Y)
  { 0x0018,       0x0231,         0x0118 }, // u      (Sym = |, Fn = Ctrl+U)
  { 0x000C,       0x0235,         0x010C }, // i      (Sym = ~, Fn = Ctrl+I)
  { 0x0012,       0x0034,         0x0112 }, // o      (Sym = ', Fn = Ctrl+O)
  { 0x0013,       0x0234,         0x0113 }, // p      (Sym = ", Fn = Ctrl+P)
  { 0x0000,       0x0000,         0x0000 }, // no key
  { 0x0050,       0x0050,         0x004A }, // LEFT   (Fn = Home)
  { 0x0052,       0x0052,         0x004B }, // UP     (Fn = Page Up)
  { 0x0004,       0x0033,         0x0104 }, // a      (Sym = ;, Fn = Ctrl+A)
  { 0x0016,       0x0233,         0x0116 }, // s      (Sym = :, Fn = Ctrl+S)
  { 0x0007,       0x0035,         0x0107 }, // d      (Sym = `, Fn = Ctrl+D)
  { 0x0009,       0x022E,         0x0109 }, // f      (Sym = +, Fn = Ctrl+F)
  { 0x000A,       0x002D,         0x010A }, // g      (Sym = -, Fn = Ctrl+G)
  { 0x000B,       0x022D,         0x010B }, // h      (Sym = _, Fn = Ctrl+H)
  { 0x000D,       0x002E,         0x010D }, // j      (Sym = =, Fn = Ctrl+J)
  { 0x000E,       0x0238,         0x010E }, // k      (Sym = ?, Fn = Ctrl+K)
  { 0x000F,       0x0000,         0x010F }, // l      (Fn = Ctrl+L)
  { 0x0028,       0x0028,         0x0249 }, // enter  (Fn = Shift+Insert for PASTE)
  { 0x0051,       0x0051,         0x004E }, // DOWN   (Fn = Page Down)
  { 0x004F,       0x004F,         0x004D }, // RIGHT  (Fn = End)
  { 0x001D,       0x0000,         0x011D }, // z      (Fn = Ctrl+Z)
  { 0x001B,       0x0000,         0x011B }, // x      (Fn = Ctrl+X)
  { 0x0006,       0x0000,         0x0106 }, // c      (Fn = Ctrl+C)
  { 0x0019,       0x0000,         0x0119 }, // v      (Fn = Ctrl+V)
  { 0x0005,       0x0000,         0x0105 }, // b      (Fn = Ctrl+B)
  { 0x0011,       0x0000,         0x0111 }, // n      (Fn = Ctrl+N)
  { 0x0010,       0x0000,         0x0110 }, // m      (Fn = Ctrl+M)
  { 0x0036,       0x0236,         0x0136 }, // ,      (Sym = <, Fn = Ctrl+,)
  { 0x0037,       0x0237,         0x0137 }, // .      (Sym = >, Fn = Ctrl+.)
  { 0x002C,       0x002C,         0x0000 }  // space  (Fn = Super Toggle)
};

uint8_t hid_report[8] = {0};
int idle = 0;

// Modifier Tracking (0 = Off, 1 = Blinking/Next Key, 2 = Locked)
int _shift = 0, _sym = 0, _fn = 0;

// Independent OS Locks for complex commands
bool super_locked = false;

void requestEvent() {
  Wire.write(hid_report, 8);
}

void setup() {
  pinMode(A3, OUTPUT); pinMode(A2, OUTPUT); pinMode(A1, OUTPUT); pinMode(A0, OUTPUT);
  DDRB = 0x00; PORTB = 0xff;
  DDRD = 0x00; PORTD = 0xff;
  pixels.begin(); pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
  Wire.begin(0x5f);
  Wire.onRequest(requestEvent);
}

void GetInput(uint8_t *keys, uint8_t &count) {
  count = 0;
  for (int row = 0; row < 4; row++) {
    
    // FIX: Set all row pins to floating inputs to prevent voltage collisions
    pinMode(A3, INPUT); pinMode(A2, INPUT); 
    pinMode(A1, INPUT); pinMode(A0, INPUT);
    
    // Only drive the currently active row to Ground (LOW)
    if (row == 0) { pinMode(A3, OUTPUT); digitalWrite(A3, LOW); }
    if (row == 1) { pinMode(A2, OUTPUT); digitalWrite(A2, LOW); }
    if (row == 2) { pinMode(A1, OUTPUT); digitalWrite(A1, LOW); }
    if (row == 3) { pinMode(A0, OUTPUT); digitalWrite(A0, LOW); }
    
    delayMicroseconds(500); 

    uint8_t d = ~PIND; 
    uint8_t b = ~PINB;

    for (int i = 0; i < 8; i++) {
      if ((d & (1 << i)) && count < 3) keys[count++] = (row * 12) + i + 1;
    }
    for (int i = 0; i < 4; i++) {
      if ((b & (1 << i)) && count < 3) keys[count++] = (row * 12) + 8 + i + 1;
    }
  }
}

void loop() {
  bool cur_shift = shiftPressed;
  bool cur_sym = symPressed;
  bool cur_fn = fnPressed;

  // --- STANDARD DOUBLE-TAP LOGIC WITH GLOBAL INSTANT CANCELLATIONS ---
  if (cur_shift) {
    idle = 0; while (shiftPressed) delay(1);
    if (_shift == 0) { 
        delay(200); 
        if(shiftPressed){ while(shiftPressed)delay(1); _shift=2; } 
        else { _shift=1; } 
    }
    else { 
        // GLOBAL CANCEL: Tapping Shift to turn it off wipes ALL modifiers
        _shift = 0; 
        _sym = 0; 
        _fn = 0; 
    }
  }
  
  if (cur_sym) {
    idle = 0; while (symPressed) delay(1);
    if (_sym == 0) { 
        delay(200); 
        if(symPressed){ while(symPressed)delay(1); _sym=2; } 
        else { _sym=1; } 
    }
    else { 
        // GLOBAL CANCEL: Tapping Sym to turn it off wipes ALL modifiers
        _sym = 0; 
        _shift = 0; 
        _fn = 0; 
    }
  }
  
  if (cur_fn) {
    idle = 0; while (fnPressed) delay(1);
    if (_fn == 0) { 
        delay(200); 
        if(fnPressed){ while(fnPressed)delay(1); _fn=2; } 
        else { _fn=1; } 
    }
    else { 
        // GLOBAL CANCEL: Tapping Fn to turn it off wipes ALL modifiers
        _fn = 0; 
        _shift = 0; 
        _sym = 0; 
    }
  }

  // --- SCAN FOR PHYSICAL KEYS ---
  uint8_t pressed_keys[3] = {0}; 
  uint8_t key_count = 0;
  GetInput(pressed_keys, key_count);

  // --- ONE-SHOT SUPER TOGGLE LOGIC ---
  bool space_pressed = false;
  for (int i = 0; i < key_count; i++) {
      if (pressed_keys[i] == 48) space_pressed = true; 
  }
  static bool prev_space_pressed = false;
  static bool prev_fn_btn = false;
  static bool ignore_space_release = false;

  if (!super_locked && _fn > 0 && space_pressed && !prev_space_pressed) {
      super_locked = true; 
      _fn = 0; 
      ignore_space_release = true;
  } else if (super_locked) {
      if ((_fn == 0 && space_pressed && !prev_space_pressed) || (cur_fn && !prev_fn_btn)) {
          super_locked = false;
          if (space_pressed) ignore_space_release = true;
          _fn = 0; 
      }
  }
  
  if (!space_pressed) {
      ignore_space_release = false;
  }

  prev_space_pressed = space_pressed;
  prev_fn_btn = cur_fn;

  // --- NEOPIXEL STATUS ---
  int r = 0, g = 0, b = 0;
  bool blink_off = ((idle / 10) % 2 == 1);

  if (_shift == 2) r = 1; 
  else if (_shift == 1) r = blink_off ? 0 : 1; 

  if (_sym == 2) { g = 1; b = 1; } 
  else if (_sym == 1) g = blink_off ? 0 : 1; 

  if (_fn == 2) { r = 1; b = 1; } 
  else if (_fn == 1) b = blink_off ? 0 : 1; 

  if (super_locked) { r = 1; g = 1; b = 0; } // YELLOW for Super Mode

  pixels.setPixelColor(0, pixels.Color(r, g, b));

  // --- BUILD THE HID REPORT ---
  uint8_t new_report[8] = {0};
  uint8_t current_modifiers = 0;

  if (_sym == 2 || (_sym == 1 && _fn > 0)) current_modifiers |= 0x04; // Alt
  if (super_locked) current_modifiers |= 0x08; // Super (GUI)

  uint8_t layer = 0;
  if (_sym == 1) layer = 1;
  if (_fn == 1 || _fn == 2) layer = 2;

  bool apply_shift = (_shift > 0);
  uint8_t valid_keys = 0;
  bool fkey_intercepted = false;

  for (int i = 0; i < key_count; i++) {
      uint8_t key_id = pressed_keys[i];
      
      if (key_id == 48 && (ignore_space_release || layer == 2)) continue; 

      uint16_t hid_data = HID_KeyMap[key_id - 1][layer];
      uint8_t keycode = hid_data & 0xFF;
      uint8_t key_mod = hid_data >> 8;
      
      // --- THE F-KEY INTERCEPT ---
      if (apply_shift && layer == 0) {
          if (key_id >= 2 && key_id <= 11) {
              keycode = 0x3A + (key_id - 2); // F1 to F10
              key_mod = 0; 
              fkey_intercepted = true;
          } else if (key_id == 46) {
              keycode = 0x44; // F11
              key_mod = 0;
              fkey_intercepted = true;
          } else if (key_id == 47) {
              keycode = 0x45; // F12
              key_mod = 0;
              fkey_intercepted = true;
          }
          
          if (fkey_intercepted && _shift == 2) {
              key_mod = 0x02; // Force Shift back onto the F-Key
          }
      }

      new_report[2 + valid_keys] = keycode; 
      current_modifiers |= key_mod; 

      // SMART CAPS LOCK
      if (apply_shift && !fkey_intercepted) {
          if (_shift == 2) {
              if (keycode >= 0x04 && keycode <= 0x1D && layer == 0) current_modifiers |= 0x02;
          } else {
              current_modifiers |= 0x02; 
          }
      }

      valid_keys++;
  }

  new_report[0] = current_modifiers;

  // --- TRIGGER I2C UPDATE ---
  for(int i = 0; i < 8; i++) {
      hid_report[i] = new_report[i];
  }

  if (valid_keys > 0) pixels.setPixelColor(0, pixels.Color(2, 2, 2));

  // --- AUTO-CLEAR BLINKING MODIFIERS & COMBO LOCKS ---
  static bool character_was_typed = false;
  if (valid_keys > 0) character_was_typed = true;

  if (key_count == 0 && character_was_typed) {
      if (_shift == 1) _shift = 0;
      
      if (_fn > 0) {
          _fn = 0;
          _sym = 0; 
      } else if (_sym == 1) {
          _sym = 0; 
      }

      if (super_locked) super_locked = false; 
      character_was_typed = false;
  }

  pixels.show();
  idle++;
  delay(10);
}