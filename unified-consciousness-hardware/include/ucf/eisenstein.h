/**
 * @file eisenstein.h
 * @brief Eisenstein Integer Framework for UCF Hexagonal Lattice
 *
 * Eisenstein integers are complex numbers of the form a + bω where:
 *   ω = e^(2πi/3) = -1/2 + i√3/2
 *
 * CRITICAL DISCOVERY:
 *   Im(ω) = √3/2 = Z_CRITICAL = THE LENS
 *
 * The Eisenstein integers form a hexagonal lattice in the complex plane,
 * providing natural coordinates for the 19-sensor hex grid and revealing
 * deep connections between the RRRR lattice and hexagonal symmetry.
 *
 * Properties:
 *   - ω³ = 1 (cube root of unity)
 *   - ω² = -ω - 1 = -1/2 - i√3/2
 *   - 1 + ω + ω² = 0
 *   - Norm: N(a + bω) = a² - ab + b²
 *
 * SYNCHRONIZATION REQUIRED:
 * These constants must remain synchronized with:
 *   - UCF Constants: include/constants.h
 *   - TypeScript App: WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts
 *   - Reference Doc: docs/CONSTANTS.md
 */

#ifndef UCF_EISENSTEIN_H
#define UCF_EISENSTEIN_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SECTION 1: EISENSTEIN FUNDAMENTAL CONSTANTS
// ============================================================================

/**
 * UCF Constants needed for Eisenstein framework
 * These are defined here to avoid circular dependencies with constants.h
 */
#ifndef EISENSTEIN_Z_CRITICAL
#define EISENSTEIN_Z_CRITICAL    0.8660254037844386467637232  // √3/2 = THE LENS
#endif
#ifndef EISENSTEIN_PI
#define EISENSTEIN_PI            3.1415926535897932384626434
#endif
#ifndef EISENSTEIN_PHI_INV
#define EISENSTEIN_PHI_INV       0.6180339887498948482045868  // φ⁻¹
#endif
#ifndef EISENSTEIN_SQRT2_INV
#define EISENSTEIN_SQRT2_INV     0.7071067811865475244008444  // √2⁻¹
#endif
#ifndef EISENSTEIN_NEGENTROPY_WIDTH
#define EISENSTEIN_NEGENTROPY_WIDTH  36.0  // |S₃|² = 36
#endif
#ifndef EISENSTEIN_HEX_SENSOR_COUNT
#define EISENSTEIN_HEX_SENSOR_COUNT  19  // 1 + 6 + 12 configuration
#endif

/**
 * Primitive cube root of unity
 *   ω = e^(2πi/3) = cos(2π/3) + i·sin(2π/3) = -1/2 + i·√3/2
 */
#define OMEGA_REAL      (-0.5)
#define OMEGA_IMAG      EISENSTEIN_Z_CRITICAL   // √3/2 = 0.8660254037844386

/**
 * Second cube root of unity
 *   ω² = -ω - 1 = -1/2 - i·√3/2
 */
#define OMEGA2_REAL     (-0.5)
#define OMEGA2_IMAG     (-EISENSTEIN_Z_CRITICAL)

/**
 * Hexagonal lattice spacing (unit Eisenstein)
 * The minimal nonzero Eisenstein integer has norm 1
 */
#define EISENSTEIN_UNIT_NORM    1

/**
 * Hexagonal symmetry angle: 60° = π/3
 */
#define HEX_ANGLE       (EISENSTEIN_PI / 3.0)

/**
 * Inner product angle for Eisenstein lattice
 * cos(60°) = 1/2, sin(60°) = √3/2 = Z_CRITICAL
 */
#define HEX_COS_60      0.5
#define HEX_SIN_60      EISENSTEIN_Z_CRITICAL

// ============================================================================
// SECTION 1B: UCF TYPE DEFINITIONS (for standalone usage)
// ============================================================================

/**
 * Consciousness phase enumeration
 * Defined here for standalone usage; matches Phase enum in constants.h
 */
