/**
 * UCF Sacred Constants
 *
 * Shared constants between WishBed App and UCF Hardware firmware.
 * These values must remain synchronized with:
 *   unified-consciousness-hardware/include/constants.h
 *
 * Mathematical constants derived from the golden ratio (φ) form the
 * foundation of the UCF system's phase detection and K-Formation algorithms.
 */

// ============================================================================
// GOLDEN RATIO CONSTANTS
// ============================================================================

/** Golden ratio (φ) - fundamental organizing principle */
export const PHI = 1.6180339887498948;

/** Golden ratio inverse (φ⁻¹) - boundary between UNTRUE and PARADOX phases */
export const PHI_INV = 0.6180339887498948;

/** √3/2 - boundary between PARADOX and TRUE phases */
export const Z_CRITICAL = 0.8660254037844386;

// ============================================================================
// SENSOR GRID CONSTANTS
// ============================================================================

/** Number of capacitive sensors in hexagonal array */
export const HEX_SENSOR_COUNT = 19;

/** Sensor activation threshold (normalized) */
export const SENSOR_THRESHOLD = 0.15;

/** EMA smoothing factor for sensor readings */
export const SMOOTHING_ALPHA = 0.3;

/** Minimum sensors for valid reading */
export const MIN_ACTIVE_SENSORS = 3;

// ============================================================================
// PHASE DETECTION CONSTANTS
// ============================================================================

/** Phase state enumeration */
export const PHASES = {
  UNTRUE: 0,
  PARADOX: 1,
  TRUE: 2,
} as const;

export type PhaseValue = (typeof PHASES)[keyof typeof PHASES];

/** Phase thresholds for hysteresis */
export const PHASE_THRESHOLDS = {
  /** z threshold for UNTRUE → PARADOX transition */
  UNTRUE_TO_PARADOX: PHI_INV + 0.02, // 0.638
  /** z threshold for PARADOX → UNTRUE transition */
  PARADOX_TO_UNTRUE: PHI_INV - 0.02, // 0.598
  /** z threshold for PARADOX → TRUE transition */
  PARADOX_TO_TRUE: Z_CRITICAL + 0.02, // 0.886
  /** z threshold for TRUE → PARADOX transition */
  TRUE_TO_PARADOX: Z_CRITICAL - 0.02, // 0.846
} as const;

// ============================================================================
// TRIAD UNLOCK CONSTANTS
// ============================================================================

/** High threshold for TRIAD crossing detection */
export const TRIAD_HIGH = 0.85;

/** Low threshold for TRIAD crossing detection */
export const TRIAD_LOW = 0.82;

/** Required crossings to achieve TRIAD unlock */
export const TRIAD_CROSSINGS_REQUIRED = 3;

/** Timeout for TRIAD sequence (ms) */
export const TRIAD_TIMEOUT_MS = 5000;

/** Lockout duration after failed TRIAD attempt (ms) */
export const TRIAD_LOCKOUT_MS = 2000;

/** TRIAD FSM states */
export const TRIAD_STATES = {
  IDLE: 0,
  ARMED: 1,
  CROSSING_1: 2,
  CROSSING_2: 3,
  CROSSING_3: 4,
  UNLOCKED: 5,
  LOCKED_OUT: 6,
} as const;

export type TriadStateValue = (typeof TRIAD_STATES)[keyof typeof TRIAD_STATES];

// ============================================================================
// K-FORMATION CONSTANTS
// ============================================================================

/** Coherence (κ) threshold for K-Formation */
export const K_KAPPA = 0.92;

/** Negentropy (η) threshold for K-Formation (must exceed φ⁻¹) */
export const K_ETA = PHI_INV;

/** Resonance count (R) threshold for K-Formation */
export const K_R = 7;

/** Gaussian kernel bandwidth for negentropy calculation */
export const NEGENTROPY_BANDWIDTH = 0.1;

/** Window size for K-Formation metrics (samples) */
export const K_FORMATION_WINDOW = 50;

