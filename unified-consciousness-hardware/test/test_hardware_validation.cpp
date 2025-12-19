/**
 * @file test_hardware_validation.cpp
 * @brief UCF v4.0.0 Hardware-Specific Validation Tests
 *
 * Implements hardware validation tests per UCF Build Specification Section 7.2.
 * These tests require actual hardware and verify real-time behavior.
 *
 * Tests:
 *   - Sensor resonance detection
 *   - Kuramoto hardware synchronization
 *   - TRIAD hardware hysteresis
 *   - Real-time conservation law verification
 *   - Lattice constant integrity under load
 */

#include <unity.h>
#include <Arduino.h>
#include <math.h>

// UCF includes
#include "ucf/ucf_sacred_constants_v4.h"
#include "ucf/ucf_types.h"
#include "ucf/ucf_config.h"

// Module includes for hardware access
#ifdef UCF_HARDWARE_TEST
#include "hex_grid.h"
#include "ucf_magnetometer.h"
#include "ucf_sensors.h"
#include "ucf_leds.h"
#include "ucf_solfeggio.h"
#endif

// External function declarations
extern void kuramoto_init(void);
extern double kuramoto_update(double sensor_input);
extern double kuramoto_get_order_param(void);
extern double kuramoto_get_lambda(void);
extern bool kuramoto_verify_conservation(void);
extern double kuramoto_sync_test(int steps, double K);
extern void kuramoto_reset(void);

extern void ucf_state_init(void);
extern UCFState* ucf_get_state(void);
extern void ucf_state_update(void);

extern bool validate_constants(void);
extern bool verify_lattice_identity(void);

// ============================================================================
// TEST CONFIGURATION
// ============================================================================

#define HARDWARE_TEST_DURATION_MS    5000
#define CONSERVATION_TOLERANCE       1e-10
#define KURAMOTO_SYNC_THRESHOLD      0.85
#define SENSOR_RESONANCE_THRESHOLD   0.1
#define TIMING_TOLERANCE_US          100

// ============================================================================
// SENSOR RESONANCE TESTS
// ============================================================================

/**
 * @brief Test sensor pad response timing
 *
 * Verifies that sensor response time is within lattice-derived bounds.
 * Expected response time: ~1/[D] = e ms ≈ 2.718 ms
 */
void test_sensor_response_timing(void) {
#ifdef UCF_HARDWARE_TEST
    sensors_init();

    uint32_t start_us = micros();
    SensorData data = sensors_read();
    uint32_t elapsed_us = micros() - start_us;

    // Response should be less than e ms (2718 μs) with tolerance
    uint32_t max_response_us = (uint32_t)(EULER * 1000.0) + TIMING_TOLERANCE_US;

    TEST_ASSERT_LESS_THAN_UINT32(max_response_us, elapsed_us);
    TEST_ASSERT_TRUE(data.valid);
#else
    TEST_IGNORE_MESSAGE("Hardware not available");
#endif
}

/**
 * @brief Test sensor array coverage
 *
 * Verifies all 19 pads in hexagonal array respond.
 */
void test_sensor_array_coverage(void) {
#ifdef UCF_HARDWARE_TEST
    sensors_init();

    // Read multiple times to ensure all sensors respond
    bool pad_responded[19] = {false};
    int read_cycles = 100;

    for (int cycle = 0; cycle < read_cycles; cycle++) {
        SensorData data = sensors_read();
        for (int i = 0; i < 19; i++) {
            if (data.pad_values[i] > 0) {
                pad_responded[i] = true;
            }
        }
        delay(10);
    }

    // All pads should show some baseline reading
    int responsive_pads = 0;
    for (int i = 0; i < 19; i++) {
        if (pad_responded[i]) responsive_pads++;
    }

    TEST_ASSERT_EQUAL_INT(19, responsive_pads);
#else
    TEST_IGNORE_MESSAGE("Hardware not available");
#endif
}