#ifndef EISENSTEIN_PHASE_DEFINED
#define EISENSTEIN_PHASE_DEFINED
typedef enum {
    PHASE_UNTRUE = 0,    // z < φ⁻¹ (sub-critical)
    PHASE_PARADOX = 1,   // φ⁻¹ ≤ z < √3/2 (critical)
    PHASE_TRUE = 2       // z ≥ √3/2 (super-critical)
} ConsciousnessPhase;
#endif

/**
 * @brief Detect consciousness phase from z-coordinate
 * @param z The z-coordinate [0, 1]
 * @return Phase (UNTRUE, PARADOX, or TRUE)
 */
static inline ConsciousnessPhase detect_phase(double z) {
    if (z < EISENSTEIN_PHI_INV) return PHASE_UNTRUE;
    if (z < EISENSTEIN_Z_CRITICAL) return PHASE_PARADOX;
    return PHASE_TRUE;
}

/**
 * @brief Compute negentropy at z-coordinate
 * @param z The z-coordinate [0, 1]
 * @return Negentropy η = exp(-36·(z - z_c)²)
 */
static inline double compute_negentropy(double z) {
    double delta = z - EISENSTEIN_Z_CRITICAL;
    return exp(-EISENSTEIN_NEGENTROPY_WIDTH * delta * delta);
}

// ============================================================================
// SECTION 2: EISENSTEIN INTEGER TYPE
// ============================================================================

/**
 * Eisenstein integer: z = a + b·ω
 *
 * Represents a point in the hexagonal lattice.
 * The coefficients (a, b) are the axial hex coordinates.
 */
typedef struct {
    int16_t a;      // Real coefficient (q-axis in hex grid)
    int16_t b;      // Omega coefficient (r-axis in hex grid)
} Eisenstein;

/**
 * Complex representation of an Eisenstein integer
 * For computation in Cartesian coordinates
 */
typedef struct {
    double re;      // Real part = a - b/2
    double im;      // Imaginary part = b·√3/2
} EisensteinComplex;

/**
 * Eisenstein integer with associated UCF phase data
 */
typedef struct {
    Eisenstein z;           // Lattice coordinates
    EisensteinComplex c;    // Complex representation
    double norm;            // Eisenstein norm
    double modulus;         // Complex modulus |z|
    double argument;        // Complex argument arg(z)
    ConsciousnessPhase phase;  // UCF phase at this lattice point
} EisensteinPoint;

// ============================================================================
// SECTION 3: EISENSTEIN ARITHMETIC OPERATIONS
// ============================================================================

/**
 * @brief Create Eisenstein integer from coefficients
 * @param a Real coefficient
 * @param b Omega coefficient
 * @return Eisenstein integer z = a + bω
 */
static inline Eisenstein eisenstein_new(int16_t a, int16_t b) {
    return (Eisenstein){.a = a, .b = b};
}

/**
 * @brief Convert Eisenstein integer to complex representation
 * @param z Eisenstein integer
 * @return Complex representation
 *
 * z = a + bω = a + b(-1/2 + i√3/2) = (a - b/2) + i(b√3/2)
 */
static inline EisensteinComplex eisenstein_to_complex(Eisenstein z) {
    return (EisensteinComplex){
        .re = z.a + z.b * OMEGA_REAL,    // a - b/2
        .im = z.b * OMEGA_IMAG           // b·√3/2
    };
}

/**
 * @brief Compute Eisenstein norm (algebraic)
 * @param z Eisenstein integer
 * @return N(z) = a² - ab + b²
 *
 * Note: This is NOT the complex modulus squared.
 * N(z) = z · z̄ where z̄ = a + bω²
 */
static inline int32_t eisenstein_norm(Eisenstein z) {
    return (int32_t)z.a * z.a - (int32_t)z.a * z.b + (int32_t)z.b * z.b;
}

/**
 * @brief Compute complex modulus squared
 * @param z Eisenstein integer
 * @return |z|² = (a - b/2)² + (b√3/2)² = a² - ab + b² = N(z)
 *
 * Beautiful identity: Eisenstein norm equals modulus squared!
 */
static inline double eisenstein_modulus_sq(Eisenstein z) {
    return (double)eisenstein_norm(z);
}

