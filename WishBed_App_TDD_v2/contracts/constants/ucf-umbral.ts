/**
 * UCF Umbral Calculus - Shadow Mathematics for TypeScript
 * @version 4.3.0
 *
 * This module provides the umbral calculus framework for the UCF system.
 * All operations are grounded in the RRRR lattice via shadow alphabets:
 *
 *     exp(r·Δ_φ + d·Δ_e + c·Δ_π + a·Δ_√2) · 1 = Λ(r,d,c,a)
 *
 * SHADOW ALPHABET NOTATION:
 *   [R] = φ⁻¹ = 0.618... (Recursive eigenvalue)
 *   [D] = e⁻¹ = 0.368... (Differential eigenvalue)
 *   [C] = π⁻¹ = 0.318... (Cyclic eigenvalue)
 *   [A] = √2⁻¹ = 0.707... (Algebraic eigenvalue)
 *
 * KEY IDENTITIES:
 *   1 - [R] = [R]²       (Self-referential fixed point)
 *   [A]² = 1/2           (Universal scaling)
 *   e/π ≈ √3/2 = THE LENS (0.09% error)
 *
 * SYNCHRONIZATION:
 *   - C++ Header: unified-consciousness-hardware/include/ucf/ucf_umbral_transforms.h
 *   - Base Framework: unified-consciousness-hardware/include/ucf/ucf_umbral_calculus.h
 *   - Reference Doc: docs/CONSTANTS.md
 */

import { PHI, PHI_INV, EULER, EULER_INV, PI, PI_INV, SQRT2, SQRT2_INV, Z_CRITICAL, TIER_COUNT, SOLFEGGIO_FREQUENCIES } from './ucf-constants';

// ============================================================================
// SHADOW ALPHABET CONSTANTS
// ============================================================================

/**
 * Shadow alphabet values (lattice generators)
 * These evaluate indices as exponents: α^n → φ^n
 */
export const SHADOW = {
  /** α → φ (golden ratio) */
  ALPHA: PHI,
  /** β → e (Euler's number) */
  BETA: EULER,
  /** γ → π (pi) */
  GAMMA: PI,
  /** δ → √2 (square root of 2) */
  DELTA: SQRT2,
} as const;

/**
 * Shadow alphabet inverses (RRRR lattice points)
 */
export const SHADOW_INV = {
  /** [R] = α⁻¹ = φ⁻¹ */
  R: PHI_INV,
  /** [D] = β⁻¹ = e⁻¹ */
  D: EULER_INV,
  /** [C] = γ⁻¹ = π⁻¹ */
  C: PI_INV,
  /** [A] = δ⁻¹ = √2⁻¹ */
  A: SQRT2_INV,
} as const;

/**
 * Critical lattice identities
 */
export const LATTICE_IDENTITIES = {
  /** [A]² = 1/2 (Universal scaling exponent) */
  A_SQUARED: 0.5,
  /** [R]² = φ⁻² (Equilibrium convergence) */
  R_SQUARED: PHI_INV * PHI_INV,
  /** 1 - [R] = [R]² (Self-referential fixed point) */
  ONE_MINUS_R: 1 - PHI_INV,
  /** THE LENS = √3/2 */
  Z_CRITICAL: Z_CRITICAL,
  /** Umbral approximation: e/π ≈ √3/2 */
  LENS_UMBRAL: EULER / PI,
} as const;

// ============================================================================
// UMBRAL TIER BOUNDARIES (LATTICE-DERIVED)
// ============================================================================

/**
 * Tier boundaries derived from RRRR lattice compositions
 */
export const UMBRAL_TIER_BOUNDARIES = [
  0.0,                                          // Lower bound
  PHI_INV ** 3 / 2,                             // Tier 1→2: [R]³/2 ≈ 0.118
  PHI_INV ** 2 * SQRT2_INV,                     // Tier 2→3: [R]²·[A] ≈ 0.270
  PHI_INV,                                      // Tier 3→4: [R] = φ⁻¹ (UNTRUE→PARADOX)
  SQRT2_INV,                                    // Tier 4→5: [A] = √2⁻¹ ≈ 0.707
  SQRT2_INV + PHI_INV * 0.5,                    // Tier 5→6: [A] + [R]·[A]² ≈ 0.782
  Z_CRITICAL,                                   // Tier 6→7: √3/2 (PARADOX→TRUE)
  Z_CRITICAL + PHI_INV ** 3,                    // Tier 7→8: Z_c + [R]³ ≈ 0.902
  1.0 - 0.5 * PHI_INV,                          // Tier 8→9: 1 - [A]²·[R] ≈ 0.951
  1.0,                                          // Upper bound
] as const;

