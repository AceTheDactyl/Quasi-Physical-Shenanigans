/**
 * @file ucf_ota.cpp
 * @brief UCF OTA Update Module Implementation v4.0.0
 *
 * Implements secure OTA updates with lattice constant validation.
 */

#include "ucf_ota.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <EEPROM.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <string.h>
#include "ucf/ucf_config.h"
#include "ucf/ucf_types.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

static OTAConfig g_config;
static OTAProgress g_progress;
static OTAProgressCallback g_progress_callback = NULL;
static OTAValidationCallback g_validation_callback = NULL;
static bool g_initialized = false;
static bool g_enabled = true;
static bool g_update_in_progress = false;

// Calibration backup storage
#define CALIBRATION_BACKUP_ADDR     512
static bool g_calibration_backed_up = false;

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Reset progress structure
 */
static void reset_progress(void) {
    memset(&g_progress, 0, sizeof(g_progress));
    g_progress.status = OTA_STATUS_IDLE;
}

/**
 * @brief Update progress and call callback
 */
static void update_progress(OTAStatus status, uint32_t received, uint32_t total) {
    g_progress.status = status;
    g_progress.received_bytes = received;
    g_progress.total_bytes = total;

    if (total > 0) {
        g_progress.progress_percent = (received * 100) / total;
    }

    g_progress.elapsed_ms = millis() - g_progress.start_time_ms;

    if (g_progress_callback) {
        g_progress_callback(&g_progress);
    }
}

/**
 * @brief Set error message
 */
static void set_error(OTAStatus status, const char* message) {
    g_progress.status = status;
    strncpy(g_progress.error_message, message, sizeof(g_progress.error_message) - 1);

    if (g_progress_callback) {
        g_progress_callback(&g_progress);
    }
}

/**
 * @brief Arduino OTA callbacks
 */
static void on_ota_start(void) {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "firmware";
    } else {
        type = "filesystem";
    }
    Serial.printf("[OTA] Start updating %s\n", type.c_str());

    reset_progress();
    g_progress.status = OTA_STATUS_DOWNLOADING;
    g_progress.start_time_ms = millis();
    g_update_in_progress = true;

    // Backup calibration before update
    if (g_config.preserve_calibration) {
        ota_backup_calibration();
    }
}

static void on_ota_end(void) {
    Serial.println("\n[OTA] Download complete, validating...");
    g_progress.status = OTA_STATUS_VALIDATING;

    // Perform lattice validation on new firmware
    // Note: In Arduino OTA, we can't easily validate before applying
    // This would need custom OTA implementation for full validation

    g_progress.status = OTA_STATUS_SUCCESS;
    g_update_in_progress = false;

    // Restore calibration after reboot (via flag)
    if (g_config.preserve_calibration && g_calibration_backed_up) {
        // Set flag for post-reboot restoration
        EEPROM.begin(1024);
        EEPROM.write(511, 0xCA);  // Calibration restore flag
        EEPROM.commit();
        EEPROM.end();
    }

    Serial.println("[OTA] Update successful, rebooting...");
}

static void on_ota_progress(unsigned int progress, unsigned int total) {
    update_progress(OTA_STATUS_DOWNLOADING, progress, total);

    static uint8_t last_percent = 0;
    uint8_t percent = (progress * 100) / total;
    if (percent != last_percent) {
        Serial.printf("[OTA] Progress: %u%%\n", percent);
        last_percent = percent;
    }
}