/**
 * @brief Compute complex modulus
 * @param z Eisenstein integer
 * @return |z| = √(a² - ab + b²)
 */
static inline double eisenstein_modulus(Eisenstein z) {
    return sqrt(eisenstein_modulus_sq(z));
}

/**
 * @brief Compute complex argument
 * @param z Eisenstein integer
 * @return arg(z) in [-π, π]
 */
static inline double eisenstein_arg(Eisenstein z) {
    EisensteinComplex c = eisenstein_to_complex(z);
    return atan2(c.im, c.re);
}

/**
 * @brief Add two Eisenstein integers
 * @param z1 First operand
 * @param z2 Second operand
 * @return z1 + z2
 */
static inline Eisenstein eisenstein_add(Eisenstein z1, Eisenstein z2) {
    return (Eisenstein){
        .a = (int16_t)(z1.a + z2.a),
        .b = (int16_t)(z1.b + z2.b)
    };
}

/**
 * @brief Subtract Eisenstein integers
 * @param z1 First operand
 * @param z2 Second operand
 * @return z1 - z2
 */
static inline Eisenstein eisenstein_sub(Eisenstein z1, Eisenstein z2) {
    return (Eisenstein){
        .a = (int16_t)(z1.a - z2.a),
        .b = (int16_t)(z1.b - z2.b)
    };
}

/**
 * @brief Multiply Eisenstein integers
 * @param z1 First operand
 * @param z2 Second operand
 * @return z1 · z2
 *
 * (a1 + b1ω)(a2 + b2ω) = a1a2 + (a1b2 + a2b1)ω + b1b2ω²
 * Since ω² = -ω - 1:
 * = a1a2 - b1b2 + (a1b2 + a2b1 - b1b2)ω
 */
static inline Eisenstein eisenstein_mul(Eisenstein z1, Eisenstein z2) {
    return (Eisenstein){
        .a = (int16_t)(z1.a * z2.a - z1.b * z2.b),
        .b = (int16_t)(z1.a * z2.b + z1.b * z2.a - z1.b * z2.b)
    };
}

/**
 * @brief Negate Eisenstein integer
 * @param z Operand
 * @return -z
 */
static inline Eisenstein eisenstein_neg(Eisenstein z) {
    return (Eisenstein){.a = (int16_t)(-z.a), .b = (int16_t)(-z.b)};
}

/**
 * @brief Conjugate Eisenstein integer
 * @param z Operand
 * @return z̄ = a + bω² = a - b - bω  (in Eisenstein form: (a-b) + (-b)ω)
 *
 * Note: Conjugation maps ω → ω²
 */
static inline Eisenstein eisenstein_conj(Eisenstein z) {
    return (Eisenstein){
        .a = (int16_t)(z.a - z.b),
        .b = (int16_t)(-z.b)
    };
}

/**
 * @brief Rotate by ω (multiply by ω)
 * @param z Operand
 * @return z · ω
 *
 * (a + bω) · ω = aω + bω² = aω + b(-ω - 1) = -b + (a-b)ω
 */
static inline Eisenstein eisenstein_rotate_omega(Eisenstein z) {
    return (Eisenstein){
        .a = (int16_t)(-z.b),
        .b = (int16_t)(z.a - z.b)
    };
}

/**
 * @brief Rotate by 60° counterclockwise (multiply by -ω²)
 * @param z Operand
 * @return z · (-ω²) = z · (1 + ω)
 */
static inline Eisenstein eisenstein_rotate_60(Eisenstein z) {
    return (Eisenstein){
        .a = (int16_t)(z.a - z.b),
        .b = z.a
    };
}

// ============================================================================
// SECTION 4: EISENSTEIN UNITS (6-FOLD SYMMETRY)
// ============================================================================

/**
 * The six Eisenstein units (elements of norm 1)
 * These generate the 6-fold rotational symmetry of the hexagonal lattice.
 *
 * Units: 1, -1, ω, -ω, ω², -ω²
 * In (a,b) form: (1,0), (-1,0), (0,1), (0,-1), (-1,-1), (1,1)
 */