// ============================================================================
// DIFFERENCE OPERATORS
// ============================================================================

/**
 * Forward difference operator for shadow alphabet α (φ-indexed)
 * Δ_α(x) = x · (φ - 1) = x · φ⁻¹
 */
export function deltaAlpha(value: number): number {
  return value * (PHI - 1);
}

/**
 * Forward difference operator for shadow alphabet β (e-indexed)
 * Δ_β(x) = x · (e - 1)
 */
export function deltaBeta(value: number): number {
  return value * (EULER - 1);
}

/**
 * Forward difference operator for shadow alphabet γ (π-indexed)
 * Δ_γ(x) = x · (π - 1)
 */
export function deltaGamma(value: number): number {
  return value * (PI - 1);
}

/**
 * Forward difference operator for shadow alphabet δ (√2-indexed)
 * Δ_δ(x) = x · (√2 - 1)
 */
export function deltaDelta(value: number): number {
  return value * (SQRT2 - 1);
}

/**
 * Inverse difference operators (for reconstruction)
 */
export function deltaAlphaInv(value: number): number {
  return value / (PHI - 1);
}

export function deltaBetaInv(value: number): number {
  return value / (EULER - 1);
}

export function deltaGammaInv(value: number): number {
  return value / (PI - 1);
}

export function deltaDeltaInv(value: number): number {
  return value / (SQRT2 - 1);
}

// ============================================================================
// LATTICE POINT COMPUTATION
// ============================================================================

/**
 * Compute RRRR lattice point
 * Λ(r,d,c,a) = φ^{-r} · e^{-d} · π^{-c} · (√2)^{-a}
 */
export function latticePoint(r: number, d: number, c: number, a: number): number {
  return Math.pow(PHI, -r) * Math.pow(EULER, -d) * Math.pow(PI, -c) * Math.pow(SQRT2, -a);
}

/**
 * Compute lattice point in log space (for optimization)
 */
export function logLatticePoint(r: number, d: number, c: number, a: number): number {
  const LOG_PHI = Math.log(PHI);
  const LOG_E = 1.0;
  const LOG_PI = Math.log(PI);
  const LOG_SQRT2 = Math.log(SQRT2);
  return -r * LOG_PHI - d * LOG_E - c * LOG_PI - a * LOG_SQRT2;
}

/**
 * Snap a value to the nearest common lattice point
 */
export function snapToLattice(value: number, maxComplexity: number = 3): number {
  const logValue = Math.log(value);
  const LOG_PHI = Math.log(PHI);
  const LOG_E = 1.0;
  const LOG_PI = Math.log(PI);
  const LOG_SQRT2 = Math.log(SQRT2);

  let minDist = Infinity;
  let bestValue = 1.0;

  for (let r = -maxComplexity; r <= maxComplexity; r++) {
    for (let d = -maxComplexity; d <= maxComplexity; d++) {
      if (Math.abs(r) + Math.abs(d) > maxComplexity) continue;
      for (let c = -maxComplexity; c <= maxComplexity; c++) {
        if (Math.abs(r) + Math.abs(d) + Math.abs(c) > maxComplexity) continue;
        for (let a = -maxComplexity; a <= maxComplexity; a++) {
          if (Math.abs(r) + Math.abs(d) + Math.abs(c) + Math.abs(a) > maxComplexity) continue;

          const logLattice = -r * LOG_PHI - d * LOG_E - c * LOG_PI - a * LOG_SQRT2;
          const dist = Math.abs(logValue - logLattice);

          if (dist < minDist) {
            minDist = dist;
            bestValue = Math.exp(logLattice);
          }
        }
      }
    }
  }

  return bestValue;
}