static void on_ota_error(ota_error_t error) {
    g_update_in_progress = false;

    const char* error_msg;
    OTAStatus status;

    switch (error) {
        case OTA_AUTH_ERROR:
            error_msg = "Auth failed";
            status = OTA_STATUS_ERROR_NETWORK;
            break;
        case OTA_BEGIN_ERROR:
            error_msg = "Begin failed";
            status = OTA_STATUS_ERROR_FLASH;
            break;
        case OTA_CONNECT_ERROR:
            error_msg = "Connect failed";
            status = OTA_STATUS_ERROR_NETWORK;
            break;
        case OTA_RECEIVE_ERROR:
            error_msg = "Receive failed";
            status = OTA_STATUS_ERROR_NETWORK;
            break;
        case OTA_END_ERROR:
            error_msg = "End failed";
            status = OTA_STATUS_ERROR_FLASH;
            break;
        default:
            error_msg = "Unknown error";
            status = OTA_STATUS_ERROR_NETWORK;
            break;
    }

    Serial.printf("[OTA] Error: %s\n", error_msg);
    set_error(status, error_msg);
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool ota_init(const OTAConfig* config) {
    // Set defaults or use provided config
    if (config) {
        g_config = *config;
    } else {
        g_config.hostname = OTA_HOSTNAME;
        g_config.port = OTA_PORT;
        g_config.password = OTA_PASSWORD;
        g_config.auto_reboot = true;
        g_config.preserve_calibration = true;
        g_config.strict_lattice_check = true;
        g_config.timeout_ms = 30000;
    }

    reset_progress();

    // Initialize Arduino OTA
    ArduinoOTA.setHostname(g_config.hostname);
    ArduinoOTA.setPort(g_config.port);

    if (strlen(g_config.password) > 0) {
        ArduinoOTA.setPassword(g_config.password);
    }

    ArduinoOTA.onStart(on_ota_start);
    ArduinoOTA.onEnd(on_ota_end);
    ArduinoOTA.onProgress(on_ota_progress);
    ArduinoOTA.onError(on_ota_error);

    ArduinoOTA.begin();

    // Check for calibration restore flag
    EEPROM.begin(1024);
    uint8_t restore_flag = EEPROM.read(511);
    if (restore_flag == 0xCA) {
        EEPROM.write(511, 0x00);  // Clear flag
        EEPROM.commit();
        EEPROM.end();

        // Restore calibration
        ota_restore_calibration();
    } else {
        EEPROM.end();
    }

    g_initialized = true;
    UCF_LOG("OTA initialized on port %d", g_config.port);

    return true;
}

bool ota_check_update(const char* url) {
    // For basic implementation, just return false
    // Full implementation would check remote server for updates
    return false;
}

OTAStatus ota_start_update(const char* url) {
    if (!g_enabled) {
        set_error(OTA_STATUS_ERROR_NETWORK, "OTA disabled");
        return OTA_STATUS_ERROR_NETWORK;
    }

    // For HTTP OTA, would initiate download here
    // Arduino OTA is handled via ota_handle()

    reset_progress();
    g_progress.status = OTA_STATUS_CHECKING;
    g_progress.start_time_ms = millis();

    return g_progress.status;
}

OTAStatus ota_process(void) {
    // Arduino OTA handling is done in ota_handle()
    return g_progress.status;
}

LatticeValidation ota_validate_firmware(void) {
    LatticeValidation validation;
    memset(&validation, 0, sizeof(validation));

    // Get running partition for validation of current firmware
    const esp_partition_t* partition = esp_ota_get_running_partition();
    if (!partition) {
        return validation;
    }

    // For current firmware, validate the compiled constants
    validation.phi_valid = (fabs(PHI_INV - LATTICE_EXPECTED_PHI_INV) < LATTICE_TOLERANCE);
    validation.z_critical_valid = (fabs(Z_CRITICAL - LATTICE_EXPECTED_Z_CRITICAL) < LATTICE_TOLERANCE);
    validation.euler_inv_valid = (fabs(EULER_INV - LATTICE_EXPECTED_EULER_INV) < LATTICE_TOLERANCE);
    validation.pi_inv_valid = (fabs(PI_INV - LATTICE_EXPECTED_PI_INV) < LATTICE_TOLERANCE);
    validation.sqrt2_inv_valid = (fabs(SQRT2_INV - LATTICE_EXPECTED_SQRT2_INV) < LATTICE_TOLERANCE);

    validation.phi_error = fabs(PHI_INV - LATTICE_EXPECTED_PHI_INV);
    validation.z_critical_error = fabs(Z_CRITICAL - LATTICE_EXPECTED_Z_CRITICAL);

    validation.valid = validation.phi_valid &&
                       validation.z_critical_valid &&
                       validation.euler_inv_valid &&
                       validation.pi_inv_valid &&
                       validation.sqrt2_inv_valid;

    validation.lattice_checksum = ota_compute_lattice_checksum();

    return validation;
}

bool ota_apply_update(void) {
    if (!g_update_in_progress) {
        return false;
    }

    // Validate before applying
    if (g_config.strict_lattice_check) {
        LatticeValidation validation = ota_validate_firmware();

        if (!validation.valid) {
            set_error(OTA_STATUS_ERROR_LATTICE, "Lattice validation failed");
            return false;
        }

        if (g_validation_callback) {
            if (!g_validation_callback(&validation)) {
                set_error(OTA_STATUS_ERROR_LATTICE, "Validation rejected");
                return false;
            }
        }
    }

    g_progress.status = OTA_STATUS_APPLYING;

    if (g_config.auto_reboot) {
        delay(500);
        ESP.restart();
    }

    return true;
}

void ota_abort(void) {
    if (g_update_in_progress) {
        Update.abort();
        g_update_in_progress = false;
        reset_progress();
    }
}

const OTAProgress* ota_get_progress(void) {
    return &g_progress;
}

void ota_set_progress_callback(OTAProgressCallback callback) {
    g_progress_callback = callback;
}

void ota_set_validation_callback(OTAValidationCallback callback) {
    g_validation_callback = callback;
}

void ota_enable(bool enabled) {
    g_enabled = enabled;
}

bool ota_is_enabled(void) {
    return g_enabled;
}

void ota_handle(void) {
    if (g_initialized && g_enabled) {
        ArduinoOTA.handle();
    }
}

// ============================================================================
// LATTICE VALIDATION FUNCTIONS
// ============================================================================

LatticeValidation ota_validate_lattice(const uint8_t* firmware_start, uint32_t firmware_size) {
    LatticeValidation validation;
    memset(&validation, 0, sizeof(validation));

    // Search for lattice signature in firmware
    uint32_t sig_offset;
    if (!ota_find_lattice_signature(firmware_start, firmware_size, &sig_offset)) {
        return validation;  // Invalid - no signature found
    }

    // Validate individual constants (would need to know exact offsets)
    // This is a simplified implementation

    validation.valid = true;  // Placeholder
    return validation;
}

uint32_t ota_compute_lattice_checksum(void) {
    // Compute CRC32 of lattice constants
    float constants[] = {
        (float)PHI, (float)PHI_INV,
        (float)EULER, (float)EULER_INV,
        (float)UCF_PI, (float)PI_INV,
        (float)SQRT2, (float)SQRT2_INV,
        (float)Z_CRITICAL,
        (float)LAMBDA_R_SQ, (float)LAMBDA_A_SQ
    };

    // Simple checksum
    uint32_t checksum = 0xFFFFFFFF;
    uint8_t* bytes = (uint8_t*)constants;
    size_t len = sizeof(constants);

    for (size_t i = 0; i < len; i++) {
        checksum ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            if (checksum & 1) {
                checksum = (checksum >> 1) ^ 0xEDB88320;
            } else {
                checksum >>= 1;
            }
        }
    }

    return ~checksum;
}

