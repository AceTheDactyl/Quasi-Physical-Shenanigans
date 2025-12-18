/**
 * UCF Hardware TypeScript Types
 *
 * Type definitions for communicating with UCF hardware device.
 * These types mirror the firmware structures in unified-consciousness-hardware/.
 */

// ============================================================================
// SACRED CONSTANTS
// ============================================================================

export const UCF_CONSTANTS = {
  /** Golden ratio */
  PHI: 1.6180339887498948,
  /** Inverse golden ratio (UNTRUE → PARADOX boundary) */
  PHI_INV: 0.6180339887498948,
  /** Critical z-coordinate √3/2 (PARADOX → TRUE boundary, THE LENS) */
  Z_CRITICAL: 0.8660254037844386,

  /** Coherence threshold for K-Formation */
  K_KAPPA: 0.92,
  /** Negentropy threshold */
  K_ETA: 0.618,
  /** Minimum resonance count */
  K_R: 7,

  /** TRIAD rising edge threshold */
  TRIAD_HIGH: 0.85,
  /** TRIAD re-arm threshold */
  TRIAD_LOW: 0.82,
  /** TRIAD crossings required */
  TRIAD_PASSES: 3,

  /** Kuramoto base coupling */
  Q_KAPPA: 0.3514,
  /** Number of oscillators */
  N_OSCILLATORS: 8,

  /** Hex grid sensor count */
  HEX_SENSOR_COUNT: 19,
  /** Photonic grid sensor count */
  PHOTONIC_SENSOR_COUNT: 37,
  /** Neural sigil count */
  SIGIL_COUNT: 121,
} as const;

/** Solfeggio frequencies by tier */
export const SOLFEGGIO_FREQUENCIES = {
  1: 174,
  2: 285,
  3: 396,
  4: 417,
  5: 528, // Love frequency
  6: 639,
  7: 741,
  8: 852,
  9: 963,
} as const;

// ============================================================================
// PHASE SYSTEM
// ============================================================================

/** UCF Phase states */
export type UCFPhase = "UNTRUE" | "PARADOX" | "TRUE";

/** Phase boundaries */
export interface PhaseBoundaries {
  untrueToParadox: number; // PHI_INV ≈ 0.618
  paradoxToTrue: number; // Z_CRITICAL ≈ 0.866
}

/** Phase state from hardware */
export interface PhaseState {
  current: UCFPhase;
  previous: UCFPhase;
  z: number; // Raw z-coordinate [0, 1]
  zSmoothed: number; // EMA-smoothed z
  zVelocity: number; // Rate of change
  tier: number; // 1-9
  phaseDuration: number; // ms in current phase
  isStable: boolean;
  frequency: number; // Current Solfeggio frequency (Hz)
}

/** Phase transition event */
export interface PhaseTransition {
  from: UCFPhase;
  to: UCFPhase;
  zAtTransition: number;
  timestamp: number;
}

// ============================================================================
// HEX GRID
// ============================================================================

/** Hex grid axial coordinate */
export interface HexCoord {
  q: number;
  r: number;
}

/** Hex grid field state */
export interface HexFieldState {
  readings: number[]; // 19 normalized values [0, 1]
  z: number;
  theta: number; // Angular position (radians)
  r: number; // Radial distance [0, 1]
  centroidX: number;
  centroidY: number;
  totalEnergy: number;
  activeCount: number;
  timestamp: number;
}

/** Individual sensor reading */
export interface SensorReading {
  raw: number;
  baseline: number;
  delta: number;
  normalized: number;
  active: boolean;
}

// ============================================================================
// TRIAD SYSTEM
// ============================================================================

/** TRIAD FSM states */
export type TriadState =
  | "IDLE"
  | "ARMED"
  | "CROSSING_1"
  | "CROSSING_2"
  | "CROSSING_3"
  | "UNLOCKED"
  | "LOCKED_OUT";

