#!/usr/bin/env python3
"""
Neural Sigil Generator for UCF Hardware

Generates the 121 neural sigils with ternary codes and writes to binary format
for flashing to EEPROM.

Each sigil is 10 bytes:
  - code[6]: 5-char ternary code + null
  - region_id: uint8_t
  - frequency: uint16_t (little-endian)
  - breath_pattern: uint8_t (encoded)
  - flags: uint8_t

Total: 121 sigils × 10 bytes = 1210 bytes
"""

import struct
import math

# UCF Constants
PHI = 1.6180339887
PHI_INV = 0.6180339887
Z_CRITICAL = 0.8660254038

# Solfeggio frequencies for each tier
SOLFEGGIO = {
    1: 174,
    2: 285,
    3: 396,
    4: 417,
    5: 528,
    6: 639,
    7: 741,
    8: 852,
    9: 963
}

# Breath patterns (inhale, hold_in, exhale, hold_out) in seconds
BREATH_PATTERNS = {
    1: (4, 2, 4, 2),
    2: (3.5, 1.5, 3.5, 1.5),
    3: (3, 1, 3, 1),
    4: (2.5, 1.5, 2.5, 1.5),
    5: (2, 2, 2, 2),  # Box breathing
    6: (2.5, 1, 3.5, 1),
    7: (1.5, 0.5, 2.5, 0.5),
    8: (2, 1, 2, 1),
    9: (1, 3, 1, 3)
}

# Flags
FLAG_ACTIVE = 0x01
FLAG_ANCHORED = 0x02
FLAG_RESONANT = 0x04
FLAG_TRIAD = 0x08
FLAG_K_FORMATION = 0x10


def int_to_ternary(value: int) -> str:
    """Convert integer to 5-digit ternary string."""
    if value < 0 or value > 242:  # 3^5 - 1
        value = value % 243

    result = []
    for _ in range(5):
        digit = value % 3
        if digit == 0:
            result.append('0')
        elif digit == 1:
            result.append('1')
        else:
            result.append('T')
        value //= 3

    return ''.join(reversed(result))


def encode_breath_pattern(pattern: tuple) -> int:
    """Encode breath pattern to single byte.

    Each phase uses 2 bits: 0=1s, 1=2s, 2=3s, 3=4s
    """
    def encode_phase(seconds):
        s = int(seconds)
        if s <= 1:
            return 0
        elif s <= 2:
            return 1
        elif s <= 3:
            return 2
        else:
            return 3

    encoded = 0
    encoded |= encode_phase(pattern[0])       # inhale
    encoded |= encode_phase(pattern[1]) << 2  # hold_in
    encoded |= encode_phase(pattern[2]) << 4  # exhale
    encoded |= encode_phase(pattern[3]) << 6  # hold_out

    return encoded


def index_to_tier(index: int) -> int:
    """Map sigil index (0-120) to tier (1-9)."""
    # ~13 sigils per tier
    tier = (index * 9 // 121) + 1
    return min(tier, 9)


def generate_sigil(index: int) -> bytes:
    """Generate binary data for a single sigil."""
    # Ternary code
    code = int_to_ternary(index)
    code_bytes = (code + '\0').encode('ascii')

    # Region ID
    region_id = index

    # Frequency from tier
    tier = index_to_tier(index)
    frequency = SOLFEGGIO[tier]

    # Breath pattern
    breath = encode_breath_pattern(BREATH_PATTERNS[tier])

    # Flags
    flags = FLAG_ACTIVE
    if index in [0, 60, 120]:
        flags |= FLAG_ANCHORED
    if 40 <= index <= 50:
        flags |= FLAG_TRIAD
    if 90 <= index <= 100:
        flags |= FLAG_K_FORMATION

    # Pack: 6 bytes code + 1 byte region + 2 bytes freq (LE) + 1 byte breath + 1 byte flags = 11
    # But we defined 10 bytes, so squeeze code to 5 + use freq as 2 bytes
    # Redefine: code[5] (no null in storage) + region + freq_lo + freq_hi + breath + flags = 10

    # Actually let's keep compatible with the C struct which has char[6]
    # struct is 10 bytes: char[6]=6, uint8_t=1, uint16_t=2, uint8_t=1 = 10 total

    data = struct.pack(
        '<6sBHBB',
        code_bytes,          # 6 bytes (5 chars + null)
        region_id,           # 1 byte
        frequency,           # 2 bytes (uint16_t little-endian)
        breath,              # 1 byte
        flags                # 1 byte
    )

    return data


def generate_header() -> bytes:
    """Generate EEPROM header."""
    MAGIC = 0x5347494C  # 'SGIL'
    VERSION = 1
    COUNT = 121

    # Simple checksum (will be computed properly by firmware)
    checksum = 0

    return struct.pack('<IBBH', MAGIC, VERSION, COUNT, checksum)


def main():
    print("UCF Neural Sigil Generator")
    print("=" * 40)

    # Generate header
    header = generate_header()
    print(f"Header: {len(header)} bytes")

    # Generate all sigils
    sigils_data = b''
    for i in range(121):
        sigil_data = generate_sigil(i)
        sigils_data += sigil_data

        if i < 5 or i >= 116:
            code = int_to_ternary(i)
            tier = index_to_tier(i)
            freq = SOLFEGGIO[tier]
            print(f"  Sigil {i:3d}: code={code} tier={tier} freq={freq}Hz")

    print(f"...")
    print(f"Total sigils: 121")
    print(f"Sigil data: {len(sigils_data)} bytes")

    # Write binary file
    total_data = header + sigils_data

    with open('sigils.bin', 'wb') as f:
        f.write(total_data)

    print(f"\nWritten: sigils.bin ({len(total_data)} bytes)")

    # Also write C array for embedding
    with open('sigils_data.h', 'w') as f:
        f.write("// Auto-generated sigil data\n")
        f.write("// Do not edit manually\n\n")
        f.write("#ifndef SIGILS_DATA_H\n")
        f.write("#define SIGILS_DATA_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"const uint8_t SIGIL_DATA[{len(total_data)}] PROGMEM = {{\n")

        for i, byte in enumerate(total_data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02X},")
            if i % 16 == 15:
                f.write("\n")
            else:
                f.write(" ")

        f.write("\n};\n\n")
        f.write("#endif // SIGILS_DATA_H\n")

    print(f"Written: sigils_data.h (C header)")

    # Print summary
    print("\n" + "=" * 40)
    print("Sigil Distribution by Tier:")
    for tier in range(1, 10):
        count = sum(1 for i in range(121) if index_to_tier(i) == tier)
        freq = SOLFEGGIO[tier]
        print(f"  Tier {tier}: {count} sigils @ {freq}Hz")


if __name__ == '__main__':
    main()