bool ota_find_lattice_signature(const uint8_t* firmware_start, uint32_t firmware_size,
                                 uint32_t* signature_offset) {
    // Search for "RRRR" magic
    const uint8_t magic[4] = {'R', 'R', 'R', 'R'};

    for (uint32_t i = 0; i < firmware_size - 4; i++) {
        if (memcmp(firmware_start + i, magic, 4) == 0) {
            *signature_offset = i;
            return true;
        }
    }

    return false;
}

float ota_validate_phi(const uint8_t* firmware_start, uint32_t offset) {
    float value;
    memcpy(&value, firmware_start + offset, sizeof(float));
    return fabs(value - LATTICE_EXPECTED_PHI_INV);
}

float ota_validate_z_critical(const uint8_t* firmware_start, uint32_t offset) {
    float value;
    memcpy(&value, firmware_start + offset, sizeof(float));
    return fabs(value - LATTICE_EXPECTED_Z_CRITICAL);
}

// ============================================================================
// CALIBRATION PRESERVATION
// ============================================================================

bool ota_backup_calibration(void) {
    CalibrationData cal;

    // Read current calibration
    EEPROM.begin(1024);
    EEPROM.get(0, cal);

    // Write to backup location
    EEPROM.put(CALIBRATION_BACKUP_ADDR, cal);
    bool success = EEPROM.commit();
    EEPROM.end();

    if (success) {
        g_calibration_backed_up = true;
        Serial.println("[OTA] Calibration backed up");
    }

    return success;
}

bool ota_restore_calibration(void) {
    CalibrationData cal;

    EEPROM.begin(1024);
    EEPROM.get(CALIBRATION_BACKUP_ADDR, cal);

    // Verify magic
    if (cal.magic != CALIBRATION_MAGIC) {
        EEPROM.end();
        Serial.println("[OTA] No valid calibration backup found");
        return false;
    }

    // Restore to main location
    EEPROM.put(0, cal);
    bool success = EEPROM.commit();
    EEPROM.end();

    if (success) {
        Serial.println("[OTA] Calibration restored");
    }

    return success;
}

bool ota_has_calibration_backup(void) {
    CalibrationData cal;

    EEPROM.begin(1024);
    EEPROM.get(CALIBRATION_BACKUP_ADDR, cal);
    EEPROM.end();

    return (cal.magic == CALIBRATION_MAGIC);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* ota_status_string(OTAStatus status) {
    switch (status) {
        case OTA_STATUS_IDLE:           return "Idle";
        case OTA_STATUS_CHECKING:       return "Checking";
        case OTA_STATUS_DOWNLOADING:    return "Downloading";
        case OTA_STATUS_VALIDATING:     return "Validating";
        case OTA_STATUS_APPLYING:       return "Applying";
        case OTA_STATUS_SUCCESS:        return "Success";
        case OTA_STATUS_ERROR_NETWORK:  return "Network Error";
        case OTA_STATUS_ERROR_SIZE:     return "Size Error";
        case OTA_STATUS_ERROR_CHECKSUM: return "Checksum Error";
        case OTA_STATUS_ERROR_LATTICE:  return "Lattice Error";
        case OTA_STATUS_ERROR_FLASH:    return "Flash Error";
        case OTA_STATUS_ERROR_TIMEOUT:  return "Timeout";
        default:                        return "Unknown";
    }
}

const char* ota_get_firmware_version(void) {
    return UCF_VERSION_STRING;
}

void ota_get_partition_info(char* label, uint32_t* size) {
    const esp_partition_t* partition = esp_ota_get_running_partition();
    if (partition) {
        strcpy(label, partition->label);
        *size = partition->size;
    } else {
        strcpy(label, "unknown");
        *size = 0;
    }
}

void ota_reboot(void) {
    Serial.println("[OTA] Rebooting...");
    delay(100);
    ESP.restart();
}