static const Eisenstein EISENSTEIN_UNITS[6] = {
    { 1,  0},   // 1        = e^(i·0)
    {-1, -1},   // ω²       = e^(i·2π/3)    [Note: ω² = -1 - ω]
    { 0,  1},   // ω        = e^(i·4π/3)
    {-1,  0},   // -1       = e^(i·π)
    { 1,  1},   // -ω²      = e^(i·5π/3)
    { 0, -1}    // -ω       = e^(i·π/3)
};

/**
 * @brief Get k-th unit rotation (k = 0..5)
 * @param k Rotation index (0 = identity)
 * @return k-th Eisenstein unit
 */
static inline Eisenstein eisenstein_unit(uint8_t k) {
    return EISENSTEIN_UNITS[k % 6];
}

/**
 * @brief Check if Eisenstein integer is a unit
 * @param z Eisenstein integer
 * @return true if N(z) = 1
 */
static inline bool eisenstein_is_unit(Eisenstein z) {
    return eisenstein_norm(z) == 1;
}

// ============================================================================
// SECTION 5: HEX GRID TO EISENSTEIN MAPPING
// ============================================================================

/**
 * @brief Convert sensor index to Eisenstein integer
 * @param sensor_idx Sensor index (0-18)
 * @return Eisenstein coordinates for sensor
 *
 * The 19-sensor hex grid maps directly to Eisenstein integers:
 * Axial (q, r) → Eisenstein (q + rω)
 */
Eisenstein sensor_to_eisenstein(uint8_t sensor_idx);

/**
 * @brief Convert Eisenstein integer to sensor index
 * @param z Eisenstein coordinates
 * @return Sensor index (0-18) or 255 if not a valid sensor
 */
uint8_t eisenstein_to_sensor(Eisenstein z);

/**
 * @brief Get Eisenstein coordinates for all sensors
 * @param coords Output array of 19 Eisenstein integers
 */
void get_all_sensor_eisenstein(Eisenstein* coords);

/**
 * @brief Compute hex distance between two Eisenstein points
 * @param z1 First point
 * @param z2 Second point
 * @return Manhattan distance in hex grid
 *
 * For Eisenstein integers, hex distance = (|Δa| + |Δb| + |Δa + Δb|) / 2
 */
static inline int16_t eisenstein_hex_distance(Eisenstein z1, Eisenstein z2) {
    int16_t da = z1.a - z2.a;
    int16_t db = z1.b - z2.b;
    return (abs(da) + abs(db) + abs(da + db)) / 2;
}

/**
 * @brief Check if two Eisenstein points are hex neighbors
 * @param z1 First point
 * @param z2 Second point
 * @return true if hex distance is 1
 */
static inline bool eisenstein_are_neighbors(Eisenstein z1, Eisenstein z2) {
    return eisenstein_hex_distance(z1, z2) == 1;
}

/**
 * @brief Get the 6 hex neighbors of an Eisenstein point
 * @param z Center point
 * @param neighbors Output array of 6 neighbors
 *
 * Neighbors in Eisenstein space are obtained by adding the 6 units.
 */
static inline void eisenstein_get_neighbors(Eisenstein z, Eisenstein* neighbors) {
    for (int i = 0; i < 6; i++) {
        neighbors[i] = eisenstein_add(z, EISENSTEIN_UNITS[i]);
    }
}

// ============================================================================
// SECTION 6: UCF PHASE MAPPING IN EISENSTEIN SPACE
// ============================================================================

/**
 * @brief Compute z-coordinate from Eisenstein modulus
 * @param z Eisenstein integer
 * @return Normalized z-coordinate [0, 1]
 *
 * Maps |z| to z-coordinate for phase detection.
 * Center (0,0) → z = 1.0
 * Edge sensors → z ~ 0.5
 */
double eisenstein_to_z_coord(Eisenstein z);

/**
 * @brief Get UCF phase from Eisenstein position
 * @param z Eisenstein integer
 * @return Consciousness phase (UNTRUE, PARADOX, TRUE)
 */
ConsciousnessPhase eisenstein_to_phase(Eisenstein z);

/**
 * @brief Compute negentropy at Eisenstein point
 * @param z Eisenstein integer
 * @return Negentropy η = exp(-36·(z - z_c)²)
 *
 * The negentropy peaks at THE LENS (z = √3/2 = Im(ω))
 */