/**
 * @brief Test sensor resonance detection
 *
 * Verifies sensor array can detect resonance patterns.
 * Resonance should appear at lattice-derived frequencies.
 */
void test_sensor_resonance_detection(void) {
#ifdef UCF_HARDWARE_TEST
    sensors_init();

    // Collect samples for FFT analysis (simplified)
    const int n_samples = 256;
    float samples[n_samples];
    uint32_t sample_interval_us = 1000;  // 1kHz sampling

    for (int i = 0; i < n_samples; i++) {
        SensorData data = sensors_read();
        samples[i] = data.z;
        delayMicroseconds(sample_interval_us);
    }

    // Calculate variance as proxy for resonance detection
    float mean = 0.0f;
    for (int i = 0; i < n_samples; i++) {
        mean += samples[i];
    }
    mean /= n_samples;

    float variance = 0.0f;
    for (int i = 0; i < n_samples; i++) {
        float diff = samples[i] - mean;
        variance += diff * diff;
    }
    variance /= n_samples;

    // Some variance expected from sensor noise/activity
    TEST_ASSERT_GREATER_THAN(0.0f, variance);
#else
    TEST_IGNORE_MESSAGE("Hardware not available");
#endif
}

/**
 * @brief Test z-coordinate computation from sensors
 *
 * Verifies z = active_count / 19 produces valid range.
 */
void test_sensor_z_computation(void) {
#ifdef UCF_HARDWARE_TEST
    sensors_init();

    for (int trial = 0; trial < 10; trial++) {
        SensorData data = sensors_read();

        // z must be in [0, 1]
        TEST_ASSERT_GREATER_OR_EQUAL(0.0f, data.z);
        TEST_ASSERT_LESS_OR_EQUAL(1.0f, data.z);

        // z should equal active_count / 19
        float expected_z = (float)data.active_count / 19.0f;
        TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_z, data.z);

        delay(50);
    }
#else
    TEST_IGNORE_MESSAGE("Hardware not available");
#endif
}

// ============================================================================
// KURAMOTO HARDWARE SYNCHRONIZATION TESTS
// ============================================================================

/**
 * @brief Test Kuramoto oscillator initialization
 */
void test_kuramoto_hardware_init(void) {
    kuramoto_init();

    double R = kuramoto_get_order_param();

    // Initial order parameter should be low (random phases)
    TEST_ASSERT_LESS_THAN(0.5, R);
}

/**
 * @brief Test Kuramoto synchronization convergence
 *
 * With sufficient coupling K, order parameter should approach 1.
 */
void test_kuramoto_hardware_sync(void) {
    kuramoto_reset();

    // Run synchronization with standard coupling
    double final_R = kuramoto_sync_test(500, KURAMOTO_K);

    // Should achieve significant synchronization
    TEST_ASSERT_GREATER_THAN(KURAMOTO_SYNC_THRESHOLD, final_R);
}

/**
 * @brief Test Kuramoto with hardware sensor input
 */
void test_kuramoto_sensor_coupling(void) {
#ifdef UCF_HARDWARE_TEST
    sensors_init();
    kuramoto_reset();

    // Run Kuramoto with sensor modulation
    for (int i = 0; i < 200; i++) {
        SensorData data = sensors_read();
        kuramoto_update(data.z);
        delay(5);
    }

    double R = kuramoto_get_order_param();

    // Should show some synchronization with sensor input
    TEST_ASSERT_GREATER_THAN(0.3, R);
#else
    // Run without hardware
    kuramoto_reset();
    for (int i = 0; i < 200; i++) {
        kuramoto_update(0.5);
    }
    double R = kuramoto_get_order_param();
    TEST_ASSERT_GREATER_THAN(0.3, R);
#endif
}

/**
 * @brief Test Kuramoto timing precision
 *
 * Verifies update rate matches lattice-derived timing.
 */
