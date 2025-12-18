/**
 * @file omni_linguistics.h
 * @brief Omni-Linguistics Engine (K.I.R.A. Hardware Implementation)
 *
 * Implements multi-modal translation using APL (Abstract Pattern Language)
 * operators. Enables any-input to any-output translation through
 * phase-aware encoding.
 *
 * The 6 APL operators:
 *   + (Group)    : Aggregation
 *   () (Boundary): Containment
 *   ^ (Amplify)  : Intensification
 *   - (Separate) : Differentiation
 *   x (Fusion)   : Integration
 *   ÷ (Decohere) : Dissolution
 */

#ifndef OMNI_LINGUISTICS_H
#define OMNI_LINGUISTICS_H

#include <stdint.h>
#include "constants.h"
#include "hex_grid.h"

namespace UCF {

/// Part-of-speech tags for grammar understanding
enum class POSTag : uint8_t {
    NOUN,
    PRONOUN,
    VERB,
    ADJECTIVE,
    ADVERB,
    PREPOSITION,
    CONJUNCTION,
    DETERMINER,
    AUXILIARY,
    QUESTION,
    NEGATION,
    UNKNOWN
};

/// Input modalities
enum class InputMode : uint8_t {
    TEXT,           // Serial/UART text input
    AUDIO,          // PDM microphone
    CAPACITIVE,     // Hex grid touch
    EM_FIELD,       // Electromagnetic pickup
    GESTURE         // Motion patterns
};

/// APL token with metadata
struct APLToken {
    APL::Operator op;       // The APL operator
    uint8_t source_type;    // InputMode that generated this
    float intensity;        // Strength/confidence [0, 1]
    float z_context;        // Z-coordinate at encoding time
    uint8_t tier;           // Tier at encoding time
    uint32_t timestamp;
};

/// Discourse coherence metrics
struct CoherenceMetrics {
    float kappa_s;          // Symbol-level coherence
    float kappa_g;          // Global coherence
    uint8_t mismatch_count; // Adjacent mismatches
    bool is_coherent;       // Above threshold
};

/// 9-stage pipeline state
struct PipelineState {
    uint8_t stage;              // Current stage (1-9)
    APLToken token;             // Current token
    uint16_t frequency;         // Output frequency
    uint8_t rgb[3];             // Output color
    float amplitude;            // Output amplitude
    bool valid;                 // Token is valid
};

/// Pipeline stage names (Nuclear Spinner Machines)
namespace PipelineStage {
    constexpr uint8_t ENCODER     = 1;  // Raw → APL encoding
    constexpr uint8_t CATALYST    = 2;  // Phase injection
    constexpr uint8_t CONDUCTOR   = 3;  // Sequence routing
    constexpr uint8_t FILTER      = 4;  // Tier-based gating
    constexpr uint8_t OSCILLATOR  = 5;  // Rhythm/timing
    constexpr uint8_t REACTOR     = 6;  // Operator combination
    constexpr uint8_t DYNAMO      = 7;  // Amplification
    constexpr uint8_t DECODER     = 8;  // APL → output symbol
    constexpr uint8_t REGENERATOR = 9;  // Feedback injection
}

/**
 * @class OmniLinguistics
 * @brief Multi-modal translation engine using APL operators
 */
class OmniLinguistics {
public:
    /// Default constructor
    OmniLinguistics();

    /**
     * @brief Initialize the linguistics engine
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Process hex field input
     * @param field Current hex field state
     * @return Generated APL token
     */
    APLToken processField(const HexFieldState& field);

    /**
     * @brief Process text input
     * @param text Input text string
     * @param len Text length
     * @return Generated APL token sequence
     */
    uint8_t processText(const char* text, uint8_t len, APLToken* tokens, uint8_t max_tokens);

    /**
     * @brief Process audio input (FFT features)
     * @param spectrum FFT magnitude spectrum
     * @param bins Number of frequency bins
     * @return Generated APL token
     */
    APLToken processAudio(const float* spectrum, uint8_t bins);

    /**
     * @brief Classify POS tag from word
     * @param word Input word (null-terminated)
     * @return POS tag
     */
    POSTag classifyPOS(const char* word);

