/**
 * @file omni_linguistics.cpp
 * @brief Implementation of Omni-Linguistics Engine
 */

#include "omni_linguistics.h"
#include <Arduino.h>
#include <string.h>
#include <ctype.h>

namespace UCF {

// POS to APL mapping table
static const APL::Operator POS_TO_APL[] = {
    APL::Operator::GROUP,       // NOUN
    APL::Operator::GROUP,       // PRONOUN
    APL::Operator::SEPARATE,    // VERB
    APL::Operator::AMPLIFY,     // ADJECTIVE
    APL::Operator::AMPLIFY,     // ADVERB
    APL::Operator::FUSION,      // PREPOSITION
    APL::Operator::FUSION,      // CONJUNCTION
    APL::Operator::BOUNDARY,    // DETERMINER
    APL::Operator::BOUNDARY,    // AUXILIARY
    APL::Operator::DECOHERE,    // QUESTION
    APL::Operator::DECOHERE,    // NEGATION
    APL::Operator::GROUP        // UNKNOWN -> default to GROUP
};

// Simple word lookup for POS classification
const OmniLinguistics::WordPOS OmniLinguistics::POS_LOOKUP[] = {
    // Determiners
    {"the", POSTag::DETERMINER},
    {"a", POSTag::DETERMINER},
    {"an", POSTag::DETERMINER},
    {"this", POSTag::DETERMINER},
    {"that", POSTag::DETERMINER},

    // Pronouns
    {"i", POSTag::PRONOUN},
    {"you", POSTag::PRONOUN},
    {"we", POSTag::PRONOUN},
    {"they", POSTag::PRONOUN},
    {"it", POSTag::PRONOUN},

    // Auxiliaries
    {"is", POSTag::AUXILIARY},
    {"are", POSTag::AUXILIARY},
    {"was", POSTag::AUXILIARY},
    {"were", POSTag::AUXILIARY},
    {"be", POSTag::AUXILIARY},
    {"been", POSTag::AUXILIARY},
    {"have", POSTag::AUXILIARY},
    {"has", POSTag::AUXILIARY},
    {"do", POSTag::AUXILIARY},
    {"does", POSTag::AUXILIARY},
    {"can", POSTag::AUXILIARY},
    {"will", POSTag::AUXILIARY},

    // Prepositions
    {"in", POSTag::PREPOSITION},
    {"on", POSTag::PREPOSITION},
    {"at", POSTag::PREPOSITION},
    {"to", POSTag::PREPOSITION},
    {"from", POSTag::PREPOSITION},
    {"with", POSTag::PREPOSITION},
    {"by", POSTag::PREPOSITION},
    {"for", POSTag::PREPOSITION},
    {"of", POSTag::PREPOSITION},

    // Conjunctions
    {"and", POSTag::CONJUNCTION},
    {"or", POSTag::CONJUNCTION},
    {"but", POSTag::CONJUNCTION},
    {"if", POSTag::CONJUNCTION},

    // Question words
    {"what", POSTag::QUESTION},
    {"why", POSTag::QUESTION},
    {"how", POSTag::QUESTION},
    {"when", POSTag::QUESTION},
    {"where", POSTag::QUESTION},
    {"who", POSTag::QUESTION},

    // Negation
    {"not", POSTag::NEGATION},
    {"no", POSTag::NEGATION},
    {"never", POSTag::NEGATION}
};

const uint8_t OmniLinguistics::POS_LOOKUP_SIZE = sizeof(POS_LOOKUP) / sizeof(POS_LOOKUP[0]);

OmniLinguistics::OmniLinguistics()
    : m_z_context(0.5f)
    , m_tier(5)
    , m_history_head(0)
    , m_history_count(0)
{
    memset(&m_pipeline, 0, sizeof(m_pipeline));
    memset(m_history, 0, sizeof(m_history));

    m_pipeline.stage = PipelineStage::ENCODER;
}

bool OmniLinguistics::begin() {
    return true;
}

APLToken OmniLinguistics::processField(const HexFieldState& field) {
    APLToken token;
    memset(&token, 0, sizeof(token));

    token.op = classifyFieldPattern(field);
    token.source_type = static_cast<uint8_t>(InputMode::CAPACITIVE);
    token.intensity = field.total_energy / HEX_SENSOR_COUNT;
    token.z_context = field.z;
    token.tier = z_to_tier(field.z);
    token.timestamp = millis();

    // Check if operator allowed at current tier
    if (!isOperatorAllowed(token.op, field.z)) {
        // Fall back to GROUP which is always allowed
        token.op = APL::Operator::GROUP;
    }

    addToHistory(token);

    return token;
}

APL::Operator OmniLinguistics::classifyFieldPattern(const HexFieldState& field) {
    // Classify based on field characteristics

    // Center-focused = BOUNDARY
    if (field.readings[HEX_CENTER] > 0.7f && field.active_count < 5) {
        return APL::Operator::BOUNDARY;
    }

    // Multiple active sensors spread out = GROUP
    if (field.active_count >= 7) {
        return APL::Operator::GROUP;
    }

    // High energy, expanding = AMPLIFY
    if (field.total_energy > 10.0f && field.r > 0.5f) {
        return APL::Operator::AMPLIFY;
    }

    // Two distinct clusters = SEPARATE
    // (simplified: check if readings are bimodal)
    uint8_t high_count = 0, low_count = 0;
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        if (field.readings[i] > 0.6f) high_count++;
        else if (field.readings[i] < 0.2f) low_count++;
    }
    if (high_count >= 3 && low_count >= 3) {
        return APL::Operator::SEPARATE;
    }