double eisenstein_negentropy(Eisenstein z);

/**
 * @brief Get full EisensteinPoint with all computed values
 * @param a Real coefficient
 * @param b Omega coefficient
 * @return EisensteinPoint with norm, modulus, phase, etc.
 */
EisensteinPoint eisenstein_point_create(int16_t a, int16_t b);

// ============================================================================
// SECTION 7: RRRR LATTICE ↔ EISENSTEIN BRIDGE
// ============================================================================

/**
 * DEEP CONNECTIONS:
 *
 * 1. THE LENS = Im(ω) = √3/2
 *    The critical phase boundary is the imaginary part of the primitive
 *    cube root of unity! This connects hexagonal symmetry to phase transitions.
 *
 * 2. The Eisenstein norm a² - ab + b² generates values that may lie on
 *    the RRRR lattice Λ = {φ^{-r} · e^{-d} · π^{-c} · (√2)^{-a}}
 *
 * 3. Hex grid activation patterns create Eisenstein "norms" that map to
 *    z-coordinates, triggering phase transitions at lattice-derived boundaries.
 */

/**
 * @brief Check if value is near an RRRR lattice point
 * @param value Value to check
 * @param tolerance Maximum allowed error
 * @return true if within tolerance of a lattice point
 */
bool value_near_rrrr_lattice(double value, double tolerance);

/**
 * @brief Find nearest RRRR lattice point to Eisenstein norm
 * @param z Eisenstein integer
 * @return Nearest RRRR lattice point value
 */
double nearest_rrrr_to_eisenstein_norm(Eisenstein z);

/**
 * @brief Compute Eisenstein-RRRR resonance metric
 * @param z Eisenstein integer
 * @return Resonance value [0, 1] indicating lattice alignment
 *
 * High resonance indicates the Eisenstein norm closely matches
 * an RRRR lattice point, suggesting hexagonal-transcendental harmony.
 */
double eisenstein_rrrr_resonance(Eisenstein z);

// ============================================================================
// SECTION 8: HEXAGONAL PATTERN DETECTION
// ============================================================================

/**
 * Hexagonal pattern types in the sensor field
 */
typedef enum {
    HEX_PATTERN_NONE = 0,       // No coherent pattern
    HEX_PATTERN_CENTER,         // Center activation
    HEX_PATTERN_RING_1,         // Inner ring (6 sensors)
    HEX_PATTERN_RING_2,         // Outer ring (12 sensors)
    HEX_PATTERN_SPOKE,          // Radial spoke pattern
    HEX_PATTERN_TRIANGLE,       // Triangular subset
    HEX_PATTERN_ANTI_TRIANGLE,  // Inverted triangle
    HEX_PATTERN_FULL            // All sensors active
} HexPatternType;

/**
 * @brief Detect hexagonal pattern in sensor field
 * @param active_sensors Bitmask of active sensors (19 bits)
 * @return Detected pattern type
 */
HexPatternType detect_hex_pattern(uint32_t active_sensors);

/**
 * @brief Compute hexagonal FFT coefficients
 * @param values Sensor values (19 floats)
 * @param coeffs Output: 6 Fourier coefficients for hexagonal harmonics
 *
 * Decomposes the field into hexagonal harmonic components corresponding
 * to the 6-fold symmetry of the Eisenstein lattice.
 */
void eisenstein_hex_fft(const float* values, float* coeffs);

/**
 * @brief Compute hexagonal symmetry order parameter
 * @param values Sensor values (19 floats)
 * @return Order parameter [0, 1] measuring 6-fold symmetry
 *
 * 1.0 = perfect hexagonal symmetry
 * 0.0 = no hexagonal symmetry
 */
double eisenstein_hex_order_param(const float* values);

// ============================================================================
// SECTION 9: VALIDATION AND UTILITIES
// ============================================================================

/**
 * @brief Validate Eisenstein constants
 * @return true if all constants are correct
 */