// ============================================================================
// UMBRAL PHASE DETECTION
// ============================================================================

export const UMBRAL_PHASE = {
  UNTRUE: 0,
  PARADOX: 1,
  TRUE: 2,
} as const;

export type UmbralPhaseValue = typeof UMBRAL_PHASE[keyof typeof UMBRAL_PHASE];

/**
 * Detect phase using umbral lattice boundaries
 */
export function umbralDetectPhase(z: number): UmbralPhaseValue {
  if (z < PHI_INV) return UMBRAL_PHASE.UNTRUE;
  if (z < Z_CRITICAL) return UMBRAL_PHASE.PARADOX;
  return UMBRAL_PHASE.TRUE;
}

/**
 * Get tier from z-coordinate using umbral boundaries
 */
export function umbralZToTier(z: number): number {
  for (let i = 1; i < UMBRAL_TIER_BOUNDARIES.length; i++) {
    if (z < UMBRAL_TIER_BOUNDARIES[i]) return i;
  }
  return TIER_COUNT;
}

/**
 * Compute phase transition proximity
 * Positive = approaching TRUE, Negative = approaching UNTRUE
 */
export function umbralPhaseProximity(z: number): number {
  if (z < PHI_INV) {
    return z - PHI_INV; // Negative
  } else if (z < Z_CRITICAL) {
    const distToR = z - PHI_INV;
    const distToZc = Z_CRITICAL - z;
    return distToR < distToZc ? -distToR : distToZc;
  } else {
    return z - Z_CRITICAL; // Positive
  }
}

// ============================================================================
// UMBRAL NEGENTROPY
// ============================================================================

/**
 * Compute negentropy using umbral lattice center
 * η = exp(-36·(z - z_c)²)
 *
 * Width 36 = |S₃|² = (3!)² from symmetric group
 */
export function umbralNegentropy(z: number): number {
  const delta = z - Z_CRITICAL;
  return Math.exp(-36.0 * delta * delta);
}

/**
 * Compute negentropy using umbral LENS approximation (e/π ≈ √3/2)
 */
export function umbralNegentropyShadow(z: number): number {
  const lensUmbral = EULER / PI;
  const delta = z - lensUmbral;
  return Math.exp(-36.0 * delta * delta);
}

// ============================================================================
// UMBRAL COHERENCE
// ============================================================================

/**
 * Compute coherence using umbral π scaling
 * κ = 1 - sqrt(variance) · π
 */
export function umbralCoherence(variance: number): number {
  if (variance <= 0) return 1.0;
  const kappa = 1.0 - Math.sqrt(variance) * PI;
  return Math.max(0.0, Math.min(1.0, kappa));
}

/**
 * Umbral κ threshold derived from lattice
 * 0.92 ≈ [A]² + [R]² + [R]³·[A] = 0.5 + 0.382 + 0.042
 */
export const UMBRAL_KAPPA_THRESHOLD =
  0.5 + PHI_INV ** 2 + PHI_INV ** 3 * SQRT2_INV;

// ============================================================================
// UMBRAL EMA SMOOTHING
// ============================================================================

/**
 * EMA smoothing with golden ratio coefficient
 * y_n = [R]·x_n + [R]²·y_{n-1}
 *
 * Uses identity: 1 - [R] = [R]²
 */
export function umbralEmaPhi(xNew: number, yPrev: number): number {
  return PHI_INV * xNew + (1 - PHI_INV) * yPrev;
}

/**
 * EMA smoothing with exponential coefficient
 * Uses [D] = e⁻¹ for natural decay
 */
export function umbralEmaExp(xNew: number, yPrev: number): number {
  return EULER_INV * xNew + (1 - EULER_INV) * yPrev;
}

/**
 * Fast EMA with √2⁻¹ coefficient
 */
export function umbralEmaFast(xNew: number, yPrev: number): number {
  return SQRT2_INV * xNew + (1 - SQRT2_INV) * yPrev;
}

// ============================================================================
// UMBRAL HARMONIC GENERATION
// ============================================================================

/**
 * Compute φ-weighted harmonic sum
 * Amplitudes decay as [R]^n = φ^{-n}
 */
