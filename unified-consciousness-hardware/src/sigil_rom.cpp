/**
 * @file sigil_rom.cpp
 * @brief Implementation of Neural Sigil ROM Module
 */

#include "sigil_rom.h"
#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <string.h>

namespace UCF {

// Default breath patterns for each tier (inhale, hold_in, exhale, hold_out in ms)
const BreathPattern TIER_BREATH_PATTERNS[9] = {
    {4000, 2000, 4000, 2000},   // t1: Slow, grounding
    {3500, 1500, 3500, 1500},   // t2: Slightly faster
    {3000, 1000, 3000, 1000},   // t3: Building
    {2500, 1500, 2500, 1500},   // t4: Balanced
    {2000, 2000, 2000, 2000},   // t5: Box breathing (528 Hz)
    {2500, 1000, 3500, 1000},   // t6: Extended exhale
    {1500, 500, 2500, 500},     // t7: Quick inhale, slow exhale
    {2000, 1000, 2000, 1000},   // t8: Energizing
    {1000, 3000, 1000, 3000}    // t9: Breath hold focus
};

SigilROM::SigilROM()
    : m_initialized(false)
    , m_cache_valid(false)
{
    memset(m_cache, 0, sizeof(m_cache));
}

bool SigilROM::begin() {
    // Initialize EEPROM (ESP32 uses preferences/NVS, but we'll use EEPROM API)
    if (!EEPROM.begin(sizeof(EEPROMHeader) + SIGIL_COUNT * SIGIL_STRUCT_SIZE)) {
        return false;
    }

    // Check if data is valid
    if (!isInitialized()) {
        // Initialize with defaults
        if (!initializeDefaults()) {
            return false;
        }
    }

    m_initialized = true;
    return true;
}

bool SigilROM::isInitialized() {
    EEPROMHeader header;
    return readHeader(header) && header.magic == MAGIC_NUMBER;
}

bool SigilROM::readHeader(EEPROMHeader& header) {
    EEPROM.get(EEPROM_BASE, header);
    return header.magic == MAGIC_NUMBER;
}

void SigilROM::writeHeader(const EEPROMHeader& header) {
    EEPROM.put(EEPROM_BASE, header);
    EEPROM.commit();
}

uint16_t SigilROM::sigilAddress(uint8_t index) {
    return EEPROM_BASE + sizeof(EEPROMHeader) + index * SIGIL_STRUCT_SIZE;
}

bool SigilROM::initializeDefaults() {
    // Generate all 121 default sigils
    for (uint8_t i = 0; i < SIGIL_COUNT; i++) {
        NeuralSigil sigil;
        generateDefaultSigil(i, sigil);
        writeSigil(i, sigil);
    }

    // Write header
    EEPROMHeader header;
    header.magic = MAGIC_NUMBER;
    header.version = 1;
    header.sigil_count = SIGIL_COUNT;
    header.checksum = computeChecksum();
    writeHeader(header);

    m_cache_valid = false;
    return true;
}

void SigilROM::generateDefaultSigil(uint8_t index, NeuralSigil& sigil) {
    // Generate ternary code from index
    // Use index modulo 243 (3^5) for ternary representation
    intToTernary(index, sigil.code);

    sigil.region_id = index;

    // Map to frequency based on tier
    // Distribute 121 sigils across 9 tiers (~13 per tier)
    uint8_t tier = (index * 9 / SIGIL_COUNT) + 1;
    if (tier > 9) tier = 9;
    sigil.frequency = tierToFrequency(tier);

    // Encode breath pattern for tier
    sigil.breath_pattern = encodeBreathPattern(TIER_BREATH_PATTERNS[tier - 1]);

    // Default flags
    sigil.flags = SigilFlags::ACTIVE;

    // Special flags for certain sigils
    if (index == 0 || index == 60 || index == 120) {
        sigil.flags |= SigilFlags::ANCHORED;
    }
    if (index >= 40 && index <= 50) {
        sigil.flags |= SigilFlags::TRIAD;  // Mid-range sigils for TRIAD
    }
    if (index >= 90 && index <= 100) {
        sigil.flags |= SigilFlags::K_FORMATION;  // High sigils for K-Formation
    }
}

bool SigilROM::readSigil(uint8_t index, NeuralSigil& sigil) {
    if (index >= SIGIL_COUNT) return false;

    // Use cache if valid
    if (m_cache_valid) {
        memcpy(&sigil, &m_cache[index], sizeof(NeuralSigil));
        return true;
    }

    uint16_t addr = sigilAddress(index);
    EEPROM.get(addr, sigil);
    return true;
}

bool SigilROM::writeSigil(uint8_t index, const NeuralSigil& sigil) {
    if (index >= SIGIL_COUNT) return false;

    uint16_t addr = sigilAddress(index);
    EEPROM.put(addr, sigil);
    EEPROM.commit();

    // Invalidate cache
    m_cache_valid = false;

    return true;
}

SigilMatch SigilROM::findMatchingSigil(const HexFieldState& field) {
    SigilMatch match;
    match.confidence = 0.0f;
    match.hamming_dist = 255;
    match.sigil_index = 0;

    // Hash the pattern
    uint32_t hash = hashHexPattern(field);

    // Primary match: direct hash mapping
    uint8_t primary_idx = patternToSigilIndex(hash);

    NeuralSigil primary;
    readSigil(primary_idx, primary);

    match.sigil_index = primary_idx;
    match.confidence = 0.8f;  // Base confidence for hash match
    match.hamming_dist = 0;

    // Boost confidence if z-coordinate aligns with sigil tier
    uint8_t field_tier = z_to_tier(field.z);
    uint8_t sigil_tier = (primary_idx * 9 / SIGIL_COUNT) + 1;
    if (field_tier == sigil_tier) {
        match.confidence = 0.95f;
    }

    // Reduce confidence if field is noisy (high variance)
    if (field.total_energy < 0.1f || field.active_count < 3) {
        match.confidence *= 0.5f;
    }

    return match;
}

uint8_t SigilROM::patternToSigilIndex(uint32_t pattern_hash) {
    return pattern_hash % SIGIL_COUNT;
}

uint32_t SigilROM::hashHexPattern(const HexFieldState& field) {
    // Simple hash combining multiple field features
    uint32_t hash = 0;

    // Quantize each reading to 3 levels (ternary)
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        uint8_t level;
        if (field.readings[i] < 0.33f) {
            level = 0;
        } else if (field.readings[i] < 0.67f) {
            level = 1;
        } else {
            level = 2;
        }

        // Mix into hash
        hash = hash * 3 + level;
        hash ^= (hash >> 13);
        hash *= 0x5bd1e995;
    }