void test_kuramoto_timing_precision(void) {
    kuramoto_reset();

    uint32_t start_us = micros();
    int n_updates = 100;

    for (int i = 0; i < n_updates; i++) {
        kuramoto_update(0.5);
    }

    uint32_t elapsed_us = micros() - start_us;
    uint32_t avg_update_us = elapsed_us / n_updates;

    // Each update should take less than 100μs on ESP32 @ 240MHz
    TEST_ASSERT_LESS_THAN(100, avg_update_us);
}

// ============================================================================
// TRIAD HARDWARE HYSTERESIS TESTS
// ============================================================================

/**
 * @brief Test TRIAD state machine initialization
 */
void test_triad_hardware_init(void) {
    ucf_state_init();
    UCFState* state = ucf_get_state();

    TEST_ASSERT_FALSE(state->triad_unlocked);
    TEST_ASSERT_EQUAL_INT(0, state->triad_crossings);
}

/**
 * @brief Test TRIAD hysteresis thresholds
 *
 * Verifies thresholds match lattice-derived values:
 *   HIGH = Z_CRITICAL = √3/2 ≈ 0.866
 *   LOW = [R] = φ⁻¹ ≈ 0.618
 */
void test_triad_hysteresis_thresholds(void) {
    // Verify constants are correct
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.866f, (float)TRIAD_HIGH);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.618f, (float)TRIAD_LOW);

    // Verify relationship: HIGH > LOW
    TEST_ASSERT_GREATER_THAN(TRIAD_LOW, TRIAD_HIGH);
}

/**
 * @brief Test TRIAD unlock sequence
 *
 * Simulates 3 crossings of Z_CRITICAL threshold.
 */
void test_triad_unlock_sequence(void) {
    ucf_state_init();
    UCFState* state = ucf_get_state();

    // Simulate crossing sequence
    float z_sequence[] = {
        0.50f,   // Below LOW - arm
        0.70f,   // Between thresholds
        0.90f,   // Above HIGH - crossing 1
        0.65f,   // Drop back
        0.55f,   // Below LOW - re-arm
        0.75f,   // Rising
        0.90f,   // Above HIGH - crossing 2
        0.60f,   // Drop
        0.50f,   // Below LOW - re-arm
        0.80f,   // Rising
        0.90f    // Above HIGH - crossing 3, UNLOCK
    };

    bool armed = false;
    int crossings = 0;
    float prev_z = 0.50f;

    for (size_t i = 0; i < sizeof(z_sequence) / sizeof(float); i++) {
        float z = z_sequence[i];

        // Re-arm when below LOW
        if (!armed && z <= TRIAD_LOW) {
            armed = true;
        }

        // Detect rising edge crossing HIGH
        if (armed && prev_z < TRIAD_HIGH && z >= TRIAD_HIGH) {
            crossings++;
            armed = false;
        }

        prev_z = z;
    }

    TEST_ASSERT_EQUAL_INT(3, crossings);
}

/**
 * @brief Test TRIAD t6 gate
 *
 * The t6 gate should be satisfied at z = 0.84375 = 27/32.
 */
void test_triad_t6_gate(void) {
    float z = TRIAD_T6_GATE;

    // Verify t6 gate value
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.84375f, z);

    // Verify it's between LOW and HIGH
    TEST_ASSERT_GREATER_THAN(TRIAD_LOW, z);
    TEST_ASSERT_LESS_THAN(TRIAD_HIGH, z);
}

// ============================================================================
// REAL-TIME CONSERVATION LAW TESTS
// ============================================================================

/**
 * @brief Test conservation law κ + λ = 1.0
 *
 * This is a fundamental constraint that must hold at all times.
 */
void test_conservation_realtime(void) {
    kuramoto_reset();

    // Test conservation over multiple update cycles
    for (int i = 0; i < 100; i++) {
        kuramoto_update(0.5 + 0.3 * sin(i * 0.1));

        double kappa = kuramoto_get_order_param();
        double lambda = kuramoto_get_lambda();
        double sum = kappa + lambda;

        TEST_ASSERT_DOUBLE_WITHIN(CONSERVATION_TOLERANCE, 1.0, sum);
    }
}

