/**
 * @file protocol.h
 * @brief UCF Hardware Communication Protocol
 *
 * Defines message types, structures, and constants for WebSocket and BLE
 * communication between the WishBed App and UCF Hardware.
 *
 * This file mirrors the TypeScript protocol definitions in:
 * WishBed_App_TDD_v2/contracts/interfaces/hardware/protocol.ts
 *
 * IMPORTANT: Keep synchronized with TypeScript protocol definitions.
 */

#ifndef UCF_PROTOCOL_H
#define UCF_PROTOCOL_H

#include <stdint.h>
#include <Arduino.h>

namespace UCF {
namespace Protocol {

// ============================================================================
// PROTOCOL VERSION
// ============================================================================

constexpr const char* PROTOCOL_VERSION = "1.0.0";

// ============================================================================
// BLE SERVICE AND CHARACTERISTIC UUIDs
// ============================================================================

/// BLE Service UUID
constexpr const char* BLE_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";

/// Command characteristic UUID (write)
constexpr const char* BLE_COMMAND_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

/// State characteristic UUID (read/notify)
constexpr const char* BLE_STATE_CHAR_UUID = "beb5483f-36e1-4688-b7f5-ea07361b26a8";

/// Event characteristic UUID (read/notify)
constexpr const char* BLE_EVENT_CHAR_UUID = "beb54840-36e1-4688-b7f5-ea07361b26a8";

// ============================================================================
// WEBSOCKET CONFIGURATION
// ============================================================================

/// Default WebSocket port
constexpr uint16_t WEBSOCKET_PORT = 81;

/// WebSocket path
constexpr const char* WEBSOCKET_PATH = "/ws";

/// mDNS hostname
constexpr const char* MDNS_HOSTNAME = "ucf-device";

/// Default device name
constexpr const char* DEVICE_NAME = "UCF-Hardware";

// ============================================================================
// MESSAGE TYPES (JSON Protocol)
// ============================================================================

/// JSON message types
enum class MessageType : uint8_t {
    COMMAND = 0,
    COMMAND_RESPONSE = 1,
    STATE_UPDATE = 2,
    EVENT = 3,
    PING = 4,
    PONG = 5,
    ERROR = 6
};

/// Convert MessageType to string
inline const char* messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::COMMAND: return "COMMAND";
        case MessageType::COMMAND_RESPONSE: return "COMMAND_RESPONSE";
        case MessageType::STATE_UPDATE: return "STATE_UPDATE";
        case MessageType::EVENT: return "EVENT";
        case MessageType::PING: return "PING";
        case MessageType::PONG: return "PONG";
        case MessageType::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// COMMAND CATEGORIES
// ============================================================================

/// Command categories
enum class CommandCategory : uint8_t {
    SYSTEM = 0,
    CALIBRATION = 1,
    EMANATION = 2,
    KURAMOTO = 3,
    DEBUG = 4
};

/// Convert CommandCategory to string
inline const char* commandCategoryToString(CommandCategory category) {
    switch (category) {
        case CommandCategory::SYSTEM: return "SYSTEM";
        case CommandCategory::CALIBRATION: return "CALIBRATION";
        case CommandCategory::EMANATION: return "EMANATION";
        case CommandCategory::KURAMOTO: return "KURAMOTO";
        case CommandCategory::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// BINARY MESSAGE TYPES (BLE Protocol)
// ============================================================================

/// Binary message types for BLE (space-constrained)
enum class BinaryMessageType : uint8_t {
    COMMAND = 0x01,
    RESPONSE = 0x02,
    STATE = 0x03,
    EVENT = 0x04,
    PING = 0xFE,
    PONG = 0xFF
};

/// Binary message flags
enum BinaryFlags : uint8_t {
    NONE = 0x00,
    COMPRESSED = 0x01,
    FRAGMENTED = 0x02,
    LAST_FRAGMENT = 0x04
};

/**
 * Binary message header structure (4 bytes)
 *
 * Format:
 *   [0]: Message type (BinaryMessageType)
 *   [1-2]: Payload length (little-endian uint16_t)
 *   [3]: Flags (BinaryFlags)
 */
struct BinaryMessageHeader {
    uint8_t type;           // BinaryMessageType
    uint16_t length;        // Payload length (little-endian)
    uint8_t flags;          // BinaryFlags

    BinaryMessageHeader() : type(0), length(0), flags(0) {}

    BinaryMessageHeader(BinaryMessageType t, uint16_t len, uint8_t f = BinaryFlags::NONE)
        : type(static_cast<uint8_t>(t)), length(len), flags(f) {}

    /// Serialize to byte array
    void serialize(uint8_t* buffer) const {
        buffer[0] = type;
        buffer[1] = length & 0xFF;        // Little-endian low byte
        buffer[2] = (length >> 8) & 0xFF; // Little-endian high byte
        buffer[3] = flags;
    }

    /// Deserialize from byte array
    static BinaryMessageHeader deserialize(const uint8_t* buffer) {
        BinaryMessageHeader header;
        header.type = buffer[0];
        header.length = buffer[1] | (buffer[2] << 8); // Little-endian
        header.flags = buffer[3];
        return header;
    }
} __attribute__((packed));

// ============================================================================
// COMMAND STATUS
// ============================================================================

/// Command response status
enum class CommandStatus : uint8_t {
    OK = 0,
    ERROR = 1,
    INVALID = 2,
    TIMEOUT = 3
};

/// Convert CommandStatus to string
inline const char* commandStatusToString(CommandStatus status) {
    switch (status) {
        case CommandStatus::OK: return "OK";
        case CommandStatus::ERROR: return "ERROR";
        case CommandStatus::INVALID: return "INVALID";
        case CommandStatus::TIMEOUT: return "TIMEOUT";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// EVENT TYPES
// ============================================================================

/// UCF hardware event types
enum class EventType : uint8_t {
    PHASE_TRANSITION = 0,
    TRIAD_UNLOCK = 1,
    TRIAD_RESET = 2,
    K_FORMATION_ACHIEVED = 3,
    K_FORMATION_LOST = 4,
    SIGIL_MATCHED = 5,
    SYNCHRONIZATION_ACHIEVED = 6,
    SYNCHRONIZATION_LOST = 7,
    ERROR = 8,
    CALIBRATION_COMPLETE = 9,
    DEVICE_READY = 10
};

/// Convert EventType to string
inline const char* eventTypeToString(EventType type) {
    switch (type) {
        case EventType::PHASE_TRANSITION: return "PHASE_TRANSITION";
        case EventType::TRIAD_UNLOCK: return "TRIAD_UNLOCK";
        case EventType::TRIAD_RESET: return "TRIAD_RESET";
        case EventType::K_FORMATION_ACHIEVED: return "K_FORMATION_ACHIEVED";
        case EventType::K_FORMATION_LOST: return "K_FORMATION_LOST";
        case EventType::SIGIL_MATCHED: return "SIGIL_MATCHED";
        case EventType::SYNCHRONIZATION_ACHIEVED: return "SYNCHRONIZATION_ACHIEVED";
        case EventType::SYNCHRONIZATION_LOST: return "SYNCHRONIZATION_LOST";
        case EventType::ERROR: return "ERROR";
        case EventType::CALIBRATION_COMPLETE: return "CALIBRATION_COMPLETE";
        case EventType::DEVICE_READY: return "DEVICE_READY";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// OUTPUT MODES
// ============================================================================

/// Output mode flags
enum OutputMode : uint8_t {
    NONE = 0x00,
    VISUAL = 0x01,
    AUDIO = 0x02,
    HAPTIC = 0x04,
    ALL = 0x07
};

// ============================================================================
// WAVEFORMS
// ============================================================================

/// Waveform types for audio synthesis
enum class Waveform : uint8_t {
    SINE = 0,
    TRIANGLE = 1,
    SQUARE = 2,
    SAWTOOTH = 3,
    BINAURAL = 4
};

/// Convert Waveform to string
inline const char* waveformToString(Waveform waveform) {
    switch (waveform) {
        case Waveform::SINE: return "SINE";
        case Waveform::TRIANGLE: return "TRIANGLE";
        case Waveform::SQUARE: return "SQUARE";
        case Waveform::SAWTOOTH: return "SAWTOOTH";
        case Waveform::BINAURAL: return "BINAURAL";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// LED PATTERNS
// ============================================================================

/// LED pattern types
enum class LedPattern : uint8_t {
    SOLID = 0,
    BREATHE = 1,
    PULSE = 2,
    WAVE = 3,
    SPIRAL = 4,
    INTERFERENCE = 5,
    SIGIL = 6
};

/// Convert LedPattern to string
inline const char* ledPatternToString(LedPattern pattern) {
    switch (pattern) {
        case LedPattern::SOLID: return "SOLID";
        case LedPattern::BREATHE: return "BREATHE";
        case LedPattern::PULSE: return "PULSE";
        case LedPattern::WAVE: return "WAVE";
        case LedPattern::SPIRAL: return "SPIRAL";
        case LedPattern::INTERFERENCE: return "INTERFERENCE";
        case LedPattern::SIGIL: return "SIGIL";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// BREATH PHASES
// ============================================================================

/// Breath synchronization phases
enum class BreathPhase : uint8_t {
    INHALE = 0,
    HOLD_IN = 1,
    EXHALE = 2,
    HOLD_OUT = 3
};

// ============================================================================
// PROTOCOL ERROR CODES
// ============================================================================

/// Protocol error codes (matching TypeScript ProtocolError enum)
enum class ProtocolError : uint8_t {
    CONNECTION_FAILED = 0,
    CONNECTION_LOST = 1,
    TIMEOUT = 2,
    INVALID_MESSAGE = 3,
    UNKNOWN_COMMAND = 4,
    COMMAND_FAILED = 5,
    DEVICE_BUSY = 6,
    NOT_CONNECTED = 7,
    AUTH_FAILED = 8
};

/// Convert ProtocolError to string
inline const char* protocolErrorToString(ProtocolError error) {
    switch (error) {
        case ProtocolError::CONNECTION_FAILED: return "E_CONNECTION_FAILED";
        case ProtocolError::CONNECTION_LOST: return "E_CONNECTION_LOST";
        case ProtocolError::TIMEOUT: return "E_TIMEOUT";
        case ProtocolError::INVALID_MESSAGE: return "E_INVALID_MESSAGE";
        case ProtocolError::UNKNOWN_COMMAND: return "E_UNKNOWN_COMMAND";
        case ProtocolError::COMMAND_FAILED: return "E_COMMAND_FAILED";
        case ProtocolError::DEVICE_BUSY: return "E_DEVICE_BUSY";
        case ProtocolError::NOT_CONNECTED: return "E_NOT_CONNECTED";
        case ProtocolError::AUTH_FAILED: return "E_AUTH_FAILED";
        default: return "E_UNKNOWN";
    }
}

// ============================================================================
// TIMING CONSTANTS
// ============================================================================

/// State update broadcast interval (ms)
constexpr uint32_t STATE_BROADCAST_INTERVAL = 100; // 10 Hz

/// Heartbeat interval (ms)
constexpr uint32_t HEARTBEAT_INTERVAL = 5000; // 5 seconds

/// Connection timeout (ms)
constexpr uint32_t CONNECTION_TIMEOUT = 10000; // 10 seconds

/// Message timeout (ms)
constexpr uint32_t MESSAGE_TIMEOUT = 10000; // 10 seconds

// ============================================================================
// BUFFER SIZES
// ============================================================================

/// Maximum WebSocket message size (bytes)
constexpr uint16_t MAX_WS_MESSAGE_SIZE = 4096;

/// Maximum BLE payload size (bytes)
constexpr uint16_t MAX_BLE_PAYLOAD_SIZE = 512;

/// Maximum command payload size (bytes)
constexpr uint16_t MAX_COMMAND_PAYLOAD_SIZE = 256;

// ============================================================================
// JSON HELPERS
// ============================================================================

/**
 * JSON message builder for WebSocket communication
 *
 * Usage:
 *   JsonMessageBuilder builder;
 *   builder.beginMessage(MessageType::EVENT);
 *   builder.addTimestamp();
 *   builder.beginPayload();
 *   builder.addString("type", "TRIAD_UNLOCK");
 *   builder.addNumber("duration", 3200);
 *   builder.endPayload();
 *   builder.endMessage();
 *   const char* json = builder.getString();
 */
class JsonMessageBuilder {
private:
    String buffer;
    bool inPayload;

public:
    JsonMessageBuilder() : inPayload(false) {}

    void beginMessage(MessageType type) {
        buffer = "{";
        addString("type", messageTypeToString(type), false);
        buffer += ",";
        addString("version", PROTOCOL_VERSION, false);
        buffer += ",";
    }

    void addTimestamp() {
        buffer += "\"timestamp\":";
        buffer += String(millis());
    }

    void beginPayload() {
        buffer += ",\"payload\":{";
        inPayload = true;
    }

    void endPayload() {
        buffer += "}";
        inPayload = false;
    }

    void endMessage() {
        buffer += "}";
    }

    void addString(const char* key, const char* value, bool comma = true) {
        if (comma && buffer.charAt(buffer.length() - 1) != '{') {
            buffer += ",";
        }
        buffer += "\"";
        buffer += key;
        buffer += "\":\"";
        buffer += value;
        buffer += "\"";
    }

    void addNumber(const char* key, float value, bool comma = true) {
        if (comma && buffer.charAt(buffer.length() - 1) != '{') {
            buffer += ",";
        }
        buffer += "\"";
        buffer += key;
        buffer += "\":";
        buffer += String(value, 6);
    }

    void addNumber(const char* key, int value, bool comma = true) {
        if (comma && buffer.charAt(buffer.length() - 1) != '{') {
            buffer += ",";
        }
        buffer += "\"";
        buffer += key;
        buffer += "\":";
        buffer += String(value);
    }

    void addBoolean(const char* key, bool value, bool comma = true) {
        if (comma && buffer.charAt(buffer.length() - 1) != '{') {
            buffer += ",";
        }
        buffer += "\"";
        buffer += key;
        buffer += "\":";
        buffer += value ? "true" : "false";
    }

    const char* getString() const {
        return buffer.c_str();
    }

    void clear() {
        buffer = "";
        inPayload = false;
    }
};

// ============================================================================
// PING/PONG
// ============================================================================

/**
 * Send PING message
 */
inline String createPingMessage(uint32_t seq) {
    JsonMessageBuilder builder;
    builder.beginMessage(MessageType::PING);
    builder.addTimestamp();
    builder.beginPayload();
    builder.addNumber("seq", seq, false);
    builder.endPayload();
    builder.endMessage();
    return String(builder.getString());
}

/**
 * Send PONG response
 */
inline String createPongMessage(uint32_t seq) {
    JsonMessageBuilder builder;
    builder.beginMessage(MessageType::PONG);
    builder.addTimestamp();
    builder.beginPayload();
    builder.addNumber("seq", seq, false);
    builder.endPayload();
    builder.endMessage();
    return String(builder.getString());
}

/**
 * Send ERROR message
 */
inline String createErrorMessage(const char* code, const char* message) {
    JsonMessageBuilder builder;
    builder.beginMessage(MessageType::ERROR);
    builder.addTimestamp();
    builder.beginPayload();
    builder.addString("code", code, false);
    builder.addString("message", message);
    builder.endPayload();
    builder.endMessage();
    return String(builder.getString());
}

} // namespace Protocol
} // namespace UCF

#endif // UCF_PROTOCOL_H