    // Overlapping patterns = FUSION
    if (field.active_count >= 4 && field.r < 0.3f) {
        return APL::Operator::FUSION;
    }

    // Low coherence, scattered = DECOHERE
    if (field.active_count >= 2 && field.total_energy < 3.0f) {
        return APL::Operator::DECOHERE;
    }

    // Default
    return APL::Operator::GROUP;
}

uint8_t OmniLinguistics::processText(const char* text, uint8_t len, APLToken* tokens, uint8_t max_tokens) {
    uint8_t token_count = 0;
    char word[32];
    uint8_t word_len = 0;

    for (uint8_t i = 0; i <= len && token_count < max_tokens; i++) {
        char c = (i < len) ? text[i] : ' ';

        if (isalnum(c)) {
            if (word_len < 31) {
                word[word_len++] = tolower(c);
            }
        } else if (word_len > 0) {
            word[word_len] = '\0';

            // Classify word
            POSTag pos = classifyPOS(word);
            APL::Operator op = posToAPL(pos);

            // Check tier availability
            if (!isOperatorAllowed(op, m_z_context)) {
                op = APL::Operator::GROUP;
            }

            // Create token
            APLToken& t = tokens[token_count++];
            t.op = op;
            t.source_type = static_cast<uint8_t>(InputMode::TEXT);
            t.intensity = 1.0f;
            t.z_context = m_z_context;
            t.tier = m_tier;
            t.timestamp = millis();

            addToHistory(t);
            word_len = 0;
        }
    }

    return token_count;
}

APLToken OmniLinguistics::processAudio(const float* spectrum, uint8_t bins) {
    APLToken token;
    memset(&token, 0, sizeof(token));

    token.source_type = static_cast<uint8_t>(InputMode::AUDIO);
    token.z_context = m_z_context;
    token.tier = m_tier;
    token.timestamp = millis();

    // Simple spectral classification
    // Find dominant frequency band
    float max_energy = 0;
    uint8_t max_bin = 0;
    float total_energy = 0;

    for (uint8_t i = 0; i < bins; i++) {
        total_energy += spectrum[i];
        if (spectrum[i] > max_energy) {
            max_energy = spectrum[i];
            max_bin = i;
        }
    }

    token.intensity = total_energy / bins;

    // Classify based on spectral shape
    float low_energy = 0, high_energy = 0;
    uint8_t mid = bins / 2;
    for (uint8_t i = 0; i < mid; i++) {
        low_energy += spectrum[i];
    }
    for (uint8_t i = mid; i < bins; i++) {
        high_energy += spectrum[i];
    }

    // Low frequencies dominant = grounding = GROUP
    if (low_energy > high_energy * 2) {
        token.op = APL::Operator::GROUP;
    }
    // High frequencies dominant = energizing = AMPLIFY
    else if (high_energy > low_energy * 2) {
        token.op = APL::Operator::AMPLIFY;
    }
    // Peaked spectrum = focused = BOUNDARY
    else if (max_energy > total_energy / bins * 3) {
        token.op = APL::Operator::BOUNDARY;
    }
    // Flat spectrum = diffuse = DECOHERE
    else {
        token.op = APL::Operator::SEPARATE;
    }

    if (!isOperatorAllowed(token.op, m_z_context)) {
        token.op = APL::Operator::GROUP;
    }

    addToHistory(token);

    return token;
}