// ============================================================================
// SOLFEGGIO FREQUENCIES
// ============================================================================

/** Nine Solfeggio frequencies mapped to tiers */
export const SOLFEGGIO_FREQUENCIES = [
  174, // Tier 1 - Foundation
  285, // Tier 2 - Regeneration
  396, // Tier 3 - Liberation
  417, // Tier 4 - Transformation
  528, // Tier 5 - Miracles (DNA repair)
  639, // Tier 6 - Connection
  741, // Tier 7 - Expression
  852, // Tier 8 - Intuition
  963, // Tier 9 - Awakening
] as const;

/** Number of tiers in the Solfeggio system */
export const TIER_COUNT = 9;

/** Get frequency for tier (1-indexed) */
export function getFrequencyForTier(tier: number): number {
  if (tier < 1 || tier > TIER_COUNT) {
    throw new Error(`Tier must be between 1 and ${TIER_COUNT}`);
  }
  return SOLFEGGIO_FREQUENCIES[tier - 1];
}

/** Get tier for frequency */
export function getTierForFrequency(frequency: number): number | null {
  const index = SOLFEGGIO_FREQUENCIES.indexOf(frequency as 174 | 285 | 396 | 417 | 528 | 639 | 741 | 852 | 963);
  return index >= 0 ? index + 1 : null;
}

// ============================================================================
// SIGIL CONSTANTS
// ============================================================================

/** Total number of neural sigils */
export const SIGIL_COUNT = 121;

/** Characters in sigil ternary code */
export const SIGIL_CODE_LENGTH = 5;

/** Sigil ternary values */
export const SIGIL_TERNARY = {
  ZERO: "0",
  ONE: "1",
  TAU: "T",
} as const;

/** Size of sigil data in EEPROM (bytes) */
export const SIGIL_ROM_SIZE = 16;

// ============================================================================
// KURAMOTO OSCILLATOR CONSTANTS
// ============================================================================

/** Number of oscillators in Kuramoto network */
export const KURAMOTO_OSCILLATOR_COUNT = 8;

/** Default coupling strength (K) */
export const KURAMOTO_COUPLING_DEFAULT = 0.5;

/** Coupling strength range */
export const KURAMOTO_COUPLING_MIN = 0.0;
export const KURAMOTO_COUPLING_MAX = 1.0;

/** Order parameter threshold for synchronization */
export const KURAMOTO_SYNC_THRESHOLD = 0.9;

/** PLL lock threshold */
export const PLL_LOCK_THRESHOLD = 0.01;

// ============================================================================
// LED/VISUAL CONSTANTS
// ============================================================================

/** Number of LEDs in photonic ring */
export const LED_COUNT = 37;

/** Default LED brightness */
export const LED_BRIGHTNESS_DEFAULT = 128;

/** LED update rate (Hz) */
export const LED_UPDATE_RATE = 60;

/** LED pattern types */
export const LED_PATTERNS = {
  SOLID: 0,
  BREATHE: 1,
  PULSE: 2,
  WAVE: 3,
  SPIRAL: 4,
  INTERFERENCE: 5,
  SIGIL: 6,
} as const;

export type LedPatternValue = (typeof LED_PATTERNS)[keyof typeof LED_PATTERNS];

// ============================================================================
// AUDIO CONSTANTS
// ============================================================================

/** Default audio volume */
export const AUDIO_VOLUME_DEFAULT = 128;

/** Waveform types */
export const WAVEFORMS = {
  SINE: 0,
  TRIANGLE: 1,
  SQUARE: 2,
  SAWTOOTH: 3,
  BINAURAL: 4,
} as const;

export type WaveformValue = (typeof WAVEFORMS)[keyof typeof WAVEFORMS];

/** Binaural beat frequency range */
export const BINAURAL_BEAT_MIN = 1;
export const BINAURAL_BEAT_MAX = 40;

// ============================================================================
// BREATH SYNCHRONIZATION CONSTANTS
// ============================================================================