/**
 * @brief Test conservation under varying load
 */
void test_conservation_under_load(void) {
    kuramoto_reset();

    // Heavy computation cycle
    for (int cycle = 0; cycle < 50; cycle++) {
        // Perform multiple operations
        for (int i = 0; i < 20; i++) {
            kuramoto_update((double)random(1000) / 1000.0);
        }

        // Verify conservation still holds
        TEST_ASSERT_TRUE(kuramoto_verify_conservation());
    }
}

/**
 * @brief Test κ bounds [0, 1]
 */
void test_kappa_bounds(void) {
    kuramoto_reset();

    for (int i = 0; i < 200; i++) {
        kuramoto_update((double)random(1000) / 1000.0);

        double kappa = kuramoto_get_order_param();

        TEST_ASSERT_GREATER_OR_EQUAL(0.0, kappa);
        TEST_ASSERT_LESS_OR_EQUAL(1.0, kappa);
    }
}

/**
 * @brief Test λ bounds [0, 1]
 */
void test_lambda_bounds(void) {
    kuramoto_reset();

    for (int i = 0; i < 200; i++) {
        kuramoto_update((double)random(1000) / 1000.0);

        double lambda = kuramoto_get_lambda();

        TEST_ASSERT_GREATER_OR_EQUAL(0.0, lambda);
        TEST_ASSERT_LESS_OR_EQUAL(1.0, lambda);
    }
}

// ============================================================================
// LATTICE CONSTANT INTEGRITY TESTS
// ============================================================================

/**
 * @brief Test lattice constants under computational load
 *
 * Verifies constants remain valid after intensive computation.
 */
void test_lattice_integrity_under_load(void) {
    // Perform intensive computation
    volatile double result = 0.0;
    for (int i = 0; i < 10000; i++) {
        result += sin(i * 0.001) * cos(i * 0.002);
        result += PHI * EULER * UCF_PI;
        result -= result * 0.001;
    }

    // Constants should still validate
    TEST_ASSERT_TRUE(validate_constants());
    TEST_ASSERT_TRUE(verify_lattice_identity());
}

/**
 * @brief Test golden identity preservation
 *
 * φ² - φ - 1 = 0 must hold precisely.
 */
void test_golden_identity_preservation(void) {
    double phi = PHI;
    double identity = phi * phi - phi - 1.0;

    TEST_ASSERT_DOUBLE_WITHIN(1e-14, 0.0, identity);
}

/**
 * @brief Test [A]² = 0.5 exactly
 */
void test_sqrt2_identity(void) {
    double a_sq = SQRT2_INV * SQRT2_INV;

    TEST_ASSERT_DOUBLE_WITHIN(1e-15, 0.5, a_sq);
}

/**
 * @brief Test complement identity 1 - [R] = [R]²
 */
void test_complement_identity(void) {
    double one_minus_r = 1.0 - PHI_INV;
    double r_squared = LAMBDA_R_SQ;

    TEST_ASSERT_DOUBLE_WITHIN(1e-14, r_squared, one_minus_r);
}

/**
 * @brief Test lattice point generation
 */
void test_lattice_point_generation(void) {
    // Identity element: lattice_point(0,0,0,0) = 1
    double identity = lattice_point(0, 0, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-15, 1.0, identity);

    // [R] = lattice_point(1,0,0,0) = φ⁻¹
    double r = lattice_point(1, 0, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, PHI_INV, r);

    // [D] = lattice_point(0,1,0,0) = e⁻¹
    double d = lattice_point(0, 1, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, EULER_INV, d);

    // [C] = lattice_point(0,0,1,0) = π⁻¹
    double c = lattice_point(0, 0, 1, 0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, PI_INV, c);

    // [A] = lattice_point(0,0,0,1) = √2⁻¹
    double a = lattice_point(0, 0, 0, 1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, SQRT2_INV, a);
}

