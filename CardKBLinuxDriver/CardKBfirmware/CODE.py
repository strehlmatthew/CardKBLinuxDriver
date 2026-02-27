import board
import digitalio
import time
import os

# --- PIN CONFIGURATION (Hardware SPI layout) ---
SCK  = digitalio.DigitalInOut(board.GP2)
MOSI = digitalio.DigitalInOut(board.GP3)
MISO = digitalio.DigitalInOut(board.GP4)
RST  = digitalio.DigitalInOut(board.GP0)

SCK.direction  = digitalio.Direction.OUTPUT
MOSI.direction = digitalio.Direction.OUTPUT
MISO.direction = digitalio.Direction.INPUT
RST.direction  = digitalio.Direction.OUTPUT

# Default States
SCK.value = False
RST.value = True

# --- SETTINGS ---
FILENAME = "firmware.hex"
PAGE_SIZE = 64 # ATmega8 Page Size

def spi_transfer(byte_val):
    received = 0
    for i in range(8):
        if (byte_val & 0x80): MOSI.value = True
        else: MOSI.value = False
        byte_val <<= 1
        time.sleep(0.000001)
        SCK.value = True # Clock High
        received <<= 1
        if MISO.value: received |= 1
        time.sleep(0.000001)
        SCK.value = False # Clock Low
    return received

def connect():
    print("Resetting Target...")
    SCK.value = False
    RST.value = True
    time.sleep(0.05)
    RST.value = False # Hold Reset
    time.sleep(0.05)
    RST.value = True  # Pulse
    time.sleep(0.001)
    RST.value = False # Hold Low
    time.sleep(0.05)

    # Handshake: 0xAC 0x53 0x00 0x00
    r1 = spi_transfer(0xAC)
    r2 = spi_transfer(0x53)
    r3 = spi_transfer(0x00) # <--- YOUR CHIP REPLIES HERE
    r4 = spi_transfer(0x00)

    print(f"Handshake Response: {hex(r1)} {hex(r2)} {hex(r3)} {hex(r4)}")

    # Check if 0x53 appeared in slot 3 (Standard) or slot 2 (Fast)
    if r3 == 0x53 or r2 == 0x53:
        print("-> SYNC OK.")
        return True
    return False

def send_cmd(b1, b2, b3, b4):
    spi_transfer(b1)
    spi_transfer(b2)
    spi_transfer(b3)
    return spi_transfer(b4)

def read_hex_file(filename):
    print(f"Reading {filename}...")
    data_buffer = {}
    try:
        with open(filename, "r") as f:
            for line in f:
                if line[0] != ':': continue
                byte_count = int(line[1:3], 16)
                address = int(line[3:7], 16)
                record_type = int(line[7:9], 16)
                if record_type == 0: # Data
                    for i in range(byte_count):
                        byte = int(line[9+i*2 : 11+i*2], 16)
                        data_buffer[address + i] = byte
    except Exception as e:
        print(f"Error reading file: {e}")
        return None
    return data_buffer

def flash_firmware():
    print("\n--- RAW FLASHER STARTING ---")

    if not connect():
        print("Failed to sync. Power cycle the CardKB and try again.")
        return

    # 1. Check Signature
    s1 = send_cmd(0x30, 0x00, 0x00, 0x00)
    s2 = send_cmd(0x30, 0x00, 0x01, 0x00)
    s3 = send_cmd(0x30, 0x00, 0x02, 0x00)
    print(f"Signature: {hex(s1)} {hex(s2)} {hex(s3)}")
    if s1 != 0x1E or s2 != 0x93 or s3 != 0x07:
        print("Wrong chip! Stopping.")
        return

    # 2. Erase Chip
    print("Erasing Chip...")
    send_cmd(0xAC, 0x80, 0x00, 0x00)
    time.sleep(0.05)
    connect() # Re-sync after erase

    # 3. Write Fuses (8MHz Internal)
    print("Writing Fuses (Low: 0xE4, High: 0xD9)...")
    send_cmd(0xAC, 0xA0, 0x00, 0xE4) # Write Low Fuse
    send_cmd(0xAC, 0xA8, 0x00, 0xD9) # Write High Fuse
    time.sleep(0.01)

    # 4. Load Firmware Data
    fw_data = read_hex_file(FILENAME)
    if not fw_data: return

    max_addr = max(fw_data.keys())
    print(f"Flashing {len(fw_data)} bytes...")

    # 5. Page Write Loop
    for page_addr in range(0, max_addr + 1, PAGE_SIZE):
        # Fill Buffer
        for i in range(PAGE_SIZE // 2): # Words
            byte_addr = page_addr + (i * 2)
            low_byte = fw_data.get(byte_addr, 0xFF)
            high_byte = fw_data.get(byte_addr + 1, 0xFF)

            # Load Low Byte
            send_cmd(0x40, 0x00, i, low_byte)
            # Load High Byte
            send_cmd(0x48, 0x00, i, high_byte)

        # Write Page
        cmd_addr = page_addr >> 1
        send_cmd(0x4C, (cmd_addr >> 8) & 0xFF, cmd_addr & 0xFF, 0x00)
        time.sleep(0.01) # Wait for write

        if page_addr % 256 == 0:
            print(f"Wrote page at {page_addr}...")

    print("\n--- FLASHING COMPLETE! ---")
    print("Disconnecting Reset...")
    RST.value = True

# Run Process
flash_firmware()

while True:
    time.sleep(1)