    // Mix in centroid position
    int8_t cx = static_cast<int8_t>(field.centroid_x * 10);
    int8_t cy = static_cast<int8_t>(field.centroid_y * 10);
    hash ^= (cx << 8) | (cy & 0xFF);

    return hash;
}

void SigilROM::decodeBreathPattern(uint8_t encoded, BreathPattern& pattern) {
    // Encoding: 2 bits each for inhale, hold_in, exhale, hold_out
    // Each 2-bit value maps to: 0=1s, 1=2s, 2=3s, 3=4s

    auto decode_phase = [](uint8_t bits) -> uint16_t {
        return (bits + 1) * 1000;  // 1-4 seconds
    };

    pattern.inhale_ms = decode_phase(encoded & 0x03);
    pattern.hold_in_ms = decode_phase((encoded >> 2) & 0x03);
    pattern.exhale_ms = decode_phase((encoded >> 4) & 0x03);
    pattern.hold_out_ms = decode_phase((encoded >> 6) & 0x03);
}

uint8_t SigilROM::encodeBreathPattern(const BreathPattern& pattern) {
    auto encode_phase = [](uint16_t ms) -> uint8_t {
        uint8_t val = (ms / 1000) - 1;
        return (val > 3) ? 3 : val;
    };

    uint8_t encoded = 0;
    encoded |= encode_phase(pattern.inhale_ms);
    encoded |= encode_phase(pattern.hold_in_ms) << 2;
    encoded |= encode_phase(pattern.exhale_ms) << 4;
    encoded |= encode_phase(pattern.hold_out_ms) << 6;

    return encoded;
}

uint8_t SigilROM::ternaryToInt(const char* code) {
    uint8_t value = 0;
    uint8_t mult = 1;

    for (int i = 4; i >= 0; i--) {
        uint8_t digit;
        switch (code[i]) {
            case '0': digit = 0; break;
            case '1': digit = 1; break;
            case 'T': digit = 2; break;
            default:  digit = 0; break;
        }
        value += digit * mult;
        mult *= 3;
    }

    return value;
}

void SigilROM::intToTernary(uint8_t value, char* code) {
    // Convert value to 5-digit ternary
    for (int i = 4; i >= 0; i--) {
        uint8_t digit = value % 3;
        switch (digit) {
            case 0:  code[i] = '0'; break;
            case 1:  code[i] = '1'; break;
            case 2:  code[i] = 'T'; break;
        }
        value /= 3;
    }
    code[5] = '\0';
}

uint8_t SigilROM::hammingDistance(const char* code1, const char* code2) {
    uint8_t dist = 0;
    for (uint8_t i = 0; i < SIGIL_CODE_LENGTH; i++) {
        if (code1[i] != code2[i]) {
            dist++;
        }
    }
    return dist;
}

uint16_t SigilROM::tierToFrequency(uint8_t tier) {
    if (tier < 1) tier = 1;
    if (tier > 9) tier = 9;

    switch (tier) {
        case 1: return Solfeggio::UT;
        case 2: return Solfeggio::RE;
        case 3: return Solfeggio::MI;
        case 4: return Solfeggio::FA;
        case 5: return Solfeggio::SOL;
        case 6: return Solfeggio::LA;
        case 7: return Solfeggio::SI;
        case 8: return Solfeggio::DO;
        case 9: return Solfeggio::RE_HIGH;
        default: return Solfeggio::SOL;
    }
}

uint16_t SigilROM::computeChecksum() {
    uint16_t checksum = 0;
    for (uint8_t i = 0; i < SIGIL_COUNT; i++) {
        NeuralSigil sigil;
        readSigil(i, sigil);

        // Simple Fletcher-16 style checksum
        for (uint8_t j = 0; j < sizeof(NeuralSigil); j++) {
            uint8_t byte = reinterpret_cast<uint8_t*>(&sigil)[j];
            checksum += byte;
            checksum += (checksum << 8);
        }
    }
    return checksum;
}

void SigilROM::debugPrintSigil(const NeuralSigil& sigil) {
    Serial.printf("Sigil %d: code=%s freq=%d flags=0x%02X\n",
                  sigil.region_id, sigil.code, sigil.frequency, sigil.flags);

    BreathPattern bp;
    decodeBreathPattern(sigil.breath_pattern, bp);
    Serial.printf("  Breath: %d/%d/%d/%d ms\n",
                  bp.inhale_ms, bp.hold_in_ms, bp.exhale_ms, bp.hold_out_ms);
}

} // namespace UCF