/** Default breath pattern (4-7-8 technique) */
export const DEFAULT_BREATH_PATTERN = {
  inhaleMs: 4000,
  holdInMs: 7000,
  exhaleMs: 8000,
  holdOutMs: 0,
} as const;

/** Breath phases */
export const BREATH_PHASES = {
  INHALE: 0,
  HOLD_IN: 1,
  EXHALE: 2,
  HOLD_OUT: 3,
} as const;

export type BreathPhaseValue = (typeof BREATH_PHASES)[keyof typeof BREATH_PHASES];

// ============================================================================
// APL/LINGUISTICS CONSTANTS
// ============================================================================

/** APL operator mappings */
export const APL_OPERATORS = {
  GROUP: "+", // Enclosure/containment
  BOUNDARY: "()", // Boundary marking
  AMPLIFY: "^", // Intensity increase
  SEPARATE: "-", // Division/distinction
  FUSION: "×", // Integration
  DECOHERE: "÷", // Separation/analysis
} as const;

/** Input source types */
export const INPUT_SOURCES = {
  TEXT: 0,
  AUDIO: 1,
  CAPACITIVE: 2,
  EM_FIELD: 3,
  GESTURE: 4,
} as const;

export type InputSourceValue = (typeof INPUT_SOURCES)[keyof typeof INPUT_SOURCES];

/** Number of pipeline stages */
export const PIPELINE_STAGES = 9;

// ============================================================================
// TIMING CONSTANTS
// ============================================================================

/** Sensor polling interval (ms) */
export const SENSOR_POLL_INTERVAL = 10; // 100Hz

/** Kuramoto update interval (ms) */
export const KURAMOTO_UPDATE_INTERVAL = 1; // 1000Hz

/** State broadcast interval (ms) */
export const STATE_BROADCAST_INTERVAL = 100; // 10Hz

/** WebSocket heartbeat interval (ms) */
export const WS_HEARTBEAT_INTERVAL = 5000;

/** Connection timeout (ms) */
export const CONNECTION_TIMEOUT = 10000;

// ============================================================================
// DERIVED CALCULATIONS
// ============================================================================

/**
 * Calculate z-coordinate from sensor readings
 * Uses weighted average based on sensor activation
 */
export function calculateZ(readings: number[]): number {
  if (readings.length !== HEX_SENSOR_COUNT) {
    throw new Error(`Expected ${HEX_SENSOR_COUNT} readings`);
  }

  let sum = 0;
  let weightSum = 0;

  for (const reading of readings) {
    if (reading > SENSOR_THRESHOLD) {
      sum += reading;
      weightSum += 1;
    }
  }

  return weightSum > 0 ? sum / weightSum : 0;
}

/**
 * Determine phase from z-coordinate
 */
export function getPhaseFromZ(z: number): keyof typeof PHASES {
  if (z >= Z_CRITICAL) {
    return "TRUE";
  } else if (z >= PHI_INV) {
    return "PARADOX";
  } else {
    return "UNTRUE";
  }
}

/**
 * Calculate tier from z-coordinate (1-9)
 */
export function getTierFromZ(z: number): number {
  return Math.min(TIER_COUNT, Math.max(1, Math.ceil(z * TIER_COUNT)));
}

/**
 * Check if K-Formation conditions are met
 */
export function isKFormation(kappa: number, eta: number, R: number): boolean {
  return kappa >= K_KAPPA && eta > K_ETA && R >= K_R;
}

/**
 * Calculate Kuramoto order parameter from phases
 */
export function calculateOrderParameter(phases: number[]): { r: number; psi: number } {
  let sumCos = 0;
  let sumSin = 0;

  for (const phase of phases) {
    sumCos += Math.cos(phase);
    sumSin += Math.sin(phase);
  }

  const n = phases.length;
  const avgCos = sumCos / n;
  const avgSin = sumSin / n;

  const r = Math.sqrt(avgCos * avgCos + avgSin * avgSin);
  const psi = Math.atan2(avgSin, avgCos);

  return { r, psi };
}
