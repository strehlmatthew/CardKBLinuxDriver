# CardKBLinuxDriver
====================================
      CUSTOM CARDKB FIRMWARE & DRIVER MANUAL
==============================================

1. THE NEOPIXEL STATUS LED
---------------------------------------------------------
The built-in LED tells you exactly what layer or modifier 
is currently active. 
• Blinking = "Single-Shot" (applies to the very next key you press).
• Solid    = "Locked" (applies to all keys until canceled).

[Colors]
• RED     : Shift / Caps Lock
• CYAN    : Sym (Acts as the 'Alt' modifier in OS)
• BLUE    : Fn  (Acts as the 'Ctrl' modifier in OS)
• YELLOW  : Super / Windows / GUI Key (Solid only)
• WHITE   : Key successfully registered and sent to Pi.

2. THE SMART MODIFIER SYSTEM
---------------------------------------------------------
This keyboard uses a smart tapping system for modifiers 
(Shift, Sym, Fn) rather than requiring you to hold them.

• Single Tap: Activates "Single-Shot" mode (LED blinks). 
              The modifier applies to the next key pressed, 
              then automatically turns off.
• Double Tap: Activates "Locked" mode (LED solid).
              The modifier stays active until you turn it off.
              (Double-tap Shift = Caps Lock).
• Auto-Unlatch: Executing any shortcut automatically clears 
                the modifiers. You never get "trapped" in a layer.
• Global Cancel: Made a mistake? If ANY modifier is blinking 
                 or solid, tapping ANY modifier key once will 
                 instantly abort and clear all active modifiers.

3. FUNCTION (Fn) LAYER = CONTROL (Ctrl)
---------------------------------------------------------
The Fn button maps natively to the Linux 'Ctrl' key.
• Fn + [Letter] = Standard Ctrl shortcuts (Ctrl+C, Ctrl+V, etc.)
• Fn + Enter    = PASTE (Outputs Shift+Insert for Linux terminals)
• Fn + Esc      = Delete
• Fn + Tab      = Insert
• Fn + Arrows   = Home / End / Page Up / Page Down

4. SYMBOL (Sym) LAYER = ALT
---------------------------------------------------------
The Sym button maps natively to the Linux 'Alt' key.
• Sym + [Key]   = Outputs the secondary symbol printed on the key.
• Sym (Locked)  = Acts as holding the 'Alt' key for OS shortcuts 
                  like Alt+Tab (App Switcher) or Alt+Arrows.

5. THE F-KEYS (F1 - F12)
---------------------------------------------------------
F-keys are triggered by applying 'Shift' to the top row.
• Shift + 1 through 0 = F1 through F10
• Shift + , (Comma)   = F11 (Fullscreen in most apps)
• Shift + . (Period)  = F12 (Developer tools)

6. THE SUPER / WINDOWS KEY
---------------------------------------------------------
Trigger the OS Super key (opens Start Menu/App Launcher).
• Action: Tap 'Fn', then tap 'Space'.
• The LED will turn YELLOW. The Super key is now held down.
• Tap Space again (or type a shortcut) to release it.

7. CTRL + ALT + DEL
---------------------------------------------------------
To trigger a true hardware-level Ctrl+Alt+Del:
1. Tap 'Sym' (LED blinks Cyan)
2. Tap 'Fn'  (LED blinks Blue)
3. Tap 'Del' 
The modifiers will automatically unlatch immediately after.
