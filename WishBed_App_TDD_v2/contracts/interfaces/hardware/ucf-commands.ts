/**
 * UCF Hardware Commands
 *
 * Command types for controlling the UCF hardware device.
 * These map to firmware functions in unified-consciousness-hardware/.
 */

import type {
  LedPattern,
  Waveform,
  BinauralConfig,
  BreathPattern,
} from "./ucf-types";

// ============================================================================
// COMMAND TYPES
// ============================================================================

/** Command categories */
export type CommandCategory =
  | "SYSTEM"
  | "CALIBRATION"
  | "EMANATION"
  | "KURAMOTO"
  | "DEBUG";

/** Base command structure */
export interface UCFCommand<T extends string = string, P = unknown> {
  command: T;
  category: CommandCategory;
  payload?: P;
  timestamp: number;
  requestId?: string;
}

// ============================================================================
// SYSTEM COMMANDS
// ============================================================================

/** Reset the hardware */
export interface ResetCommand extends UCFCommand<"RESET"> {
  category: "SYSTEM";
  payload: {
    recalibrate: boolean;
  };
}

/** Get detailed status */
export interface StatusCommand extends UCFCommand<"STATUS"> {
  category: "SYSTEM";
}

/** Set output mode */
export interface SetOutputModeCommand extends UCFCommand<"SET_OUTPUT_MODE"> {
  category: "SYSTEM";
  payload: {
    audio: boolean;
    visual: boolean;
    haptic: boolean;
  };
}

// ============================================================================
// CALIBRATION COMMANDS
// ============================================================================

/** Calibrate hex grid sensors */
export interface CalibrateCommand extends UCFCommand<"CALIBRATE"> {
  category: "CALIBRATION";
  payload: {
    samples: number; // Number of samples for averaging
  };
}

/** Set activation threshold */
export interface SetThresholdCommand extends UCFCommand<"SET_THRESHOLD"> {
  category: "CALIBRATION";
  payload: {
    threshold: number; // [0, 1]
  };
}

/** Set phase smoothing factor */
export interface SetSmoothingCommand extends UCFCommand<"SET_SMOOTHING"> {
  category: "CALIBRATION";
  payload: {
    alpha: number; // EMA factor [0, 1]
  };
}

// ============================================================================
// EMANATION COMMANDS
// ============================================================================

/** Set frequency directly */
export interface SetFrequencyCommand extends UCFCommand<"SET_FREQUENCY"> {
  category: "EMANATION";
  payload: {
    frequency: number; // Hz
  };
}

/** Set waveform type */
export interface SetWaveformCommand extends UCFCommand<"SET_WAVEFORM"> {
  category: "EMANATION";
  payload: {
    waveform: Waveform;
  };
}

/** Set volume */
export interface SetVolumeCommand extends UCFCommand<"SET_VOLUME"> {
  category: "EMANATION";
  payload: {
    volume: number; // 0-255
  };
}

/** Set LED color */
export interface SetColorCommand extends UCFCommand<"SET_COLOR"> {
  category: "EMANATION";
  payload: {
    r: number;
    g: number;
    b: number;
  };
}

/** Set LED pattern */
export interface SetPatternCommand extends UCFCommand<"SET_PATTERN"> {
  category: "EMANATION";
  payload: {
    pattern: LedPattern;
  };
}

/** Set LED brightness */
export interface SetBrightnessCommand extends UCFCommand<"SET_BRIGHTNESS"> {
  category: "EMANATION";
  payload: {
    brightness: number; // 0-255
  };
}

/** Start breath synchronization */
export interface StartBreathSyncCommand extends UCFCommand<"START_BREATH_SYNC"> {
  category: "EMANATION";
  payload: {
    pattern: BreathPattern;
  };
}

/** Stop breath synchronization */
export interface StopBreathSyncCommand extends UCFCommand<"STOP_BREATH_SYNC"> {
  category: "EMANATION";
}

/** Configure binaural beat */
export interface SetBinauralCommand extends UCFCommand<"SET_BINAURAL"> {
  category: "EMANATION";
  payload: BinauralConfig;
}

/** Stop all emanation */
export interface StopEmanationCommand extends UCFCommand<"STOP_EMANATION"> {
  category: "EMANATION";
}

