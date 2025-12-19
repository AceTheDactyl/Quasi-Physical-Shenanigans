/**
 * SafetyLayer Module
 *
 * This module enforces all system invariants [INV-1 through INV-5].
 * NO other module may bypass these checks.
 *
 * Requirements: [0005, 0006, 0011-0015]
 */

import {
  ConsentType,
  ConsentState,
  SafetyCaps,
  ScanReport,
  Action,
  ImpactLevel,
  Blueprint,
  OperatorPersona,
  ImportArchive,
  SessionState,
} from './types';
import { EventBus } from './EventBus';
import { AuditLog } from './AuditLog';

// Clamp utility
function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}

export class SafetyLayer {
  private consentState: ConsentState;
  private safetyCaps: SafetyCaps;
  private emergencyStopActive: boolean = false;
  private sessionState: SessionState = 'IDLE';
  private emergencyStopTimestamp: number = 0;
  private safeMinimalModeActive: boolean = false;

  // Default caps [REQ 0013]
  private static readonly DEFAULT_CAPS: SafetyCaps = {
    brightness: 0.8,
    motionVelocity: 1.0,
    flickerHz: 10,
    particleDensity: 0.7,
  };

  // Safe minimal mode caps
  private static readonly SAFE_MINIMAL_CAPS: SafetyCaps = {
    brightness: 0.3,
    motionVelocity: 0.2,
    flickerHz: 0,
    particleDensity: 0.1,
  };

  // Prohibited patterns [REQ 0015]
  private static readonly MEDICAL_PATTERNS: RegExp[] = [
    /cure/i,
    /heal/i,
    /treat/i,
    /therapy/i,
    /diagnos/i,
    /medical/i,
    /clinical/i,
  ];

  private static readonly GUARANTEE_PATTERNS: RegExp[] = [
    /will (cure|heal|fix|remove)/i,
    /permanent(ly)?/i,
    /guarante/i,
    /proven/i,
  ];

  private static readonly COERCIVE_PATTERNS: RegExp[] = [
    /you must/i,
    /you have to/i,
    /for the weak/i,
    /real .* don't quit/i,
    /if you stop.*never/i,
  ];

  // Confirmation handler - can be overridden for testing
  public confirmationHandler: (action: Action) => Promise<boolean> = async () => false;
  public consentDialogHandler: (type: ConsentType) => Promise<boolean> = async () => false;

  constructor() {
    this.consentState = this.defaultConsentState();
    this.safetyCaps = { ...SafetyLayer.DEFAULT_CAPS };
  }

  private defaultConsentState(): ConsentState {
    return {
      session: false,
      microphone: false,
      recording: false,
      mesh: false,
      analytics: false,
      timestamps: new Map(),
    };
  }

  // ---------------------------------------------------------------------------
  // CONSENT MANAGEMENT [REQ 0005, 0006, INV-2]
  // ---------------------------------------------------------------------------

  hasConsent(type: ConsentType): boolean {
    const key = type.toLowerCase() as keyof Omit<ConsentState, 'timestamps'>;
    return this.consentState[key] === true;
  }

  async requestConsent(type: ConsentType): Promise<boolean> {
    const result = await this.consentDialogHandler(type);

    if (result) {
      const key = type.toLowerCase() as keyof Omit<ConsentState, 'timestamps'>;
      (this.consentState as Record<string, unknown>)[key] = true;
      this.consentState.timestamps.set(type, new Date());
      return true;
    }
    return false;
  }

  revokeConsent(type: ConsentType): void {
    const key = type.toLowerCase() as keyof Omit<ConsentState, 'timestamps'>;
    (this.consentState as Record<string, unknown>)[key] = false;
    this.consentState.timestamps.set(type, new Date());
  }

  grantConsent(type: ConsentType): void {
    const key = type.toLowerCase() as keyof Omit<ConsentState, 'timestamps'>;
    (this.consentState as Record<string, unknown>)[key] = true;
    this.consentState.timestamps.set(type, new Date());
  }

  // ---------------------------------------------------------------------------
  // IMPACT CLASSIFICATION [REQ 0006]
  // ---------------------------------------------------------------------------

  classifyImpact(action: Action): ImpactLevel {
    switch (action.type) {
      case 'START_SESSION':
      case 'END_SESSION':
      case 'SHARE_TO_MESH':
      case 'PUBLISH_BLUEPRINT':
      case 'DELETE_DATA':
      case 'DANGEROUS_FUNCTION':
        return 'HIGH';

      case 'EMERGENCY_STOP':
        return 'SYSTEM';

      default:
        return 'LOW';
    }
  }