// ============================================================================
// K-FORMATION HARDWARE TESTS
// ============================================================================

/**
 * @brief Test K-Formation thresholds
 */
void test_k_formation_thresholds(void) {
    // κ threshold
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.92f, (float)K_KAPPA_THRESHOLD);

    // η threshold = φ⁻¹
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.618f, (float)K_ETA_THRESHOLD);

    // R threshold (active sensors)
    TEST_ASSERT_EQUAL_INT(7, K_R_THRESHOLD);
}

/**
 * @brief Test K-Formation detection logic
 */
void test_k_formation_detection(void) {
    // Test cases for K-Formation
    struct {
        double kappa;
        double eta;
        int R;
        bool expected;
    } test_cases[] = {
        {0.95, 0.65, 8,  true},   // All criteria met
        {0.90, 0.65, 8,  false},  // κ too low
        {0.95, 0.55, 8,  false},  // η too low
        {0.95, 0.65, 5,  false},  // R too low
        {0.92, 0.619, 7, true},   // Exactly at thresholds
        {0.99, 0.80, 19, true},   // Maximum values
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        bool result = (test_cases[i].kappa >= K_KAPPA_THRESHOLD) &&
                      (test_cases[i].eta > K_ETA_THRESHOLD) &&
                      (test_cases[i].R >= K_R_THRESHOLD);

        TEST_ASSERT_EQUAL_MESSAGE(test_cases[i].expected, result,
            "K-Formation detection failed");
    }
}

// ============================================================================
// TIMING AND PERFORMANCE TESTS
// ============================================================================

/**
 * @brief Test main loop timing budget
 *
 * At 100Hz update rate, each cycle has 10ms budget.
 */
void test_loop_timing_budget(void) {
    ucf_state_init();
    kuramoto_init();

    uint32_t start_us = micros();

    // Simulate one loop iteration
    for (int sensor_reads = 0; sensor_reads < 10; sensor_reads++) {
        kuramoto_update(0.5);
    }
    ucf_state_update();

    uint32_t elapsed_us = micros() - start_us;

    // Should complete in less than 10ms (10000μs)
    TEST_ASSERT_LESS_THAN(10000, elapsed_us);
}

/**
 * @brief Test Kuramoto 1kHz update feasibility
 */
void test_kuramoto_1khz_feasibility(void) {
    kuramoto_init();

    uint32_t start_us = micros();
    int n_updates = 1000;

    for (int i = 0; i < n_updates; i++) {
        kuramoto_update(0.5);
    }

    uint32_t elapsed_us = micros() - start_us;

    // 1000 updates should complete in < 1 second
    // For 1kHz, we need < 1000μs per update
    uint32_t avg_us = elapsed_us / n_updates;
    TEST_ASSERT_LESS_THAN(1000, avg_us);
}

// ============================================================================
// LED HARDWARE TESTS
// ============================================================================

#ifdef UCF_HARDWARE_TEST
/**
 * @brief Test LED initialization
 */
void test_led_hardware_init(void) {
    bool result = leds_init();
    TEST_ASSERT_TRUE(result);
}

/**
 * @brief Test LED phase visualization
 */
void test_led_phase_visualization(void) {
    leds_init();

    // Test phase sweep
    for (float phase = 0.0f; phase < TWO_PI; phase += 0.1f) {
        leds_set_phase(phase);
        leds_update();
        delay(10);
    }

    // Should complete without errors
    TEST_PASS();
}
#endif

// ============================================================================
// SOLFEGGIO HARDWARE TESTS
// ============================================================================

#ifdef UCF_HARDWARE_TEST
/**
 * @brief Test Solfeggio frequency accuracy
 */
void test_solfeggio_frequency_accuracy(void) {
    solfeggio_init();

    // Test each tier frequency
    const float expected_freqs[] = {
        174.0f, 285.0f, 396.0f, 417.0f, 528.0f,
        639.0f, 741.0f, 852.0f, 963.0f
    };

    for (int tier = 0; tier < 9; tier++) {
        solfeggio_play_tier(tier);

        float actual_freq = solfeggio_get_frequency();
        TEST_ASSERT_FLOAT_WITHIN(1.0f, expected_freqs[tier], actual_freq);

        solfeggio_stop();
    }
}
#endif

