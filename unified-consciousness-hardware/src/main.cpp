/**
 * @file main.cpp
 * @brief UCF Hardware - Main Entry Point
 *
 * Unified Consciousness Field Hardware Implementation
 *
 * This firmware integrates all UCF modules:
 * - Hex Grid: 19-sensor capacitive array
 * - Phase Engine: z-coordinate and phase detection
 * - TRIAD FSM: Hysteresis unlock system
 * - K-Formation: Meta-cognition detection
 * - Sigil ROM: 121 neural sigils in EEPROM
 * - Emanation: Audio and visual output
 * - Omni-Linguistics: APL translation engine
 * - Photonic Capture: Interference pattern encoding
 * - Kuramoto Stabilizer: Oscillator synchronization
 *
 * Hardware Target: ESP32-WROOM-32
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "constants.h"
#include "hex_grid.h"
#include "phase_engine.h"
#include "triad_fsm.h"
#include "k_formation.h"
#include "sigil_rom.h"
#include "emanation.h"
#include "omni_linguistics.h"
#include "photonic_capture.h"
#include "kuramoto_stabilizer.h"

using namespace UCF;

// ============================================================================
// GLOBAL MODULE INSTANCES
// ============================================================================

HexGrid hexGrid;
PhaseEngine phaseEngine;
TriadFSM triadFSM;
KFormation kFormation;
SigilROM sigilROM;
Emanation emanation;
OmniLinguistics omniLing;
PhotonicCapture photonic;
KuramotoStabilizer kuramoto;

// ============================================================================
// TIMING
// ============================================================================

uint32_t lastSensorUpdate = 0;
uint32_t lastPhaseUpdate = 0;
uint32_t lastKuramotoStep = 0;
uint32_t lastEmanationUpdate = 0;
uint32_t lastSerialPrint = 0;

// ============================================================================
// STATE
// ============================================================================

HexFieldState currentField;
bool systemReady = false;

// ============================================================================
// CALLBACKS
// ============================================================================

void onTriadUnlock() {
    Serial.println(">>> TRIAD UNLOCKED <<<");

    // Special emanation for TRIAD unlock
    emanation.setPattern(LedPattern::PULSE);
    emanation.setFrequency(Solfeggio::SOL);  // 528 Hz - Love frequency
}

void onKFormation(const KFormationMetrics& metrics) {
    Serial.println(">>> K-FORMATION ACHIEVED <<<");
    Serial.printf("    kappa=%.3f eta=%.3f R=%d\n",
                  metrics.kappa, metrics.eta, metrics.R);

    // Special pattern for K-Formation
    emanation.setPattern(LedPattern::SPIRAL);
    emanation.setFrequency(Solfeggio::RE_HIGH);  // 963 Hz
}

void onSynchronization(float order_param) {
    Serial.printf(">>> SYNCHRONIZED r=%.3f <<<\n", order_param);
    emanation.setPattern(LedPattern::INTERFERENCE);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void setup() {
    // Serial for debugging
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        delay(10);
    }

    Serial.println();
    Serial.println("========================================");
    Serial.println("  UCF Hardware - Unified Consciousness");
    Serial.println("  Field Implementation v1.0");
    Serial.println("========================================");
    Serial.println();

    // Initialize I2C
    Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
    Wire.setClock(400000);  // 400 kHz

    // Initialize SPI
    SPI.begin(Pins::SPI_SCK, Pins::SPI_MISO, Pins::SPI_MOSI);
    pinMode(Pins::SPI_CS_EEPROM, OUTPUT);
    pinMode(Pins::SPI_CS_DIGIPOT, OUTPUT);
    digitalWrite(Pins::SPI_CS_EEPROM, HIGH);
    digitalWrite(Pins::SPI_CS_DIGIPOT, HIGH);

    // Initialize modules
    Serial.print("Initializing Hex Grid... ");
    if (hexGrid.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Calibrating sensors... ");
    hexGrid.calibrate(50);
    Serial.println("OK");

    Serial.print("Initializing Phase Engine... ");
    if (phaseEngine.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing TRIAD FSM... ");
    if (triadFSM.begin()) {
        triadFSM.onUnlock(onTriadUnlock);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing K-Formation... ");
    if (kFormation.begin()) {
        kFormation.onKFormation(onKFormation);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing Sigil ROM... ");
    if (sigilROM.begin()) {
        Serial.println("OK");
        if (!sigilROM.isInitialized()) {
            Serial.print("  Writing default sigils... ");
            sigilROM.initializeDefaults();
            Serial.println("OK");
        }
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing Emanation... ");
    if (emanation.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing Omni-Linguistics... ");
    if (omniLing.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing Photonic Capture... ");
    if (photonic.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    Serial.print("Initializing Kuramoto Stabilizer... ");
    if (kuramoto.begin(10.0f)) {  // 10 Hz base frequency
        kuramoto.onSynchronization(onSynchronization);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }

    // Set default emanation state
    emanation.setOutputMode(OutputMode::VISUAL | OutputMode::AUDIO);
    emanation.setPattern(LedPattern::BREATHE);
    emanation.setBrightness(128);

    Serial.println();
    Serial.println("System ready.");
    Serial.println("Sacred constants:");
    Serial.printf("  PHI = %.10f\n", PHI);
    Serial.printf("  PHI_INV = %.10f\n", PHI_INV);
    Serial.printf("  Z_CRITICAL = %.10f\n", Z_CRITICAL);
    Serial.println();

    systemReady = true;
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    if (!systemReady) {
        delay(100);
        return;
    }

    uint32_t now = millis();

    // ========================================================================
    // SENSOR READING (100 Hz)
    // ========================================================================
    if (now - lastSensorUpdate >= Timing::SENSOR_POLL_INTERVAL) {
        lastSensorUpdate = now;

        // Read hex grid
        currentField = hexGrid.readField();

        // Update phase engine
        phaseEngine.update(currentField);

        // Update K-Formation
        kFormation.update(currentField);

        // Update TRIAD FSM with coherence
        float coherence = kFormation.getKappa();
        triadFSM.update(coherence);

        // Update omni-linguistics context
        omniLing.setZContext(currentField.z);

        // Process field through linguistics
        APLToken token = omniLing.processField(currentField);

        // Update Kuramoto from z
        kuramoto.updateFromZ(currentField.z);
    }

    // ========================================================================
    // KURAMOTO STEP (1000 Hz)
    // ========================================================================
    if (now - lastKuramotoStep >= Timing::KURAMOTO_STEP_INTERVAL) {
        float dt = (now - lastKuramotoStep) / 1000.0f;
        lastKuramotoStep = now;

        // Apply magnetic modulation
        float K = kuramoto.applyMagneticModulation(Q_KAPPA);
        kuramoto.setCoupling(K);

        // Advance simulation
        kuramoto.step(dt);
    }

    // ========================================================================
    // EMANATION UPDATE (10 Hz)
    // ========================================================================
    if (now - lastEmanationUpdate >= Timing::EMANATION_UPDATE_INTERVAL) {
        lastEmanationUpdate = now;

        // Update emanation from phase state
        emanation.update(phaseEngine.getState());

        // If K-Formation, use special pattern
        if (kFormation.isActive()) {
            PhotonicPattern pattern = photonic.generateLIMNUS(currentField.z);
            photonic.displayPattern(pattern);
        }
    }

    // ========================================================================
    // PHASE UPDATE (20 Hz)
    // ========================================================================
    if (now - lastPhaseUpdate >= Timing::PHASE_UPDATE_INTERVAL) {
        lastPhaseUpdate = now;

        // Update phase indicators
        phaseEngine.updateIndicators();
        triadFSM.updateIndicator();
        kFormation.updateIndicator();

        // Check for sigil match
        SigilMatch match = sigilROM.findMatchingSigil(currentField);
        if (match.confidence > 0.8f) {
            NeuralSigil sigil;
            sigilROM.readSigil(match.sigil_index, sigil);
            emanation.setFromSigil(sigil);
        }
    }

    // ========================================================================
    // SERIAL OUTPUT (1 Hz)
    // ========================================================================
    if (now - lastSerialPrint >= 1000) {
        lastSerialPrint = now;

        const PhaseState& ps = phaseEngine.getState();
        const KFormationMetrics& kf = kFormation.getMetrics();
        const KuramotoState& ks = kuramoto.getState();

        Serial.printf("z=%.3f t%d %s | kappa=%.3f eta=%.3f R=%d | r=%.3f %s%s\n",
                      ps.z,
                      ps.tier,
                      phaseToString(ps.current),
                      kf.kappa,
                      kf.eta,
                      kf.R,
                      ks.order_param,
                      triadFSM.isUnlocked() ? "TRIAD " : "",
                      kFormation.isActive() ? "K-FORM" : "");
    }

    // ========================================================================
    // SERIAL COMMAND PROCESSING
    // ========================================================================
    if (Serial.available()) {
        char cmd = Serial.read();

        switch (cmd) {
            case 'r':  // Reset
                Serial.println("Resetting system...");
                hexGrid.calibrate(50);
                triadFSM.reset();
                kFormation.resetStats();
                kuramoto.reset();
                break;

            case 's':  // Status
                printDetailedStatus();
                break;

            case 'p':  // Pattern cycle
                cycleEmanationPattern();
                break;

            case '+':  // Increase coupling
                {
                    float K = kuramoto.getCoupling() + 0.05f;
                    if (K > 1.0f) K = 1.0f;
                    kuramoto.setCoupling(K);
                    Serial.printf("Coupling: %.2f\n", K);
                }
                break;

            case '-':  // Decrease coupling
                {
                    float K = kuramoto.getCoupling() - 0.05f;
                    if (K < 0.1f) K = 0.1f;
                    kuramoto.setCoupling(K);
                    Serial.printf("Coupling: %.2f\n", K);
                }
                break;

            case 't':  // Force TRIAD unlock (testing)
                triadFSM.forceUnlock();
                break;

            case 'l':  // List sigils
                listSigils();
                break;

            case '?':  // Help
                printHelp();
                break;
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void printDetailedStatus() {
    Serial.println("\n===== DETAILED STATUS =====\n");

    // Hex Grid
    Serial.println("-- Hex Grid --");
    Serial.printf("  Active sensors: %d\n", currentField.active_count);
    Serial.printf("  Total energy: %.2f\n", currentField.total_energy);
    Serial.printf("  Centroid: (%.2f, %.2f)\n", currentField.centroid_x, currentField.centroid_y);

    // Phase Engine
    const PhaseState& ps = phaseEngine.getState();
    Serial.println("\n-- Phase Engine --");
    Serial.printf("  z: %.4f (smoothed: %.4f)\n", ps.z, ps.z_smoothed);
    Serial.printf("  Tier: %d\n", ps.tier);
    Serial.printf("  Phase: %s\n", phaseToString(ps.current));
    Serial.printf("  Stable: %s\n", ps.is_stable ? "yes" : "no");
    Serial.printf("  Frequency: %d Hz\n", phaseEngine.getCurrentFrequency());

    // TRIAD FSM
    const TriadStatus& ts = triadFSM.getStatus();
    Serial.println("\n-- TRIAD FSM --");
    Serial.printf("  State: %s\n", triadStateToString(ts.state));
    Serial.printf("  Crossings: %d/%d\n", ts.crossing_count, TRIAD_PASSES_REQUIRED);
    Serial.printf("  Unlocked: %s\n", ts.is_unlocked ? "YES" : "no");

    // K-Formation
    const KFormationMetrics& kf = kFormation.getMetrics();
    const KFormationStatus& kfs = kFormation.getStatus();
    Serial.println("\n-- K-Formation --");
    Serial.printf("  Kappa: %.3f %s\n", kf.kappa, kf.kappa_satisfied ? "[OK]" : "");
    Serial.printf("  Eta: %.3f %s\n", kf.eta, kf.eta_satisfied ? "[OK]" : "");
    Serial.printf("  R: %d %s\n", kf.R, kf.R_satisfied ? "[OK]" : "");
    Serial.printf("  Active: %s\n", kfs.is_active ? "YES" : "no");
    Serial.printf("  Total formations: %d\n", kfs.total_formations);

    // Kuramoto
    const KuramotoState& ks = kuramoto.getState();
    const StabilizationStatus& ss = kuramoto.getStatus();
    Serial.println("\n-- Kuramoto Stabilizer --");
    Serial.printf("  Order param r: %.3f\n", ks.order_param);
    Serial.printf("  Coupling K: %.3f\n", ks.coupling);
    Serial.printf("  Collective phase: %.2f rad\n", ks.collective_phase);
    Serial.printf("  Synchronized: %s\n", ss.is_synchronized ? "YES" : "no");
    Serial.printf("  PLL locked: %s\n", ss.pll_locked ? "YES" : "no");

    // Magnetic field
    Serial.println("\n-- Magnetic Field --");
    Serial.printf("  |B|: %.2f uT\n", ss.magnetic.magnitude);
    Serial.printf("  (x,y,z): (%.1f, %.1f, %.1f) uT\n",
                  ss.magnetic.x, ss.magnetic.y, ss.magnetic.z);

    Serial.println("\n===========================\n");
}

void cycleEmanationPattern() {
    static uint8_t patternIndex = 0;
    const LedPattern patterns[] = {
        LedPattern::SOLID,
        LedPattern::BREATHE,
        LedPattern::PULSE,
        LedPattern::WAVE,
        LedPattern::SPIRAL,
        LedPattern::INTERFERENCE
    };
    const char* names[] = {
        "SOLID", "BREATHE", "PULSE", "WAVE", "SPIRAL", "INTERFERENCE"
    };

    patternIndex = (patternIndex + 1) % 6;
    emanation.setPattern(patterns[patternIndex]);
    Serial.printf("Pattern: %s\n", names[patternIndex]);
}

void listSigils() {
    Serial.println("\n-- Neural Sigils (first 10) --\n");
    for (uint8_t i = 0; i < 10; i++) {
        NeuralSigil sigil;
        sigilROM.readSigil(i, sigil);
        sigilROM.debugPrintSigil(sigil);
    }
    Serial.printf("\n... and %d more\n", SIGIL_COUNT - 10);
}

void printHelp() {
    Serial.println("\n-- Commands --");
    Serial.println("  r  : Reset/recalibrate");
    Serial.println("  s  : Detailed status");
    Serial.println("  p  : Cycle LED pattern");
    Serial.println("  +  : Increase coupling");
    Serial.println("  -  : Decrease coupling");
    Serial.println("  t  : Force TRIAD unlock");
    Serial.println("  l  : List sigils");
    Serial.println("  ?  : This help");
    Serial.println();
}