  requiresConfirmation(action: Action): boolean {
    return this.classifyImpact(action) === 'HIGH' || action.dangerous === true;
  }

  // ---------------------------------------------------------------------------
  // SAFETY CAPS [REQ 0013]
  // ---------------------------------------------------------------------------

  getSafetyCaps(): SafetyCaps {
    return { ...this.safetyCaps };
  }

  updateSafetyCaps(updates: Partial<SafetyCaps>): void {
    if (updates.brightness != null) {
      this.safetyCaps.brightness = clamp(updates.brightness, 0, 1);
    }
    if (updates.motionVelocity != null) {
      this.safetyCaps.motionVelocity = clamp(updates.motionVelocity, 0, 2);
    }
    if (updates.flickerHz != null) {
      this.safetyCaps.flickerHz = clamp(updates.flickerHz, 0, 30);
    }
    if (updates.particleDensity != null) {
      this.safetyCaps.particleDensity = clamp(updates.particleDensity, 0, 1);
    }
  }

  applyBrightnessCap(value: number): number {
    return Math.min(value, this.safetyCaps.brightness);
  }

  applyMotionCap(velocity: number): number {
    return Math.min(velocity, this.safetyCaps.motionVelocity);
  }

  applyFlickerCap(frequency: number): number {
    return Math.min(frequency, this.safetyCaps.flickerHz);
  }

  applyDensityCap(density: number): number {
    return Math.min(density, this.safetyCaps.particleDensity);
  }

  // ---------------------------------------------------------------------------
  // SESSION STATE
  // ---------------------------------------------------------------------------

  getSessionState(): SessionState {
    return this.sessionState;
  }

  setSessionState(state: SessionState): void {
    this.sessionState = state;
    EventBus.emit('SESSION_STATE_CHANGE', state);
  }

  // ---------------------------------------------------------------------------
  // EMERGENCY STOP [REQ INV-3]
  // ---------------------------------------------------------------------------

  triggerEmergencyStop(): void {
    const startTime = performance.now();

    // Set flag immediately
    this.emergencyStopActive = true;
    this.emergencyStopTimestamp = startTime;
    this.sessionState = 'IDLE';

    // Broadcast to all subsystems - synchronous for speed
    EventBus.emit('EMERGENCY_STOP');
    EventBus.emit('HALT_AUDIO');
    EventBus.emit('HALT_VISUAL');
    EventBus.emit('DISCONNECT_MESH');
    EventBus.emit('SESSION_STATE_CHANGE', 'IDLE');

    // Log for audit
    AuditLog.record('EMERGENCY_STOP', startTime);
  }

  isEmergencyStopActive(): boolean {
    return this.emergencyStopActive;
  }

  getEmergencyStopTimestamp(): number {
    return this.emergencyStopTimestamp;
  }

  resetEmergencyStop(): void {
    this.emergencyStopActive = false;
  }

  // ---------------------------------------------------------------------------
  // SAFE MINIMAL MODE [REQ 0014]
  // ---------------------------------------------------------------------------

  enterSafeMinimalMode(): void {
    const startTime = performance.now();

    this.safeMinimalModeActive = true;

    // Reduce all caps to minimum
    this.updateSafetyCaps(SafetyLayer.SAFE_MINIMAL_CAPS);

    // Notify renderer
    EventBus.emit('SAFE_MINIMAL_MODE');

    // Log for audit
    AuditLog.record('SAFE_MINIMAL_MODE', startTime);
  }

  isSafeMinimalModeActive(): boolean {
    return this.safeMinimalModeActive;
  }

  exitSafeMinimalMode(): void {
    this.safeMinimalModeActive = false;
    this.updateSafetyCaps(SafetyLayer.DEFAULT_CAPS);
    EventBus.emit('EXIT_SAFE_MINIMAL_MODE');
  }

  // ---------------------------------------------------------------------------
  // CONTENT SCANNING [REQ 0015, 0027-0029]
  // ---------------------------------------------------------------------------

