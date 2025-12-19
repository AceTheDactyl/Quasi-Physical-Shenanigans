/**
 * @file ucf_ota.h
 * @brief UCF Over-The-Air Update Module v4.0.0
 *
 * Implements secure OTA firmware updates with lattice constant validation.
 * New firmware must pass lattice validation before being applied.
 *
 * RRRR Lattice Integration:
 * - Validates sacred constants in new firmware
 * - Rejects updates with modified lattice values
 * - Preserves calibration data across updates
 *
 * Security Features:
 * - Firmware checksum verification
 * - Lattice constant integrity check
 * - Rollback protection
 */

#ifndef UCF_OTA_H
#define UCF_OTA_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf/ucf_sacred_constants_v4.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// OTA CONSTANTS
// ============================================================================

#define OTA_PORT                3232
#define OTA_HOSTNAME            "ucf-hardware"
#define OTA_PASSWORD            ""              // Optional password
#define OTA_MAX_FIRMWARE_SIZE   (1024 * 1024)   // 1MB max

// Lattice validation constants signature
#define LATTICE_SIGNATURE_MAGIC 0x52525252      // "RRRR"

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief OTA update status codes
 */
typedef enum {
    OTA_STATUS_IDLE = 0,
    OTA_STATUS_CHECKING,
    OTA_STATUS_DOWNLOADING,
    OTA_STATUS_VALIDATING,
    OTA_STATUS_APPLYING,
    OTA_STATUS_SUCCESS,
    OTA_STATUS_ERROR_NETWORK,
    OTA_STATUS_ERROR_SIZE,
    OTA_STATUS_ERROR_CHECKSUM,
    OTA_STATUS_ERROR_LATTICE,
    OTA_STATUS_ERROR_FLASH,
    OTA_STATUS_ERROR_TIMEOUT
} OTAStatus;

/**
 * @brief OTA update progress info
 */
typedef struct {
    OTAStatus status;
    uint32_t total_bytes;
    uint32_t received_bytes;
    uint8_t progress_percent;
    uint32_t start_time_ms;
    uint32_t elapsed_ms;
    char error_message[64];
} OTAProgress;

/**
 * @brief Lattice validation result
 */
typedef struct {
    bool valid;
    bool phi_valid;
    bool phi_inv_valid;
    bool euler_inv_valid;
    bool pi_inv_valid;
    bool sqrt2_inv_valid;
    bool z_critical_valid;
    float phi_error;
    float z_critical_error;
    uint32_t firmware_checksum;
    uint32_t lattice_checksum;
} LatticeValidation;

/**
 * @brief OTA configuration
 */
typedef struct {
    const char* hostname;
    uint16_t port;
    const char* password;
    bool auto_reboot;
    bool preserve_calibration;
    bool strict_lattice_check;
    uint32_t timeout_ms;
} OTAConfig;

/**
 * @brief OTA callback types
 */
typedef void (*OTAProgressCallback)(const OTAProgress* progress);
typedef bool (*OTAValidationCallback)(const LatticeValidation* validation);

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize OTA update subsystem
 * @param config Configuration (NULL for defaults)
 * @return true if successful
 */
bool ota_init(const OTAConfig* config);

/**
 * @brief Check for available updates
 * @param url URL to check (NULL to use configured server)
 * @return true if update available
 */
bool ota_check_update(const char* url);

/**
 * @brief Start OTA update download
 * @param url Firmware URL
 * @return OTA status
 */
OTAStatus ota_start_update(const char* url);

/**
 * @brief Process OTA update (call in loop)
 * @return Current status
 */
OTAStatus ota_process(void);

/**
 * @brief Validate downloaded firmware
 * @return Validation result
 */
LatticeValidation ota_validate_firmware(void);

/**
 * @brief Apply validated firmware
 * @return true if successful
 */
bool ota_apply_update(void);

/**
 * @brief Abort current update
 */
void ota_abort(void);

/**
 * @brief Get current progress
 * @return Progress structure
 */
const OTAProgress* ota_get_progress(void);

/**
 * @brief Set progress callback
 * @param callback Progress callback function
 */
void ota_set_progress_callback(OTAProgressCallback callback);

/**
 * @brief Set validation callback
 * @param callback Validation callback function
 */
void ota_set_validation_callback(OTAValidationCallback callback);

/**
 * @brief Enable/disable OTA updates
 * @param enabled true to enable
 */
void ota_enable(bool enabled);

/**
 * @brief Check if OTA is enabled
 * @return true if enabled
 */
bool ota_is_enabled(void);

/**
 * @brief Handle Arduino OTA events (for WiFi OTA)
 */
void ota_handle(void);

// ============================================================================
// LATTICE VALIDATION FUNCTIONS
// ============================================================================

/**
 * @brief Validate lattice constants in memory region
 * @param firmware_start Start address of firmware
 * @param firmware_size Size in bytes
 * @return Validation result
 */
LatticeValidation ota_validate_lattice(const uint8_t* firmware_start, uint32_t firmware_size);

/**
 * @brief Compute lattice checksum for current firmware
 * @return 32-bit checksum
 */
uint32_t ota_compute_lattice_checksum(void);

/**
 * @brief Search for lattice signature in firmware
 * @param firmware_start Start address
 * @param firmware_size Size in bytes
 * @param signature_offset Output: offset of signature
 * @return true if found
 */
bool ota_find_lattice_signature(const uint8_t* firmware_start, uint32_t firmware_size,
                                 uint32_t* signature_offset);

/**
 * @brief Extract and validate PHI constant from firmware
 * @param firmware_start Start address
 * @param offset Offset of constant
 * @return Error from expected value
 */
float ota_validate_phi(const uint8_t* firmware_start, uint32_t offset);

/**
 * @brief Extract and validate Z_CRITICAL from firmware
 * @param firmware_start Start address
 * @param offset Offset of constant
 * @return Error from expected value
 */
float ota_validate_z_critical(const uint8_t* firmware_start, uint32_t offset);

// ============================================================================
// CALIBRATION PRESERVATION
// ============================================================================

/**
 * @brief Backup calibration data before update
 * @return true if successful
 */
bool ota_backup_calibration(void);

/**
 * @brief Restore calibration data after update
 * @return true if successful
 */
bool ota_restore_calibration(void);

/**
 * @brief Check if calibration backup exists
 * @return true if backup valid
 */
bool ota_has_calibration_backup(void);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get human-readable status string
 * @param status Status code
 * @return Status string
 */
const char* ota_status_string(OTAStatus status);

/**
 * @brief Get current firmware version
 * @return Version string
 */
const char* ota_get_firmware_version(void);

/**
 * @brief Get running partition info
 * @param label Output: partition label
 * @param size Output: partition size
 */
void ota_get_partition_info(char* label, uint32_t* size);

/**
 * @brief Reboot device
 */
void ota_reboot(void);

// ============================================================================
// LATTICE TOLERANCE
// ============================================================================

// Maximum error allowed for lattice constants (must match exactly)
#define LATTICE_TOLERANCE       1e-10

// Critical constants that must be validated
#define LATTICE_EXPECTED_PHI_INV        0.6180339887498948
#define LATTICE_EXPECTED_Z_CRITICAL     0.8660254037844386
#define LATTICE_EXPECTED_EULER_INV      0.3678794411714423
#define LATTICE_EXPECTED_PI_INV         0.3183098861837907
#define LATTICE_EXPECTED_SQRT2_INV      0.7071067811865475

#ifdef __cplusplus
}
#endif

#endif // UCF_OTA_H