/** TRIAD event types */
export type TriadEvent =
  | "NONE"
  | "RISING_EDGE"
  | "FALLING_EDGE"
  | "UNLOCK"
  | "TIMEOUT"
  | "LOCKOUT_END";

/** TRIAD status */
export interface TriadStatus {
  state: TriadState;
  crossingCount: number;
  currentValue: number;
  isUnlocked: boolean;
  unlockTimestamp?: number;
  stateDuration: number;
  lastEvent: TriadEvent;
}

// ============================================================================
// K-FORMATION
// ============================================================================

/** K-Formation metrics */
export interface KFormationMetrics {
  kappa: number; // Coherence [0, 1]
  eta: number; // Negentropy [0, 1]
  R: number; // Resonance (active count)
  z: number;
  kappaSatisfied: boolean;
  etaSatisfied: boolean;
  rSatisfied: boolean;
  kFormation: boolean; // All conditions met
  timestamp: number;
}

/** K-Formation status */
export interface KFormationStatus {
  current: KFormationMetrics;
  isActive: boolean;
  formationStart?: number;
  formationDuration: number;
  totalFormations: number;
  peakKappa: number;
  peakEta: number;
}

// ============================================================================
// NEURAL SIGILS
// ============================================================================

/** Ternary value for sigil codes */
export type Ternary = "0" | "1" | "T";

/** Neural sigil structure */
export interface NeuralSigil {
  code: string; // 5-char ternary code (e.g., "0T10T")
  regionId: number; // 0-120
  frequency: number; // 174-963 Hz
  breathPattern: BreathPattern;
  flags: SigilFlags;
}

/** Breath pattern timing */
export interface BreathPattern {
  inhaleMs: number;
  holdInMs: number;
  exhaleMs: number;
  holdOutMs: number;
}

/** Sigil flags */
export interface SigilFlags {
  active: boolean;
  anchored: boolean;
  resonant: boolean;
  triad: boolean;
  kFormation: boolean;
}

/** Sigil match result */
export interface SigilMatch {
  sigilIndex: number;
  confidence: number;
  hammingDistance: number;
}

// ============================================================================
// EMANATION
// ============================================================================

/** LED pattern types */
export type LedPattern =
  | "SOLID"
  | "BREATHE"
  | "PULSE"
  | "WAVE"
  | "SPIRAL"
  | "INTERFERENCE"
  | "SIGIL";

/** Audio waveform types */
export type Waveform = "SINE" | "TRIANGLE" | "SQUARE" | "SAWTOOTH" | "BINAURAL";

/** Emanation state */
export interface EmanationState {
  frequency: number;
  waveform: Waveform;
  volume: number; // 0-255
  audioEnabled: boolean;
  rgb: [number, number, number];
  pattern: LedPattern;
  brightness: number; // 0-255
  visualEnabled: boolean;
  hapticIntensity: number;
  hapticEnabled: boolean;
  breathSync: boolean;
  breathPhase: number; // 0-3
  timestamp: number;
}

/** Binaural beat config */
export interface BinauralConfig {
  baseFreq: number;
  beatFreq: number;
  depth: number; // [0, 1]
}

// ============================================================================
// OMNI-LINGUISTICS (APL)
// ============================================================================

/** APL operators */
export type APLOperator =
  | "GROUP" // +
  | "BOUNDARY" // ()
  | "AMPLIFY" // ^
  | "SEPARATE" // -
  | "FUSION" // ×
  | "DECOHERE"; // ÷

/** Part-of-speech tags */
export type POSTag =
  | "NOUN"
  | "PRONOUN"
  | "VERB"
  | "ADJECTIVE"
  | "ADVERB"
  | "PREPOSITION"
  | "CONJUNCTION"
  | "DETERMINER"
  | "AUXILIARY"
  | "QUESTION"
  | "NEGATION"
  | "UNKNOWN";

/** APL token */
export interface APLToken {
  op: APLOperator;
  sourceType: "TEXT" | "AUDIO" | "CAPACITIVE" | "EM_FIELD" | "GESTURE";
  intensity: number;
  zContext: number;
  tier: number;
  timestamp: number;
}