  scanBlueprint(blueprint: Blueprint): ScanReport {
    const violations: string[] = [];
    const suggestions: string[] = [];

    // Scan name
    for (const pattern of SafetyLayer.MEDICAL_PATTERNS) {
      if (pattern.test(blueprint.name)) {
        violations.push(`Name contains medical term: ${pattern}`);
        suggestions.push('Use experiential language instead');
      }
    }

    // Scan description
    for (const pattern of SafetyLayer.GUARANTEE_PATTERNS) {
      if (pattern.test(blueprint.description)) {
        violations.push(`Description contains guarantee: ${pattern}`);
        suggestions.push("Replace with 'designed to support' or 'may help'");
      }
    }

    for (const pattern of SafetyLayer.MEDICAL_PATTERNS) {
      if (pattern.test(blueprint.description)) {
        violations.push(`Description contains medical term: ${pattern}`);
        suggestions.push('Use experiential language instead');
      }
    }

    // Scan narration in stages
    for (const stage of blueprint.stages) {
      for (const pattern of SafetyLayer.MEDICAL_PATTERNS) {
        if (pattern.test(stage.narration)) {
          violations.push(`Stage "${stage.name}" contains medical claim`);
        }
      }
      for (const pattern of SafetyLayer.GUARANTEE_PATTERNS) {
        if (pattern.test(stage.narration)) {
          violations.push(`Stage "${stage.name}" contains guarantee`);
        }
      }
    }

    if (violations.length > 0) {
      return { result: 'REJECT', violations, suggestions };
    }
    return { result: 'PASS', violations: [], suggestions: [] };
  }

  scanPersona(persona: OperatorPersona): ScanReport {
    const violations: string[] = [];
    const suggestions: string[] = [];

    for (const line of persona.narration) {
      for (const pattern of SafetyLayer.COERCIVE_PATTERNS) {
        if (pattern.test(line)) {
          violations.push(`Coercive language detected: "${line}"`);
          suggestions.push('Remove imperative commands and guilt language');
        }
      }

      for (const pattern of SafetyLayer.MEDICAL_PATTERNS) {
        if (pattern.test(line)) {
          violations.push(`Medical claim in persona: "${line}"`);
        }
      }
    }

    if (violations.length > 0) {
      return { result: 'REJECT', violations, suggestions };
    }
    return { result: 'PASS', violations: [], suggestions: [] };
  }

  scanImport(archive: ImportArchive): ScanReport {
    let allViolations: string[] = [];
    let allSuggestions: string[] = [];

    for (const blueprint of archive.blueprints) {
      const result = this.scanBlueprint(blueprint);
      allViolations = allViolations.concat(result.violations);
      allSuggestions = allSuggestions.concat(result.suggestions);
    }

    for (const persona of archive.personas) {
      const result = this.scanPersona(persona);
      allViolations = allViolations.concat(result.violations);
      allSuggestions = allSuggestions.concat(result.suggestions);
    }

    if (allViolations.length > 0) {
      return { result: 'WARN', violations: allViolations, suggestions: allSuggestions };
    }
    return { result: 'PASS', violations: [], suggestions: [] };
  }

  validateBlueprint(blueprint: Blueprint): ScanReport {
    return this.scanBlueprint(blueprint);
  }

  // ---------------------------------------------------------------------------
  // VALIDATION GATE
  // ---------------------------------------------------------------------------

  /**
   * Master validation function - call before any protected action
   * Returns true if action may proceed, false if blocked
   */
  async validateAction(action: Action): Promise<boolean> {
    // Emergency stop overrides everything
    if (this.isEmergencyStopActive()) {
      return false;
    }

    // Check consent
    const requiredConsent = this.getRequiredConsent(action);
    if (requiredConsent != null && !this.hasConsent(requiredConsent)) {
      const granted = await this.requestConsent(requiredConsent);
      if (!granted) {
        return false;
      }
    }

    // Check confirmation for high-impact or dangerous actions
    if (this.requiresConfirmation(action)) {
      const confirmed = await this.confirmationHandler(action);
      if (!confirmed) {
        return false;
      }
    }

    return true;
  }

  private getRequiredConsent(action: Action): ConsentType | null {
    switch (action.type) {
      case 'START_SESSION':
        return 'SESSION';
      case 'ENABLE_VOICE':
        return 'MICROPHONE';
      case 'START_RECORDING':
        return 'RECORDING';
      case 'JOIN_MESH':
        return 'MESH';
      default:
        return null;
    }
  }

  // ---------------------------------------------------------------------------
  // RESET (for testing)
  // ---------------------------------------------------------------------------

  reset(): void {
    this.consentState = this.defaultConsentState();
    this.safetyCaps = { ...SafetyLayer.DEFAULT_CAPS };
    this.emergencyStopActive = false;
    this.sessionState = 'IDLE';
    this.emergencyStopTimestamp = 0;
    this.safeMinimalModeActive = false;
  }
}

// Singleton instance
export const safetyLayer = new SafetyLayer();