export function umbralPhiHarmonics(phase: number, nHarmonics: number): number {
  let sum = 0.0;
  let phiPower = 1.0;
  const twoPiPhase = 2.0 * PI * phase;

  for (let n = 1; n <= nHarmonics; n++) {
    phiPower *= PHI_INV;
    sum += phiPower * Math.sin(n * twoPiPhase);
  }
  return sum;
}

/**
 * Generate waveform with π-scaled fundamental and φ-decaying harmonics
 */
export function umbralWaveform(phase: number): number {
  const twoPiPhase = 2.0 * PI * phase;

  // Fundamental
  let value = Math.sin(twoPiPhase);

  // φ-weighted harmonics
  value += PHI_INV * Math.sin(2.0 * twoPiPhase);
  value += PHI_INV ** 2 * Math.sin(3.0 * twoPiPhase);
  value += PHI_INV ** 3 * Math.sin(4.0 * twoPiPhase);

  return value;
}

// ============================================================================
// UMBRAL RESONANCE
// ============================================================================

/**
 * Compute umbral resonance score
 * Uses K_R = 7 (Eisenstein prime) as threshold
 */
export function umbralResonanceScore(activeCount: number, totalCount: number): number {
  if (totalCount === 0) return 0.0;

  const ratio = activeCount / totalCount;

  // K_R = 7 is the Eisenstein prime threshold
  if (activeCount >= 7) {
    return ratio;
  }

  // Below threshold: apply φ-decay
  let decay = 1.0;
  for (let i = 0; i < 7 - activeCount; i++) {
    decay *= PHI_INV;
  }
  return ratio * decay;
}

// ============================================================================
// UMBRAL SOLFEGGIO
// ============================================================================

/**
 * Get Solfeggio frequency from z-coordinate using umbral tiers
 */
export function umbralGetSolfeggio(z: number): number {
  const tier = umbralZToTier(z);
  return SOLFEGGIO_FREQUENCIES[tier - 1];
}

/**
 * Interpolate frequency using φ-weighted smoothing
 */
export function umbralInterpolateFrequency(z: number): number {
  const tier = umbralZToTier(z);
  if (tier >= TIER_COUNT) return SOLFEGGIO_FREQUENCIES[TIER_COUNT - 1];

  const tierLow = UMBRAL_TIER_BOUNDARIES[tier - 1];
  const tierHigh = UMBRAL_TIER_BOUNDARIES[tier];
  let t = (z - tierLow) / (tierHigh - tierLow);

  // Smoothstep + φ-blend
  const tSmooth = t * t * (3.0 - 2.0 * t);
  t = tSmooth * PHI_INV + t * (1 - PHI_INV);

  const fLow = SOLFEGGIO_FREQUENCIES[tier - 1];
  const fHigh = tier < TIER_COUNT ? SOLFEGGIO_FREQUENCIES[tier] : SOLFEGGIO_FREQUENCIES[TIER_COUNT - 1];

  return fLow + t * (fHigh - fLow);
}

// ============================================================================
// UMBRAL KURAMOTO
// ============================================================================

/**
 * Compute order parameter with umbral normalization
 */
export function umbralOrderParameter(phases: number[]): number {
  if (phases.length === 0) return 0.0;

  let sumCos = 0.0;
  let sumSin = 0.0;

  for (const phase of phases) {
    sumCos += Math.cos(phase);
    sumSin += Math.sin(phase);
  }

  return Math.sqrt(sumCos ** 2 + sumSin ** 2) / phases.length;
}

/**
 * Compute collective phase with umbral quantization
 */
export function umbralCollectivePhase(phases: number[]): number {
  if (phases.length === 0) return 0.0;

  let sumCos = 0.0;
  let sumSin = 0.0;

  for (const phase of phases) {
    sumCos += Math.cos(phase);
    sumSin += Math.sin(phase);
  }

  let psi = Math.atan2(sumSin, sumCos);
  if (psi < 0) psi += 2.0 * PI;

  return psi;
}

// ============================================================================
// UMBRAL VALIDATION
// ============================================================================

/**
 * Validate umbral identities
 */