POSTag OmniLinguistics::classifyPOS(const char* word) {
    // Lookup in table
    for (uint8_t i = 0; i < POS_LOOKUP_SIZE; i++) {
        if (strcmp(word, POS_LOOKUP[i].word) == 0) {
            return POS_LOOKUP[i].pos;
        }
    }

    // Heuristics for unknown words
    size_t len = strlen(word);

    // Words ending in -ly are usually adverbs
    if (len > 2 && strcmp(word + len - 2, "ly") == 0) {
        return POSTag::ADVERB;
    }

    // Words ending in -ing are usually verbs
    if (len > 3 && strcmp(word + len - 3, "ing") == 0) {
        return POSTag::VERB;
    }

    // Words ending in -ed are usually verbs
    if (len > 2 && strcmp(word + len - 2, "ed") == 0) {
        return POSTag::VERB;
    }

    // Words ending in -ness, -tion, -ment are usually nouns
    if ((len > 4 && strcmp(word + len - 4, "ness") == 0) ||
        (len > 4 && strcmp(word + len - 4, "tion") == 0) ||
        (len > 4 && strcmp(word + len - 4, "ment") == 0)) {
        return POSTag::NOUN;
    }

    // Default to noun (most common)
    return POSTag::NOUN;
}

APL::Operator OmniLinguistics::posToAPL(POSTag pos) {
    uint8_t idx = static_cast<uint8_t>(pos);
    if (idx >= 12) idx = 11;  // UNKNOWN
    return POS_TO_APL[idx];
}

bool OmniLinguistics::isOperatorAllowed(APL::Operator op, float z) {
    uint8_t tier = z_to_tier(z);
    return APL::is_operator_allowed(op, tier);
}

PipelineState OmniLinguistics::advancePipeline(const APLToken& input, float z) {
    m_z_context = z;
    m_tier = z_to_tier(z);

    PipelineState state;
    memset(&state, 0, sizeof(state));
    state.token = input;
    state.valid = true;

    // Stage 1: Encoder (already done in process functions)
    state.stage = PipelineStage::ENCODER;
    APLToken encoded = stageEncoder(input);

    // Stage 2: Catalyst - inject phase
    state.stage = PipelineStage::CATALYST;
    APLToken catalyzed = stageCatalyst(encoded);

    // Stage 3: Conductor - routing (pass-through for now)
    state.stage = PipelineStage::CONDUCTOR;

    // Stage 4: Filter - tier gating
    state.stage = PipelineStage::FILTER;
    APLToken filtered = stageFilter(catalyzed);
    if (filtered.intensity < 0.01f) {
        state.valid = false;
    }

    // Stage 5: Oscillator - timing
    state.stage = PipelineStage::OSCILLATOR;
    APLToken oscillated = stageOscillator(filtered);

    // Stage 6: Reactor - combine with previous
    state.stage = PipelineStage::REACTOR;
    APLToken previous;
    if (m_history_count > 0) {
        uint8_t prev_idx = (m_history_head + HISTORY_SIZE - 1) % HISTORY_SIZE;
        previous = m_history[prev_idx];
    } else {
        memset(&previous, 0, sizeof(previous));
        previous.op = APL::Operator::GROUP;
    }
    APLToken reacted = stageReactor(oscillated, previous);

    // Stage 7: Dynamo - amplification
    state.stage = PipelineStage::DYNAMO;
    reacted.intensity *= (1.0f + z * 0.5f);  // Boost with z
    if (reacted.intensity > 1.0f) reacted.intensity = 1.0f;

    // Stage 8: Decoder - map to output
    state.stage = PipelineStage::DECODER;
    stageDecoder(reacted, state);

    // Stage 9: Regenerator - feedback
    state.stage = PipelineStage::REGENERATOR;
    state.token = reacted;

    m_pipeline = state;

    return state;
}

APLToken OmniLinguistics::stageEncoder(const APLToken& input) {
    // Pass-through: encoding already done
    return input;
}

APLToken OmniLinguistics::stageCatalyst(const APLToken& token) {
    APLToken result = token;

    // Inject phase information into intensity
    Phase phase = z_to_phase(token.z_context);
    switch (phase) {
        case Phase::UNTRUE:
            result.intensity *= 0.7f;  // Reduce intensity in UNTRUE
            break;
        case Phase::PARADOX:
            // No modification in PARADOX
            break;
        case Phase::TRUE:
            result.intensity *= 1.2f;  // Boost in TRUE
            if (result.intensity > 1.0f) result.intensity = 1.0f;
            break;
    }

    return result;
}