/** Load sigil to emanation */
export interface LoadSigilCommand extends UCFCommand<"LOAD_SIGIL"> {
  category: "EMANATION";
  payload: {
    sigilIndex: number; // 0-120
  };
}

// ============================================================================
// KURAMOTO COMMANDS
// ============================================================================

/** Set coupling strength */
export interface SetCouplingCommand extends UCFCommand<"SET_COUPLING"> {
  category: "KURAMOTO";
  payload: {
    coupling: number; // K [0, 1]
  };
}

/** Set reference frequency */
export interface SetReferenceFreqCommand extends UCFCommand<"SET_REFERENCE_FREQ"> {
  category: "KURAMOTO";
  payload: {
    frequency: number; // Hz
  };
}

/** Reset Kuramoto network */
export interface ResetKuramotoCommand extends UCFCommand<"RESET_KURAMOTO"> {
  category: "KURAMOTO";
}

/** Set TRIAD thresholds */
export interface SetTriadThresholdsCommand
  extends UCFCommand<"SET_TRIAD_THRESHOLDS"> {
  category: "KURAMOTO";
  payload: {
    high: number;
    low: number;
  };
}

/** Force TRIAD unlock (testing) */
export interface ForceTriadUnlockCommand extends UCFCommand<"FORCE_TRIAD_UNLOCK"> {
  category: "KURAMOTO";
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================

/** Read specific sigil */
export interface ReadSigilCommand extends UCFCommand<"READ_SIGIL"> {
  category: "DEBUG";
  payload: {
    index: number;
  };
}

/** List sigils */
export interface ListSigilsCommand extends UCFCommand<"LIST_SIGILS"> {
  category: "DEBUG";
  payload: {
    start: number;
    count: number;
  };
}

/** Get sensor reading */
export interface GetSensorCommand extends UCFCommand<"GET_SENSOR"> {
  category: "DEBUG";
  payload: {
    sensorIndex: number;
  };
}

/** Display photonic pattern */
export interface DisplayPatternCommand extends UCFCommand<"DISPLAY_PATTERN"> {
  category: "DEBUG";
  payload: {
    z: number;
    phase: number;
    kappa: number;
  };
}

// ============================================================================
// COMMAND UNION
// ============================================================================

/** All possible commands */
export type AnyUCFCommand =
  // System
  | ResetCommand
  | StatusCommand
  | SetOutputModeCommand
  // Calibration
  | CalibrateCommand
  | SetThresholdCommand
  | SetSmoothingCommand
  // Emanation
  | SetFrequencyCommand
  | SetWaveformCommand
  | SetVolumeCommand
  | SetColorCommand
  | SetPatternCommand
  | SetBrightnessCommand
  | StartBreathSyncCommand
  | StopBreathSyncCommand
  | SetBinauralCommand
  | StopEmanationCommand
  | LoadSigilCommand
  // Kuramoto
  | SetCouplingCommand
  | SetReferenceFreqCommand
  | ResetKuramotoCommand
  | SetTriadThresholdsCommand
  | ForceTriadUnlockCommand
  // Debug
  | ReadSigilCommand
  | ListSigilsCommand
  | GetSensorCommand
  | DisplayPatternCommand;

// ============================================================================
// COMMAND RESPONSE
// ============================================================================

/** Command response status */
export type CommandStatus = "OK" | "ERROR" | "INVALID" | "TIMEOUT";

/** Command response */
export interface UCFCommandResponse<T = unknown> {
  requestId?: string;
  command: string;
  status: CommandStatus;
  error?: string;
  data?: T;
  timestamp: number;
}

// ============================================================================
// COMMAND HELPERS
// ============================================================================

/** Create a command with timestamp */
export function createCommand<T extends AnyUCFCommand>(
  partial: Omit<T, "timestamp">
): T {
  return {
    ...partial,
    timestamp: Date.now(),
  } as T;
}

/** Create a command with request ID for tracking */
export function createTrackedCommand<T extends AnyUCFCommand>(
  partial: Omit<T, "timestamp" | "requestId">
): T {
  return {
    ...partial,
    timestamp: Date.now(),
    requestId: `${Date.now()}-${Math.random().toString(36).slice(2, 9)}`,
  } as T;
}