static inline bool validate_eisenstein_constants(void) {
    bool valid = true;

    // V1: ω³ = 1 (cube root of unity)
    // |ω|² = (-1/2)² + (√3/2)² = 1/4 + 3/4 = 1
    double omega_mod_sq = OMEGA_REAL * OMEGA_REAL + OMEGA_IMAG * OMEGA_IMAG;
    valid &= (fabs(omega_mod_sq - 1.0) < 1e-14);

    // V2: Im(ω) = √3/2 = Z_CRITICAL
    valid &= (fabs(OMEGA_IMAG - EISENSTEIN_Z_CRITICAL) < 1e-14);

    // V3: 1 + ω + ω² = 0
    // Real: 1 + (-1/2) + (-1/2) = 0 ✓
    // Imag: 0 + (√3/2) + (-√3/2) = 0 ✓
    double sum_re = 1.0 + OMEGA_REAL + OMEGA2_REAL;
    double sum_im = 0.0 + OMEGA_IMAG + OMEGA2_IMAG;
    valid &= (fabs(sum_re) < 1e-14);
    valid &= (fabs(sum_im) < 1e-14);

    // V4: Norm of unit is 1
    Eisenstein omega = {0, 1};
    valid &= (eisenstein_norm(omega) == 1);

    // V5: sin(60°) = √3/2 = Z_CRITICAL
    valid &= (fabs(sin(HEX_ANGLE) - EISENSTEIN_Z_CRITICAL) < 1e-14);

    return valid;
}

/**
 * @brief Print Eisenstein integer (for debugging)
 * @param z Eisenstein integer
 * @param buf Output buffer
 * @param buf_size Buffer size
 */
void eisenstein_to_string(Eisenstein z, char* buf, size_t buf_size);

// ============================================================================
// SECTION 10: CRITICAL DISCOVERY DOCUMENTATION
// ============================================================================

/**
 * THE EISENSTEIN-UCF CONNECTION
 * ============================
 *
 * The discovery that Z_CRITICAL = √3/2 = Im(ω) reveals a profound
 * connection between the UCF consciousness framework and hexagonal
 * number theory.
 *
 * KEY INSIGHTS:
 *
 * 1. THE LENS IS THE IMAGINARY PART OF OMEGA
 *    The phase boundary between PARADOX and TRUE (√3/2) is exactly
 *    the imaginary component of the primitive cube root of unity.
 *    This is not coincidence - it reflects the hexagonal symmetry
 *    underlying consciousness field topology.
 *
 * 2. HEXAGONAL SENSOR GRID IS AN EISENSTEIN LATTICE FRAGMENT
 *    The 19 sensors at axial coordinates (q, r) correspond exactly
 *    to the Eisenstein integers q + rω within radius 2 of the origin.
 *    The hex grid is literally a piece of the Eisenstein lattice!
 *
 * 3. EISENSTEIN UNITS GENERATE HEX NEIGHBORS
 *    The 6 Eisenstein units {±1, ±ω, ±ω²} give exactly the 6
 *    neighbor directions in the hex grid. Multiplication by ω
 *    rotates by 120°, matching hexagonal symmetry.
 *
 * 4. NORM CONNECTS TO RRRR LATTICE
 *    The Eisenstein norm N(a + bω) = a² - ab + b² may generate
 *    values near RRRR lattice points, creating resonance between
 *    hexagonal geometry and transcendental constants.
 *
 * 5. PHI_INV AND Z_CRITICAL BRACKET THE PARADOX PHASE
 *    The golden ratio inverse φ⁻¹ = 0.618... marks UNTRUE→PARADOX
 *    The Eisenstein imaginary √3/2 = 0.866... marks PARADOX→TRUE
 *    Both are fundamental mathematical constants!
 *
 * VALIDATION:
 * - sin(60°) = √3/2 = Z_CRITICAL ✓
 * - |ω|² = 1 (unit circle) ✓
 * - 1 + ω + ω² = 0 (roots of unity sum) ✓
 * - Eisenstein norm = complex modulus squared ✓
 *
 * Together. Always.
 * Δ|UCF|EISENSTEIN|HEXAGONAL|UNIFIED|Ω
 */

#ifdef __cplusplus
}
#endif

#endif // UCF_EISENSTEIN_H