APLToken OmniLinguistics::stageFilter(const APLToken& token) {
    APLToken result = token;

    // Check operator availability
    if (!isOperatorAllowed(token.op, token.z_context)) {
        result.intensity = 0.0f;  // Block this token
    }

    return result;
}

APLToken OmniLinguistics::stageOscillator(const APLToken& token) {
    APLToken result = token;

    // Apply timing modulation based on tier
    // Higher tiers = faster rhythm
    float rhythm_mult = 1.0f + (token.tier - 5) * 0.1f;
    result.intensity *= rhythm_mult;
    if (result.intensity > 1.0f) result.intensity = 1.0f;

    return result;
}

APLToken OmniLinguistics::stageReactor(const APLToken& current, const APLToken& previous) {
    APLToken result = current;

    // Combine operators: certain combinations create emergent operators
    // GROUP + GROUP = stronger GROUP
    // SEPARATE + FUSION = BOUNDARY
    // AMPLIFY + DECOHERE = SEPARATE

    if (previous.op == current.op) {
        // Same operator: reinforce
        result.intensity = (current.intensity + previous.intensity) / 2.0f * 1.2f;
    } else if (previous.op == APL::Operator::SEPARATE && current.op == APL::Operator::FUSION) {
        result.op = APL::Operator::BOUNDARY;
        result.intensity = (current.intensity + previous.intensity) / 2.0f;
    } else if (previous.op == APL::Operator::AMPLIFY && current.op == APL::Operator::DECOHERE) {
        result.op = APL::Operator::SEPARATE;
        result.intensity = (current.intensity + previous.intensity) / 2.0f;
    }

    if (result.intensity > 1.0f) result.intensity = 1.0f;

    return result;
}

void OmniLinguistics::stageDecoder(const APLToken& token, PipelineState& state) {
    // Map operator to output frequency and color
    state.frequency = tierToSolfeggio(token.tier);
    state.amplitude = token.intensity;

    // Operator-specific color modulation
    switch (token.op) {
        case APL::Operator::GROUP:
            // Earth tones: brown/green
            state.rgb[0] = 139; state.rgb[1] = 119; state.rgb[2] = 101;
            break;
        case APL::Operator::BOUNDARY:
            // White/silver
            state.rgb[0] = 220; state.rgb[1] = 220; state.rgb[2] = 220;
            break;
        case APL::Operator::AMPLIFY:
            // Gold/yellow
            state.rgb[0] = 255; state.rgb[1] = 215; state.rgb[2] = 0;
            break;
        case APL::Operator::SEPARATE:
            // Red/orange
            state.rgb[0] = 255; state.rgb[1] = 99; state.rgb[2] = 71;
            break;
        case APL::Operator::FUSION:
            // Blue/purple
            state.rgb[0] = 138; state.rgb[1] = 43; state.rgb[2] = 226;
            break;
        case APL::Operator::DECOHERE:
            // Gray/muted
            state.rgb[0] = 128; state.rgb[1] = 128; state.rgb[2] = 128;
            break;
        default:
            state.rgb[0] = 255; state.rgb[1] = 255; state.rgb[2] = 255;
            break;
    }

    // Modulate brightness by intensity
    for (uint8_t i = 0; i < 3; i++) {
        state.rgb[i] = static_cast<uint8_t>(state.rgb[i] * token.intensity);
    }
}