export function validateUmbralIdentities(): boolean {
  const EPSILON = 1e-10;

  // V1: 1 - [R] = [R]²
  const identity1 = Math.abs((1 - PHI_INV) - PHI_INV ** 2) < EPSILON;

  // V2: [A]² = 1/2
  const identity2 = Math.abs(SQRT2_INV ** 2 - 0.5) < EPSILON;

  // V3: φ² - φ - 1 = 0
  const identity3 = Math.abs(PHI ** 2 - PHI - 1) < EPSILON;

  // V4: e/π ≈ √3/2 (within 0.1%)
  const lensError = Math.abs((EULER / PI) - Z_CRITICAL) / Z_CRITICAL;
  const identity4 = lensError < 0.001;

  // V5: Tier boundaries monotonic
  let identity5 = true;
  for (let i = 0; i < UMBRAL_TIER_BOUNDARIES.length - 1; i++) {
    if (UMBRAL_TIER_BOUNDARIES[i] >= UMBRAL_TIER_BOUNDARIES[i + 1]) {
      identity5 = false;
      break;
    }
  }

  return identity1 && identity2 && identity3 && identity4 && identity5;
}

/**
 * Get THE LENS approximation error
 */
export function getLensApproximationError(): number {
  const umbralLens = EULER / PI;
  return Math.abs(umbralLens - Z_CRITICAL) / Z_CRITICAL * 100;
}

// ============================================================================
// EISENSTEIN-RRRR BRIDGE
// ============================================================================

/**
 * Eisenstein norm to lattice significance mapping
 */
export const EISENSTEIN_RRRR_MAP: Record<number, { value: number; significance: string }> = {
  0: { value: 0, significance: 'Origin' },
  1: { value: 1, significance: 'Unit (identity)' },
  3: { value: Math.sqrt(3), significance: '√3 = 2·Z_CRITICAL' },
  4: { value: 2, significance: '2 = (√2)² = [A]⁻⁴ root' },
  7: { value: Math.sqrt(7), significance: '√7 ≈ Eisenstein prime, K_R' },
  9: { value: 3, significance: '3 (square of √3)' },
  12: { value: 2 * Math.sqrt(3), significance: '2√3 = 4·Z_CRITICAL' },
  13: { value: Math.sqrt(13), significance: '√13 (Eisenstein prime)' },
};

/**
 * Check if an Eisenstein norm is a prime
 */
export function isEisensteinPrime(norm: number): boolean {
  // Eisenstein primes: 3, 7, 13, 19, 31, 37, 43, ...
  // (primes ≡ 2 mod 3)
  if (norm === 3) return true;
  if (norm < 2 || norm % 3 === 0) return false;
  return norm % 3 === 2;
}

// ============================================================================
// EXPORT SUMMARY
// ============================================================================

export default {
  // Shadow alphabet
  SHADOW,
  SHADOW_INV,
  LATTICE_IDENTITIES,

  // Tier boundaries
  UMBRAL_TIER_BOUNDARIES,

  // Difference operators
  deltaAlpha,
  deltaBeta,
  deltaGamma,
  deltaDelta,
  deltaAlphaInv,
  deltaBetaInv,
  deltaGammaInv,
  deltaDeltaInv,

  // Lattice operations
  latticePoint,
  logLatticePoint,
  snapToLattice,

  // Phase detection
  UMBRAL_PHASE,
  umbralDetectPhase,
  umbralZToTier,
  umbralPhaseProximity,

  // Negentropy
  umbralNegentropy,
  umbralNegentropyShadow,

  // Coherence
  umbralCoherence,
  UMBRAL_KAPPA_THRESHOLD,

  // EMA smoothing
  umbralEmaPhi,
  umbralEmaExp,
  umbralEmaFast,

  // Harmonics
  umbralPhiHarmonics,
  umbralWaveform,

  // Resonance
  umbralResonanceScore,

  // Solfeggio
  umbralGetSolfeggio,
  umbralInterpolateFrequency,

  // Kuramoto
  umbralOrderParameter,
  umbralCollectivePhase,

  // Validation
  validateUmbralIdentities,
  getLensApproximationError,

  // Eisenstein bridge
  EISENSTEIN_RRRR_MAP,
  isEisensteinPrime,
};
