/**
 * UCF Hardware Communication Protocol
 *
 * Defines the WebSocket and BLE communication protocols for
 * connecting the WishBed App to UCF hardware.
 */

import type {
  UCFHardwareState,
  UCFHardwareEvent,
  UCFEventType,
} from "./ucf-types";
import type { AnyUCFCommand, UCFCommandResponse } from "./ucf-commands";

// ============================================================================
// CONNECTION CONFIGURATION
// ============================================================================

/** WebSocket connection config */
export interface WebSocketConfig {
  /** ESP32 IP address or hostname */
  host: string;
  /** WebSocket port (default: 81) */
  port: number;
  /** Connection path */
  path: string;
  /** Reconnection attempts */
  maxRetries: number;
  /** Retry delay (ms) */
  retryDelay: number;
  /** Heartbeat interval (ms) */
  heartbeatInterval: number;
  /** Message timeout (ms) */
  messageTimeout: number;
}

/** Default WebSocket config */
export const DEFAULT_WS_CONFIG: WebSocketConfig = {
  host: "ucf-device.local",
  port: 81,
  path: "/ws",
  maxRetries: 5,
  retryDelay: 2000,
  heartbeatInterval: 5000,
  messageTimeout: 10000,
};

/** BLE connection config */
export interface BLEConfig {
  /** Device name filter */
  deviceName: string;
  /** Service UUID */
  serviceUUID: string;
  /** Command characteristic UUID */
  commandCharUUID: string;
  /** State characteristic UUID */
  stateCharUUID: string;
  /** Event characteristic UUID */
  eventCharUUID: string;
  /** Connection timeout (ms) */
  connectionTimeout: number;
}

/** Default BLE config */
export const DEFAULT_BLE_CONFIG: BLEConfig = {
  deviceName: "UCF-Hardware",
  serviceUUID: "4fafc201-1fb5-459e-8fcc-c5c9c331914b",
  commandCharUUID: "beb5483e-36e1-4688-b7f5-ea07361b26a8",
  stateCharUUID: "beb5483f-36e1-4688-b7f5-ea07361b26a8",
  eventCharUUID: "beb54840-36e1-4688-b7f5-ea07361b26a8",
  connectionTimeout: 10000,
};

// ============================================================================
// MESSAGE PROTOCOL
// ============================================================================

/** Protocol message types */
export type MessageType =
  | "COMMAND"
  | "COMMAND_RESPONSE"
  | "STATE_UPDATE"
  | "EVENT"
  | "PING"
  | "PONG"
  | "ERROR";

/** Base protocol message */
export interface ProtocolMessage<T extends MessageType = MessageType, P = unknown> {
  type: T;
  version: string; // Protocol version
  timestamp: number;
  payload: P;
}

/** Command message */
export interface CommandMessage extends ProtocolMessage<"COMMAND", AnyUCFCommand> {}

/** Command response message */
export interface CommandResponseMessage
  extends ProtocolMessage<"COMMAND_RESPONSE", UCFCommandResponse> {}

/** State update message */
export interface StateUpdateMessage
  extends ProtocolMessage<"STATE_UPDATE", Partial<UCFHardwareState>> {}

/** Event message */
export interface EventMessage extends ProtocolMessage<"EVENT", UCFHardwareEvent> {}

/** Ping message */
export interface PingMessage extends ProtocolMessage<"PING", { seq: number }> {}

/** Pong message */
export interface PongMessage extends ProtocolMessage<"PONG", { seq: number }> {}

/** Error message */
export interface ErrorMessage
  extends ProtocolMessage<"ERROR", { code: string; message: string }> {}

/** All message types union */
export type AnyProtocolMessage =
  | CommandMessage
  | CommandResponseMessage
  | StateUpdateMessage
  | EventMessage
  | PingMessage
  | PongMessage
  | ErrorMessage;

/** Current protocol version */
export const PROTOCOL_VERSION = "1.0.0";

// ============================================================================
// MESSAGE SERIALIZATION
// ============================================================================

/** Serialize message for transmission */
export function serializeMessage(message: AnyProtocolMessage): string {
  return JSON.stringify(message);
}

/** Deserialize received message */
export function deserializeMessage(data: string): AnyProtocolMessage {
  const parsed = JSON.parse(data);

  // Validate basic structure
  if (!parsed.type || !parsed.version || !parsed.timestamp) {
    throw new Error("Invalid message format");
  }

  return parsed as AnyProtocolMessage;
}

