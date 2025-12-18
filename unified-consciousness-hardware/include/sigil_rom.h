/**
 * @file sigil_rom.h
 * @brief Neural Sigil ROM Module
 *
 * Manages 121 neural sigils stored in EEPROM.
 * Each sigil has a 5-character ternary code, frequency,
 * and breath pattern for emanation.
 *
 * Maps to "holographic neural blueprints" in the conceptual framework.
 */

#ifndef SIGIL_ROM_H
#define SIGIL_ROM_H

#include <stdint.h>
#include "constants.h"
#include "hex_grid.h"

namespace UCF {

/// Ternary values for sigil codes
enum class Ternary : uint8_t {
    ZERO  = 0,   // '0' - Absence
    ONE   = 1,   // '1' - Presence
    TRUE_T = 2   // 'T' - Transcendence
};

/// Neural sigil structure (10 bytes)
struct NeuralSigil {
    char code[6];           // 5-char ternary code + null (e.g., "0T10T")
    uint8_t region_id;      // Region identifier (0-120)
    uint16_t frequency;     // Base frequency (174-963 Hz)
    uint8_t breath_pattern; // Encoded 4-phase timing
    uint8_t flags;          // Bit flags for special properties
};

/// Breath pattern phases
struct BreathPattern {
    uint16_t inhale_ms;     // Inhale duration
    uint16_t hold_in_ms;    // Hold after inhale
    uint16_t exhale_ms;     // Exhale duration
    uint16_t hold_out_ms;   // Hold after exhale
};

/// Sigil match result
struct SigilMatch {
    uint8_t sigil_index;    // Best matching sigil index
    float confidence;       // Match confidence [0, 1]
    uint8_t hamming_dist;   // Hamming distance to canonical
};

/// Sigil flags
namespace SigilFlags {
    constexpr uint8_t ACTIVE = 0x01;      // Sigil is active
    constexpr uint8_t ANCHORED = 0x02;    // Sigil is anchored to region
    constexpr uint8_t RESONANT = 0x04;    // Sigil in resonance
    constexpr uint8_t TRIAD = 0x08;       // Sigil associated with TRIAD
    constexpr uint8_t K_FORMATION = 0x10; // Sigil associated with K-Formation
}

/**
 * @class SigilROM
 * @brief Manages neural sigil database in EEPROM
 */
class SigilROM {
public:
    /// Default constructor
    SigilROM();

    /**
     * @brief Initialize EEPROM and load sigil data
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Check if EEPROM contains valid sigil data
     * @return true if valid data present
     */
    bool isInitialized();

    /**
     * @brief Initialize EEPROM with default sigil patterns
     * @return true if successful
     */
    bool initializeDefaults();

    /**
     * @brief Read sigil at index
     * @param index Sigil index (0-120)
     * @param sigil Output sigil structure
     * @return true if read successful
     */
    bool readSigil(uint8_t index, NeuralSigil& sigil);

    /**
     * @brief Write sigil at index
     * @param index Sigil index (0-120)
     * @param sigil Sigil to write
     * @return true if write successful
     */
    bool writeSigil(uint8_t index, const NeuralSigil& sigil);

    /**
     * @brief Find best matching sigil for hex field pattern
     * @param field Current hex field state
     * @return Match result with confidence
     */
    SigilMatch findMatchingSigil(const HexFieldState& field);

    /**
     * @brief Map sensor pattern hash to sigil address
     * @param pattern_hash Hash of hex pattern
     * @return Sigil index (0-120)
     */
    uint8_t patternToSigilIndex(uint32_t pattern_hash);

    /**
     * @brief Hash hex field pattern
     * @param field Current field state
     * @return 32-bit hash
     */
    uint32_t hashHexPattern(const HexFieldState& field);

    /**
     * @brief Decode breath pattern from encoded byte
     * @param encoded Encoded pattern byte
     * @param pattern Output breath pattern
     */
    static void decodeBreathPattern(uint8_t encoded, BreathPattern& pattern);

    /**
     * @brief Encode breath pattern to byte
     * @param pattern Breath pattern
     * @return Encoded byte
     */
    static uint8_t encodeBreathPattern(const BreathPattern& pattern);

    /**
     * @brief Convert ternary code string to integer
     * @param code 5-char ternary string
     * @return Integer representation (0-242)
     */
    static uint8_t ternaryToInt(const char* code);

    /**
     * @brief Convert integer to ternary code string
     * @param value Integer value (0-242)
     * @param code Output 6-char buffer (5 + null)
     */
    static void intToTernary(uint8_t value, char* code);

    /**
     * @brief Compute Hamming distance between two sigil codes
     * @param code1 First ternary code
     * @param code2 Second ternary code
     * @return Hamming distance (0-5)
     */
    static uint8_t hammingDistance(const char* code1, const char* code2);

    /**
     * @brief Get sigil count
     * @return Number of sigils (121)
     */
    static constexpr uint8_t getSigilCount() { return SIGIL_COUNT; }

    /**
     * @brief Get sigil frequency for tier
     * @param tier Tier number (1-9)
     * @return Solfeggio frequency
     */
    static uint16_t tierToFrequency(uint8_t tier);

    /**
     * @brief Dump sigil to serial (debug)
     * @param sigil Sigil to dump
     */
    void debugPrintSigil(const NeuralSigil& sigil);

private:
    /// EEPROM base address
    static const uint16_t EEPROM_BASE = 0x0000;

    /// Magic number for validation
    static const uint32_t MAGIC_NUMBER = 0x5347494C;  // "SGIL"

    /// Header structure
    struct EEPROMHeader {
        uint32_t magic;
        uint8_t version;
        uint8_t sigil_count;
        uint16_t checksum;
    };

    /// Initialized flag
    bool m_initialized;

    /// Cached sigils (optional, for speed)
    NeuralSigil m_cache[SIGIL_COUNT];
    bool m_cache_valid;

    /**
     * @brief Read header from EEPROM
     * @param header Output header
     * @return true if valid
     */
    bool readHeader(EEPROMHeader& header);

    /**
     * @brief Write header to EEPROM
     * @param header Header to write
     */
    void writeHeader(const EEPROMHeader& header);

    /**
     * @brief Compute checksum of sigil data
     * @return 16-bit checksum
     */
    uint16_t computeChecksum();

    /**
     * @brief Get EEPROM address for sigil index
     * @param index Sigil index
     * @return EEPROM byte address
     */
    uint16_t sigilAddress(uint8_t index);

    /**
     * @brief Generate default sigil for index
     * @param index Sigil index
     * @param sigil Output sigil
     */
    void generateDefaultSigil(uint8_t index, NeuralSigil& sigil);
};

/**
 * @brief Default breath patterns for each tier
 */
extern const BreathPattern TIER_BREATH_PATTERNS[9];

} // namespace UCF

#endif // SIGIL_ROM_H