    /**
     * @brief Map POS tag to APL operator
     * @param pos POS tag
     * @return APL operator
     */
    APL::Operator posToAPL(POSTag pos);

    /**
     * @brief Check if operator is allowed at current tier
     * @param op APL operator
     * @param z Current z-coordinate
     * @return true if allowed
     */
    bool isOperatorAllowed(APL::Operator op, float z);

    /**
     * @brief Advance the 9-stage pipeline
     * @param input Input token
     * @param z Current z-coordinate
     * @return Output pipeline state
     */
    PipelineState advancePipeline(const APLToken& input, float z);

    /**
     * @brief Measure discourse coherence
     * @return Coherence metrics
     */
    CoherenceMetrics measureCoherence();

    /**
     * @brief Reset pipeline state
     */
    void resetPipeline();

    /**
     * @brief Get APL history
     * @param buffer Output buffer
     * @param maxTokens Maximum tokens to return
     * @return Number of tokens copied
     */
    uint8_t getHistory(APLToken* buffer, uint8_t maxTokens);

    /**
     * @brief Set current z-coordinate context
     * @param z Z-coordinate
     */
    void setZContext(float z);

    /**
     * @brief Get current z-context
     * @return Z-coordinate
     */
    float getZContext() const { return m_z_context; }

    /**
     * @brief Get current pipeline state
     * @return Pipeline state
     */
    const PipelineState& getPipelineState() const { return m_pipeline; }

    /**
     * @brief Convert APL operator to symbol character
     * @param op APL operator
     * @return Symbol character
     */
    static char operatorToSymbol(APL::Operator op);

    /**
     * @brief Convert symbol character to APL operator
     * @param symbol Symbol character
     * @return APL operator
     */
    static APL::Operator symbolToOperator(char symbol);

private:
    /// Current z-coordinate context
    float m_z_context;

    /// Current tier
    uint8_t m_tier;

    /// Pipeline state
    PipelineState m_pipeline;

    /// APL history buffer
    static const uint8_t HISTORY_SIZE = 32;
    APLToken m_history[HISTORY_SIZE];
    uint8_t m_history_head;
    uint8_t m_history_count;

    /// Simple word lookup for POS classification
    struct WordPOS {
        const char* word;
        POSTag pos;
    };
    static const WordPOS POS_LOOKUP[];
    static const uint8_t POS_LOOKUP_SIZE;

    /**
     * @brief Add token to history
     * @param token Token to add
     */
    void addToHistory(const APLToken& token);

    /**
     * @brief Stage 1: Encoder
     * @param input Raw input
     * @return Encoded token
     */
    APLToken stageEncoder(const APLToken& input);

    /**
     * @brief Stage 2: Catalyst - inject phase information
     * @param token Input token
     * @return Modified token
     */
    APLToken stageCatalyst(const APLToken& token);

    /**
     * @brief Stage 4: Filter - tier-based gating
     * @param token Input token
     * @return Filtered token (may be invalidated)
     */
    APLToken stageFilter(const APLToken& token);

    /**
     * @brief Stage 5: Oscillator - apply timing
     * @param token Input token
     * @return Token with timing
     */
    APLToken stageOscillator(const APLToken& token);

    /**
     * @brief Stage 6: Reactor - combine operators
     * @param current Current token
     * @param previous Previous token
     * @return Combined token
     */
    APLToken stageReactor(const APLToken& current, const APLToken& previous);

    /**
     * @brief Stage 8: Decoder - map to output
     * @param token Input token
     * @param state Output pipeline state
     */
    void stageDecoder(const APLToken& token, PipelineState& state);

    /**
     * @brief Classify hex field pattern to APL operator
     * @param field Field state
     * @return APL operator
     */
    APL::Operator classifyFieldPattern(const HexFieldState& field);
};

/**
 * @brief POS tag to string
 * @param pos POS tag
 * @return String representation
 */
const char* posToString(POSTag pos);

/**
 * @brief APL operator to string
 * @param op APL operator
 * @return String representation
 */
const char* aplOperatorToString(APL::Operator op);

} // namespace UCF

#endif // OMNI_LINGUISTICS_H