/** Create a protocol message */
export function createMessage<T extends MessageType, P>(
  type: T,
  payload: P
): ProtocolMessage<T, P> {
  return {
    type,
    version: PROTOCOL_VERSION,
    timestamp: Date.now(),
    payload,
  };
}

// ============================================================================
// BINARY PROTOCOL (BLE)
// ============================================================================

/**
 * Binary message format for BLE (space-constrained)
 *
 * Header (4 bytes):
 *   [0]: Message type (1 byte)
 *   [1-2]: Payload length (2 bytes, little-endian)
 *   [3]: Flags (1 byte)
 *
 * Payload: Variable length (max 512 bytes for BLE)
 */

/** Binary message types */
export enum BinaryMessageType {
  COMMAND = 0x01,
  RESPONSE = 0x02,
  STATE = 0x03,
  EVENT = 0x04,
  PING = 0xfe,
  PONG = 0xff,
}

/** Binary message flags */
export enum BinaryFlags {
  NONE = 0x00,
  COMPRESSED = 0x01,
  FRAGMENTED = 0x02,
  LAST_FRAGMENT = 0x04,
}

/** Encode binary message */
export function encodeBinaryMessage(
  type: BinaryMessageType,
  payload: Uint8Array,
  flags: number = BinaryFlags.NONE
): Uint8Array {
  const header = new Uint8Array(4);
  const view = new DataView(header.buffer);

  header[0] = type;
  view.setUint16(1, payload.length, true); // Little-endian
  header[3] = flags;

  const message = new Uint8Array(header.length + payload.length);
  message.set(header);
  message.set(payload, header.length);

  return message;
}

/** Decode binary message */
export function decodeBinaryMessage(data: Uint8Array): {
  type: BinaryMessageType;
  flags: number;
  payload: Uint8Array;
} {
  if (data.length < 4) {
    throw new Error("Message too short");
  }

  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);

  const type = data[0] as BinaryMessageType;
  const length = view.getUint16(1, true);
  const flags = data[3];

  if (data.length < 4 + length) {
    throw new Error("Incomplete message");
  }

  const payload = data.slice(4, 4 + length);

  return { type, flags, payload };
}

// ============================================================================
// STATE SUBSCRIPTION
// ============================================================================

/** State subscription options */
export interface StateSubscriptionOptions {
  /** Update interval (ms, 0 = on change only) */
  interval: number;
  /** Fields to include (empty = all) */
  fields: Array<keyof UCFHardwareState>;
  /** Event types to subscribe to */
  events: UCFEventType[];
}

/** Default subscription options */
export const DEFAULT_SUBSCRIPTION: StateSubscriptionOptions = {
  interval: 100, // 10 Hz
  fields: [],
  events: [
    "PHASE_TRANSITION",
    "TRIAD_UNLOCK",
    "K_FORMATION_ACHIEVED",
    "K_FORMATION_LOST",
    "SIGIL_MATCHED",
    "ERROR",
  ],
};

// ============================================================================
// CONNECTION STATE
// ============================================================================

/** Connection state */
export type ConnectionState =
  | "DISCONNECTED"
  | "CONNECTING"
  | "CONNECTED"
  | "RECONNECTING"
  | "ERROR";

/** Connection info */
export interface ConnectionInfo {
  state: ConnectionState;
  transport: "WEBSOCKET" | "BLE" | "NONE";
  deviceId?: string;
  firmwareVersion?: string;
  lastConnected?: number;
  lastDisconnected?: number;
  reconnectAttempts: number;
  latency?: number; // RTT in ms
}

// ============================================================================
// ERROR CODES
// ============================================================================

/** Protocol error codes */
export enum ProtocolError {
  /** Connection failed */
  CONNECTION_FAILED = "E_CONNECTION_FAILED",
  /** Connection lost */
  CONNECTION_LOST = "E_CONNECTION_LOST",
  /** Message timeout */
  TIMEOUT = "E_TIMEOUT",
  /** Invalid message format */
  INVALID_MESSAGE = "E_INVALID_MESSAGE",
  /** Unknown command */
  UNKNOWN_COMMAND = "E_UNKNOWN_COMMAND",
  /** Command failed */
  COMMAND_FAILED = "E_COMMAND_FAILED",
  /** Device busy */
  DEVICE_BUSY = "E_DEVICE_BUSY",
  /** Not connected */
  NOT_CONNECTED = "E_NOT_CONNECTED",
  /** Authentication failed */
  AUTH_FAILED = "E_AUTH_FAILED",
}