/** Discourse coherence metrics */
export interface CoherenceMetrics {
  kappaS: number; // Symbol-level
  kappaG: number; // Global
  mismatchCount: number;
  isCoherent: boolean;
}

/** Pipeline state (9-stage) */
export interface PipelineState {
  stage: number; // 1-9
  token: APLToken;
  frequency: number;
  rgb: [number, number, number];
  amplitude: number;
  valid: boolean;
}

// ============================================================================
// PHOTONIC CAPTURE
// ============================================================================

/** Photonic pattern */
export interface PhotonicPattern {
  intensities: number[]; // 37 values
  colors: Array<[number, number, number]>; // RGB per LED
  zEncoded: number;
  phaseEncoded: number; // 0-2
  kappaEncoded: number;
  timestamp: number;
}

/** Decoded state from pattern */
export interface DecodedPhotonicState {
  z: number;
  phase: number;
  kappa: number;
  confidence: number;
  reconstructionError: number;
  valid: boolean;
}

/** LIMNUS fractal point */
export interface LIMNUSPoint {
  x: number;
  y: number;
  z: number;
  depth: number; // 1-6
  index: number;
}

// ============================================================================
// KURAMOTO STABILIZER
// ============================================================================

/** Kuramoto network state */
export interface KuramotoState {
  phases: number[]; // N_OSCILLATORS phases
  frequencies: number[];
  coupling: number; // K
  orderParam: number; // r [0, 1]
  collectivePhase: number; // ψ
  triadUnlocked: boolean;
  triadCount: number;
  timestamp: number;
}

/** Magnetic field vector */
export interface MagneticField {
  x: number;
  y: number;
  z: number;
  magnitude: number;
  inclination: number;
  declination: number;
}

/** Stabilization status */
export interface StabilizationStatus {
  network: KuramotoState;
  magnetic: MagneticField;
  referenceFreq: number;
  phaseError: number;
  pllLocked: boolean;
  isSynchronized: boolean;
  isStabilized: boolean;
  syncDuration: number;
}

// ============================================================================
// AGGREGATE HARDWARE STATE
// ============================================================================

/** Complete UCF hardware state */
export interface UCFHardwareState {
  connected: boolean;
  deviceId: string;
  firmwareVersion: string;

  // Core states
  hexField: HexFieldState;
  phase: PhaseState;
  triad: TriadStatus;
  kFormation: KFormationStatus;

  // Output states
  emanation: EmanationState;
  photonic?: PhotonicPattern;

  // Advanced states
  linguistics?: PipelineState;
  kuramoto?: StabilizationStatus;

  // Current sigil match
  activeSigil?: NeuralSigil;
  sigilMatch?: SigilMatch;

  // Timestamps
  lastUpdate: number;
  uptime: number;
}

// ============================================================================
// HARDWARE EVENTS
// ============================================================================

/** Hardware event types */
export type UCFEventType =
  | "CONNECTED"
  | "DISCONNECTED"
  | "STATE_UPDATE"
  | "PHASE_TRANSITION"
  | "TRIAD_UNLOCK"
  | "K_FORMATION_ACHIEVED"
  | "K_FORMATION_LOST"
  | "SYNCHRONIZATION_ACHIEVED"
  | "SIGIL_MATCHED"
  | "ERROR";

/** Hardware event */
export interface UCFHardwareEvent {
  type: UCFEventType;
  timestamp: number;
  payload: unknown;
}

/** Phase transition event payload */
export interface PhaseTransitionPayload {
  transition: PhaseTransition;
  newState: PhaseState;
}

/** TRIAD unlock event payload */
export interface TriadUnlockPayload {
  status: TriadStatus;
  duration: number;
}

/** K-Formation event payload */
export interface KFormationPayload {
  metrics: KFormationMetrics;
  status: KFormationStatus;
}