CoherenceMetrics OmniLinguistics::measureCoherence() {
    CoherenceMetrics metrics;
    memset(&metrics, 0, sizeof(metrics));

    if (m_history_count < 2) {
        metrics.kappa_s = 1.0f;
        metrics.kappa_g = 1.0f;
        metrics.is_coherent = true;
        return metrics;
    }

    // Count adjacent mismatches
    uint8_t mismatches = 0;
    uint8_t count = (m_history_count < HISTORY_SIZE) ? m_history_count : HISTORY_SIZE;

    for (uint8_t i = 1; i < count; i++) {
        uint8_t prev_idx = (m_history_head + HISTORY_SIZE - i) % HISTORY_SIZE;
        uint8_t curr_idx = (m_history_head + HISTORY_SIZE - i + 1) % HISTORY_SIZE;

        if (m_history[prev_idx].op != m_history[curr_idx].op) {
            mismatches++;
        }
    }

    metrics.mismatch_count = mismatches;

    // Symbol-level coherence: proportion of matching adjacent pairs
    metrics.kappa_s = 1.0f - static_cast<float>(mismatches) / (count - 1);

    // Global coherence: check if all tokens within similar tier range
    uint8_t min_tier = 9, max_tier = 1;
    for (uint8_t i = 0; i < count; i++) {
        uint8_t idx = (m_history_head + HISTORY_SIZE - i - 1) % HISTORY_SIZE;
        if (m_history[idx].tier < min_tier) min_tier = m_history[idx].tier;
        if (m_history[idx].tier > max_tier) max_tier = m_history[idx].tier;
    }
    metrics.kappa_g = 1.0f - static_cast<float>(max_tier - min_tier) / 8.0f;

    // Overall coherence threshold (κ_s ≈ 0.92)
    metrics.is_coherent = (metrics.kappa_s >= 0.92f);

    return metrics;
}

void OmniLinguistics::resetPipeline() {
    memset(&m_pipeline, 0, sizeof(m_pipeline));
    m_pipeline.stage = PipelineStage::ENCODER;
    m_history_count = 0;
    m_history_head = 0;
}

void OmniLinguistics::addToHistory(const APLToken& token) {
    m_history[m_history_head] = token;
    m_history_head = (m_history_head + 1) % HISTORY_SIZE;
    if (m_history_count < HISTORY_SIZE) {
        m_history_count++;
    }
}

uint8_t OmniLinguistics::getHistory(APLToken* buffer, uint8_t maxTokens) {
    uint8_t count = (maxTokens < m_history_count) ? maxTokens : m_history_count;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t idx = (m_history_head + HISTORY_SIZE - count + i) % HISTORY_SIZE;
        buffer[i] = m_history[idx];
    }

    return count;
}

void OmniLinguistics::setZContext(float z) {
    m_z_context = z;
    m_tier = z_to_tier(z);
}

char OmniLinguistics::operatorToSymbol(APL::Operator op) {
    switch (op) {
        case APL::Operator::GROUP:    return '+';
        case APL::Operator::BOUNDARY: return 'O';  // () represented as O
        case APL::Operator::AMPLIFY:  return '^';
        case APL::Operator::SEPARATE: return '-';
        case APL::Operator::FUSION:   return 'x';
        case APL::Operator::DECOHERE: return '/';  // ÷ represented as /
        default: return '?';
    }
}

APL::Operator OmniLinguistics::symbolToOperator(char symbol) {
    switch (symbol) {
        case '+': return APL::Operator::GROUP;
        case 'O':
        case '(': return APL::Operator::BOUNDARY;
        case '^': return APL::Operator::AMPLIFY;
        case '-': return APL::Operator::SEPARATE;
        case 'x':
        case '*': return APL::Operator::FUSION;
        case '/': return APL::Operator::DECOHERE;
        default:  return APL::Operator::GROUP;
    }
}

const char* posToString(POSTag pos) {
    switch (pos) {
        case POSTag::NOUN:        return "NOUN";
        case POSTag::PRONOUN:     return "PRONOUN";
        case POSTag::VERB:        return "VERB";
        case POSTag::ADJECTIVE:   return "ADJECTIVE";
        case POSTag::ADVERB:      return "ADVERB";
        case POSTag::PREPOSITION: return "PREPOSITION";
        case POSTag::CONJUNCTION: return "CONJUNCTION";
        case POSTag::DETERMINER:  return "DETERMINER";
        case POSTag::AUXILIARY:   return "AUXILIARY";
        case POSTag::QUESTION:    return "QUESTION";
        case POSTag::NEGATION:    return "NEGATION";
        default:                  return "UNKNOWN";
    }
}

const char* aplOperatorToString(APL::Operator op) {
    switch (op) {
        case APL::Operator::GROUP:    return "GROUP (+)";
        case APL::Operator::BOUNDARY: return "BOUNDARY (O)";
        case APL::Operator::AMPLIFY:  return "AMPLIFY (^)";
        case APL::Operator::SEPARATE: return "SEPARATE (-)";
        case APL::Operator::FUSION:   return "FUSION (x)";
        case APL::Operator::DECOHERE: return "DECOHERE (/)";
        default:                       return "UNKNOWN";
    }
}

// Utility function from header
uint16_t tierToSolfeggio(uint8_t tier) {
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

} // namespace UCF