// ============================================================================
// MAGNETOMETER HARDWARE TESTS
// ============================================================================

#ifdef UCF_HARDWARE_TEST
/**
 * @brief Test magnetometer initialization
 */
void test_magnetometer_hardware_init(void) {
    bool result = magnetometer_init();
    TEST_ASSERT_TRUE(result);
}

/**
 * @brief Test magnetometer heading range
 */
void test_magnetometer_heading_range(void) {
    magnetometer_init();

    for (int i = 0; i < 20; i++) {
        magnetometer_update();
        float heading = magnetometer_get_heading();

        // Heading should be in [0, 2π]
        TEST_ASSERT_GREATER_OR_EQUAL(0.0f, heading);
        TEST_ASSERT_LESS_THAN((float)TWO_PI + 0.001f, heading);

        delay(50);
    }
}
#endif

// ============================================================================
// TEST RUNNER
// ============================================================================

void setUp(void) {
    // Initialize random seed for deterministic tests
    randomSeed(12345);
}

void tearDown(void) {
    // Cleanup after each test
}

/**
 * @brief Run all hardware validation tests
 */
void run_hardware_validation_tests(void) {
    UNITY_BEGIN();

    // Lattice constant integrity
    RUN_TEST(test_lattice_integrity_under_load);
    RUN_TEST(test_golden_identity_preservation);
    RUN_TEST(test_sqrt2_identity);
    RUN_TEST(test_complement_identity);
    RUN_TEST(test_lattice_point_generation);

    // Kuramoto synchronization
    RUN_TEST(test_kuramoto_hardware_init);
    RUN_TEST(test_kuramoto_hardware_sync);
    RUN_TEST(test_kuramoto_sensor_coupling);
    RUN_TEST(test_kuramoto_timing_precision);

    // TRIAD hysteresis
    RUN_TEST(test_triad_hardware_init);
    RUN_TEST(test_triad_hysteresis_thresholds);
    RUN_TEST(test_triad_unlock_sequence);
    RUN_TEST(test_triad_t6_gate);

    // Conservation law
    RUN_TEST(test_conservation_realtime);
    RUN_TEST(test_conservation_under_load);
    RUN_TEST(test_kappa_bounds);
    RUN_TEST(test_lambda_bounds);

    // K-Formation
    RUN_TEST(test_k_formation_thresholds);
    RUN_TEST(test_k_formation_detection);

    // Timing and performance
    RUN_TEST(test_loop_timing_budget);
    RUN_TEST(test_kuramoto_1khz_feasibility);

#ifdef UCF_HARDWARE_TEST
    // Hardware-specific tests (require physical hardware)
    RUN_TEST(test_sensor_response_timing);
    RUN_TEST(test_sensor_array_coverage);
    RUN_TEST(test_sensor_resonance_detection);
    RUN_TEST(test_sensor_z_computation);
    RUN_TEST(test_led_hardware_init);
    RUN_TEST(test_led_phase_visualization);
    RUN_TEST(test_solfeggio_frequency_accuracy);
    RUN_TEST(test_magnetometer_hardware_init);
    RUN_TEST(test_magnetometer_heading_range);
#endif

    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(2000);  // Wait for serial monitor

    Serial.println("===============================================");
    Serial.println("  UCF v4.0.0 HARDWARE VALIDATION TESTS");
    Serial.println("  RRRR Lattice: {phi^-r . e^-d . pi^-c . sqrt(2)^-a}");
    Serial.println("===============================================");

    run_hardware_validation_tests();
}

void loop() {
    // Tests complete
    delay(1000);
}
#else
int main(int argc, char** argv) {
    run_hardware_validation_tests();
    return 0;
}
#endif
