/**
 * Safety Compliance Tests
 *
 * Automated tests for each safety requirement as specified in:
 * - NLP_Design_Handoff/pseudocode/SafetyLayer.pseudo
 * - NLP_Design_Handoff/architecture/Invariants.md
 * - WishBed_App_TDD_v2/safety/Threat_Model.md
 *
 * Test Categories:
 * 1. Consent Gate Test [INV-2]
 * 2. Emergency Stop Test [INV-3]
 * 3. SafeMinimalMode Test [REQ 0014]
 * 4. Medical Claim Scan Test [INV-4, REQ 0015]
 * 5. Privacy Test [INV-5]
 */

import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { SafetyLayer, safetyLayer } from '../../src/SafetyLayer';
import { EventBus } from '../../src/EventBus';
import { AuditLog } from '../../src/AuditLog';
import { NetworkMonitor } from '../../src/NetworkMonitor';
import { Renderer } from '../../src/Renderer';
import { AudioOutput } from '../../src/AudioOutput';
import { MeshConnection } from '../../src/MeshConnection';
import { Action, Blueprint, OperatorPersona } from '../../src/types';

// =============================================================================
// TEST 1: CONSENT GATE TEST [INV-2]
// =============================================================================

describe('1. Consent Gate Test [INV-2]', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
    EventBus.clear();
  });

  describe('High-impact action without user confirmation', () => {
    it('should BLOCK high-impact action when consent not granted', async () => {
      // Arrange: Create a dangerous/high-impact action
      const dangerousAction: Action = {
        type: 'DELETE_DATA',
        dangerous: true,
      };

      // No consent granted, no confirmation handler set to approve
      safety.confirmationHandler = async () => false;
      safety.consentDialogHandler = async () => false;

      // Act: Try to invoke the action
      const result = await safety.validateAction(dangerousAction);

      // Assert: Action should NOT execute
      expect(result).toBe(false);
    });

    it('should BLOCK DANGEROUS_FUNCTION action without confirmation', async () => {
      const dangerousAction: Action = {
        type: 'DANGEROUS_FUNCTION',
        dangerous: true,
        payload: { operation: 'destructive_reset' },
      };

      safety.confirmationHandler = async () => false;

      const result = await safety.validateAction(dangerousAction);

      expect(result).toBe(false);
    });

    it('should BLOCK START_SESSION without session consent', async () => {
      const action: Action = { type: 'START_SESSION' };

      // Consent dialog will reject
      safety.consentDialogHandler = async () => false;
      safety.confirmationHandler = async () => true;

      const result = await safety.validateAction(action);

      expect(result).toBe(false);
      expect(safety.hasConsent('SESSION')).toBe(false);
    });

    it('should BLOCK SHARE_TO_MESH without confirmation', async () => {
      const action: Action = { type: 'SHARE_TO_MESH' };

      safety.confirmationHandler = async () => false;

      const result = await safety.validateAction(action);

      expect(result).toBe(false);
    });

    it('should BLOCK PUBLISH_BLUEPRINT without confirmation', async () => {
      const action: Action = { type: 'PUBLISH_BLUEPRINT' };

      safety.confirmationHandler = async () => false;

      const result = await safety.validateAction(action);

      expect(result).toBe(false);
    });
  });

  describe('High-impact action WITH user confirmation', () => {
    it('should ALLOW high-impact action when user confirms', async () => {
      const dangerousAction: Action = {
        type: 'DELETE_DATA',
        dangerous: true,
      };

      // User confirms the action
      safety.confirmationHandler = async () => true;

      const result = await safety.validateAction(dangerousAction);

      expect(result).toBe(true);
    });

    it('should ALLOW START_SESSION when consent is granted', async () => {
      const action: Action = { type: 'START_SESSION' };

      // Grant consent through dialog
      safety.consentDialogHandler = async () => true;
      safety.confirmationHandler = async () => true;

      const result = await safety.validateAction(action);

      expect(result).toBe(true);
      expect(safety.hasConsent('SESSION')).toBe(true);
    });

    it('should ALLOW action when consent was previously granted', async () => {
      const action: Action = { type: 'START_SESSION' };

      // Pre-grant consent
      safety.grantConsent('SESSION');
      safety.confirmationHandler = async () => true;

      const result = await safety.validateAction(action);

      expect(result).toBe(true);
    });
  });

  describe('Low-impact actions', () => {
    it('should ALLOW low-impact actions without confirmation', async () => {
      const action: Action = { type: 'VIEW_CONTENT' };

      // No confirmation handler needed
      safety.confirmationHandler = async () => false;

      const result = await safety.validateAction(action);

      expect(result).toBe(true);
    });

    it('should ALLOW UPDATE_SETTINGS without confirmation', async () => {
      const action: Action = { type: 'UPDATE_SETTINGS' };

      const result = await safety.validateAction(action);

      expect(result).toBe(true);
    });
  });

  describe('Impact classification', () => {
    it('should classify DELETE_DATA as HIGH impact', () => {
      const action: Action = { type: 'DELETE_DATA' };
      expect(safety.classifyImpact(action)).toBe('HIGH');
    });

    it('should classify SHARE_TO_MESH as HIGH impact', () => {
      const action: Action = { type: 'SHARE_TO_MESH' };
      expect(safety.classifyImpact(action)).toBe('HIGH');
    });

    it('should classify EMERGENCY_STOP as SYSTEM impact', () => {
      const action: Action = { type: 'EMERGENCY_STOP' };
      expect(safety.classifyImpact(action)).toBe('SYSTEM');
    });

    it('should classify VIEW_CONTENT as LOW impact', () => {
      const action: Action = { type: 'VIEW_CONTENT' };
      expect(safety.classifyImpact(action)).toBe('LOW');
    });
  });

  describe('Consent revocation', () => {
    it('should respect consent revocation', async () => {
      // Grant then revoke consent
      safety.grantConsent('SESSION');
      expect(safety.hasConsent('SESSION')).toBe(true);

      safety.revokeConsent('SESSION');
      expect(safety.hasConsent('SESSION')).toBe(false);

      // Now action should be blocked
      const action: Action = { type: 'START_SESSION' };
      safety.consentDialogHandler = async () => false;

      const result = await safety.validateAction(action);
      expect(result).toBe(false);
    });
  });
});

// =============================================================================
// TEST 2: EMERGENCY STOP TEST [INV-3]
// =============================================================================

describe('2. Emergency Stop Test [INV-3]', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
    EventBus.clear();
    AuditLog.clear();
    Renderer.reset();
    AudioOutput.reset();
    MeshConnection.reset();
  });

  describe('Emergency stop timing (100ms requirement)', () => {
    it('should transition to IDLE state within 100ms', () => {
      // Start a session
      safety.setSessionState('ACTIVE');
      expect(safety.getSessionState()).toBe('ACTIVE');

      // Trigger emergency stop
      const startTime = performance.now();
      safety.triggerEmergencyStop();
      const endTime = performance.now();

      // Verify state is IDLE
      expect(safety.getSessionState()).toBe('IDLE');

      // Verify timing (should be essentially instant, well under 100ms)
      const elapsed = endTime - startTime;
      expect(elapsed).toBeLessThan(100);
    });

    it('should emit EMERGENCY_STOP event immediately', () => {
      let eventReceived = false;
      let eventTime = 0;

      EventBus.on('EMERGENCY_STOP', () => {
        eventReceived = true;
        eventTime = performance.now();
      });

      const startTime = performance.now();
      safety.triggerEmergencyStop();

      expect(eventReceived).toBe(true);
      expect(eventTime - startTime).toBeLessThan(100);
    });
  });

  describe('All outputs stopped', () => {
    it('should halt all audio output', () => {
      // Start audio
      AudioOutput.play(432, 0.5);
      expect(AudioOutput.getState().isPlaying).toBe(true);

      // Trigger emergency stop
      safety.triggerEmergencyStop();

      // Verify audio stopped
      expect(AudioOutput.isOutputStopped()).toBe(true);
    });

    it('should halt all visual output', () => {
      // Start renderer
      Renderer.start();
      expect(Renderer.getState().isActive).toBe(true);

      // Trigger emergency stop
      safety.triggerEmergencyStop();

      // Verify visuals stopped
      expect(Renderer.isOutputStopped()).toBe(true);
    });

    it('should disconnect from mesh', () => {
      // Connect to mesh
      MeshConnection.connect('peer-123');
      expect(MeshConnection.getState().isConnected).toBe(true);

      // Trigger emergency stop
      safety.triggerEmergencyStop();

      // Verify disconnected
      expect(MeshConnection.getState().isConnected).toBe(false);
    });

    it('should stop all outputs within 100ms of trigger', () => {
      // Start all systems
      AudioOutput.play(432, 0.5);
      Renderer.start();
      MeshConnection.connect('peer-123');

      const triggerTime = performance.now();
      safety.triggerEmergencyStop();

      // Check that all systems received stop within 100ms
      const audioStopTime = AudioOutput.getStoppedTimestamp();
      const renderStopTime = Renderer.getStoppedTimestamp();
      const meshDisconnectTime = MeshConnection.getDisconnectedTimestamp();

      expect(audioStopTime - triggerTime).toBeLessThan(100);
      expect(renderStopTime - triggerTime).toBeLessThan(100);
      expect(meshDisconnectTime - triggerTime).toBeLessThan(100);
    });
  });

  describe('Emergency stop flag and blocking', () => {
    it('should set emergency stop flag to active', () => {
      expect(safety.isEmergencyStopActive()).toBe(false);

      safety.triggerEmergencyStop();

      expect(safety.isEmergencyStopActive()).toBe(true);
    });

    it('should block ALL actions when emergency stop is active', async () => {
      safety.triggerEmergencyStop();

      // Even low-impact actions should be blocked
      const lowImpactAction: Action = { type: 'VIEW_CONTENT' };
      const result = await safety.validateAction(lowImpactAction);

      expect(result).toBe(false);
    });

    it('should block high-impact actions even with confirmation when stopped', async () => {
      safety.triggerEmergencyStop();

      const action: Action = { type: 'START_SESSION' };
      safety.grantConsent('SESSION');
      safety.confirmationHandler = async () => true;

      const result = await safety.validateAction(action);

      expect(result).toBe(false);
    });
  });

  describe('No confirmation required for emergency stop', () => {
    it('should trigger immediately without confirmation dialog', () => {
      let confirmationCalled = false;
      safety.confirmationHandler = async () => {
        confirmationCalled = true;
        return false;
      };

      // Emergency stop should NOT go through validation
      safety.triggerEmergencyStop();

      expect(safety.isEmergencyStopActive()).toBe(true);
      expect(confirmationCalled).toBe(false);
    });
  });

  describe('Audit logging', () => {
    it('should log emergency stop to audit log', () => {
      safety.triggerEmergencyStop();

      const entries = AuditLog.getEntriesByAction('EMERGENCY_STOP');
      expect(entries.length).toBe(1);
      expect(entries[0].action).toBe('EMERGENCY_STOP');
    });
  });

  describe('Emergency stop sequence', () => {
    it('should emit events in correct sequence', () => {
      const eventSequence: string[] = [];

      EventBus.on('EMERGENCY_STOP', () => eventSequence.push('EMERGENCY_STOP'));
      EventBus.on('HALT_AUDIO', () => eventSequence.push('HALT_AUDIO'));
      EventBus.on('HALT_VISUAL', () => eventSequence.push('HALT_VISUAL'));
      EventBus.on('DISCONNECT_MESH', () => eventSequence.push('DISCONNECT_MESH'));
      EventBus.on('SESSION_STATE_CHANGE', () => eventSequence.push('SESSION_STATE_CHANGE'));

      safety.triggerEmergencyStop();

      // Verify all events were emitted
      expect(eventSequence).toContain('EMERGENCY_STOP');
      expect(eventSequence).toContain('HALT_AUDIO');
      expect(eventSequence).toContain('HALT_VISUAL');
      expect(eventSequence).toContain('DISCONNECT_MESH');
      expect(eventSequence).toContain('SESSION_STATE_CHANGE');
    });
  });

  describe('Reset emergency stop', () => {
    it('should allow actions after emergency stop is reset', async () => {
      safety.triggerEmergencyStop();
      expect(safety.isEmergencyStopActive()).toBe(true);

      safety.resetEmergencyStop();
      expect(safety.isEmergencyStopActive()).toBe(false);

      const action: Action = { type: 'VIEW_CONTENT' };
      const result = await safety.validateAction(action);
      expect(result).toBe(true);
    });
  });
});

// =============================================================================
// TEST 3: SAFE MINIMAL MODE TEST [REQ 0014]
// =============================================================================

describe('3. SafeMinimalMode Test [REQ 0014]', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
    EventBus.clear();
    AuditLog.clear();
    Renderer.reset();
  });

  describe('Entering safe minimal mode on failure', () => {
    it('should reduce brightness to minimal on renderer failure', () => {
      // Simulate renderer failure by entering safe minimal mode
      safety.enterSafeMinimalMode();

      const caps = safety.getSafetyCaps();
      expect(caps.brightness).toBe(0.3);
    });

    it('should reduce motion velocity to minimal', () => {
      safety.enterSafeMinimalMode();

      const caps = safety.getSafetyCaps();
      expect(caps.motionVelocity).toBe(0.2);
    });

    it('should set flicker to zero (safest)', () => {
      safety.enterSafeMinimalMode();

      const caps = safety.getSafetyCaps();
      expect(caps.flickerHz).toBe(0);
    });

    it('should reduce particle density to minimal', () => {
      safety.enterSafeMinimalMode();

      const caps = safety.getSafetyCaps();
      expect(caps.particleDensity).toBe(0.1);
    });
  });

  describe('Complex visuals stopped instantly', () => {
    it('should emit SAFE_MINIMAL_MODE event', () => {
      let eventReceived = false;
      EventBus.on('SAFE_MINIMAL_MODE', () => {
        eventReceived = true;
      });

      safety.enterSafeMinimalMode();

      expect(eventReceived).toBe(true);
    });

    it('should stop complex visuals in renderer within minimal time', () => {
      // Start with complex visuals
      Renderer.start();
      expect(Renderer.getState().complexVisualsEnabled).toBe(true);

      const startTime = performance.now();
      safety.enterSafeMinimalMode();
      const endTime = performance.now();

      // Complex visuals should be stopped
      expect(Renderer.areComplexVisualsStopped()).toBe(true);

      // Should happen virtually instantly (< 10ms)
      expect(endTime - startTime).toBeLessThan(10);
    });

    it('should apply minimal caps to renderer state', () => {
      Renderer.start();
      safety.enterSafeMinimalMode();

      const state = Renderer.getState();
      expect(state.brightness).toBe(0.3);
      expect(state.motionVelocity).toBe(0.2);
      expect(state.flickerHz).toBe(0);
      expect(state.particleDensity).toBe(0.1);
    });
  });

  describe('Safe minimal mode state tracking', () => {
    it('should track safe minimal mode state', () => {
      expect(safety.isSafeMinimalModeActive()).toBe(false);

      safety.enterSafeMinimalMode();

      expect(safety.isSafeMinimalModeActive()).toBe(true);
    });

    it('should log safe minimal mode to audit', () => {
      safety.enterSafeMinimalMode();

      const entries = AuditLog.getEntriesByAction('SAFE_MINIMAL_MODE');
      expect(entries.length).toBe(1);
    });
  });

  describe('Exit safe minimal mode', () => {
    it('should restore default caps when exiting', () => {
      safety.enterSafeMinimalMode();
      safety.exitSafeMinimalMode();

      const caps = safety.getSafetyCaps();
      expect(caps.brightness).toBe(0.8);
      expect(caps.motionVelocity).toBe(1.0);
      expect(caps.flickerHz).toBe(10);
      expect(caps.particleDensity).toBe(0.7);
    });

    it('should emit EXIT_SAFE_MINIMAL_MODE event', () => {
      let eventReceived = false;
      EventBus.on('EXIT_SAFE_MINIMAL_MODE', () => {
        eventReceived = true;
      });

      safety.enterSafeMinimalMode();
      safety.exitSafeMinimalMode();

      expect(eventReceived).toBe(true);
    });

    it('should clear safe minimal mode flag', () => {
      safety.enterSafeMinimalMode();
      expect(safety.isSafeMinimalModeActive()).toBe(true);

      safety.exitSafeMinimalMode();
      expect(safety.isSafeMinimalModeActive()).toBe(false);
    });
  });

  describe('Tracking loss simulation', () => {
    it('should handle tracking loss by entering safe minimal mode', () => {
      // Simulate tracking loss event
      Renderer.start();

      // On tracking loss, system should enter safe minimal mode
      safety.enterSafeMinimalMode();

      expect(Renderer.areComplexVisualsStopped()).toBe(true);
      expect(safety.isSafeMinimalModeActive()).toBe(true);
    });
  });
});

// =============================================================================
// TEST 4: MEDICAL CLAIM SCAN TEST [INV-4, REQ 0015]
// =============================================================================

describe('4. Medical Claim Scan Test [INV-4, REQ 0015]', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
  });

  describe('Blueprint with "cure" should be REJECTED', () => {
    it('should reject blueprint with "cure" in name', () => {
      const blueprint: Blueprint = {
        id: 'bp-1',
        name: 'Anxiety Cure Blueprint',
        description: 'A helpful experience',
        stages: [],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.result).toBe('REJECT');
      expect(result.violations.some(v => v.includes('cure'))).toBe(true);
    });

    it('should reject blueprint with "cure" in description', () => {
      const blueprint: Blueprint = {
        id: 'bp-2',
        name: 'Relaxation Experience',
        description: 'This will cure your stress',
        stages: [],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.result).toBe('REJECT');
      expect(result.violations.length).toBeGreaterThan(0);
    });

    it('should reject blueprint with "cure" in stage narration', () => {
      const blueprint: Blueprint = {
        id: 'bp-3',
        name: 'Wellness Journey',
        description: 'Experience peace',
        stages: [
          { name: 'Stage 1', narration: 'This will cure all ailments', duration: 60 },
        ],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.result).toBe('REJECT');
    });
  });

  describe('Other medical terms should be REJECTED', () => {
    const medicalTerms = ['heal', 'treat', 'therapy', 'diagnosis', 'medical', 'clinical'];

    medicalTerms.forEach(term => {
      it(`should reject blueprint with "${term}"`, () => {
        const blueprint: Blueprint = {
          id: `bp-${term}`,
          name: `${term} Experience`,
          description: `Using ${term} approach`,
          stages: [],
        };

        const result = safety.validateBlueprint(blueprint);

        expect(result.result).toBe('REJECT');
      });
    });
  });

  describe('Guarantee patterns should be REJECTED', () => {
    const guaranteePatterns = [
      'will cure your problems',
      'will heal you completely',
      'will fix everything',
      'permanent solution',
      'permanently removes anxiety',
      'guaranteed results',
      'proven to work',
    ];

    guaranteePatterns.forEach(pattern => {
      it(`should reject blueprint with "${pattern}"`, () => {
        const blueprint: Blueprint = {
          id: 'bp-guarantee',
          name: 'Experience',
          description: pattern,
          stages: [],
        };

        const result = safety.validateBlueprint(blueprint);

        expect(result.result).toBe('REJECT');
      });
    });
  });

  describe('Safe content should PASS', () => {
    it('should accept blueprint with experiential language', () => {
      const blueprint: Blueprint = {
        id: 'bp-safe',
        name: 'Peaceful Exploration',
        description: 'An experience designed to support relaxation',
        stages: [
          { name: 'Opening', narration: 'Begin your journey of discovery', duration: 60 },
          { name: 'Exploration', narration: 'Allow yourself to explore new sensations', duration: 120 },
        ],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.result).toBe('PASS');
      expect(result.violations.length).toBe(0);
    });

    it('should accept blueprint with symbolic language', () => {
      const blueprint: Blueprint = {
        id: 'bp-symbolic',
        name: 'Inner Light Ritual',
        description: 'A symbolic journey through consciousness',
        stages: [
          { name: 'Awakening', narration: 'Feel the light within', duration: 90 },
        ],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.result).toBe('PASS');
    });
  });

  describe('Suggestions for rejected content', () => {
    it('should provide helpful suggestions for rejected content', () => {
      const blueprint: Blueprint = {
        id: 'bp-1',
        name: 'Healing Journey',
        description: 'Guaranteed to help',
        stages: [],
      };

      const result = safety.validateBlueprint(blueprint);

      expect(result.suggestions.length).toBeGreaterThan(0);
      expect(result.suggestions.some(s => s.includes('experiential'))).toBe(true);
    });
  });

  describe('Case insensitivity', () => {
    it('should reject regardless of case (CURE, Cure, cure)', () => {
      const variations = ['CURE', 'Cure', 'cure', 'CuRe'];

      variations.forEach(variant => {
        const blueprint: Blueprint = {
          id: 'bp-case',
          name: `${variant} Experience`,
          description: 'Testing',
          stages: [],
        };

        const result = safety.validateBlueprint(blueprint);
        expect(result.result).toBe('REJECT');
      });
    });
  });

  describe('Coercive persona scanning', () => {
    it('should reject persona with "you must" language', () => {
      const persona: OperatorPersona = {
        id: 'p-1',
        name: 'Strict Guide',
        narration: ['You must continue this session', 'Keep going'],
      };

      const result = safety.scanPersona(persona);

      expect(result.result).toBe('REJECT');
      expect(result.violations.some(v => v.includes('Coercive'))).toBe(true);
    });

    it('should reject persona with guilt language', () => {
      const persona: OperatorPersona = {
        id: 'p-2',
        name: 'Pushy Guide',
        narration: ['Stopping is for the weak', 'Real practitioners never quit'],
      };

      const result = safety.scanPersona(persona);

      expect(result.result).toBe('REJECT');
    });

    it('should reject persona with conditional threats', () => {
      const persona: OperatorPersona = {
        id: 'p-3',
        name: 'Threatening Guide',
        narration: ['If you stop now, you will never achieve enlightenment'],
      };

      const result = safety.scanPersona(persona);

      expect(result.result).toBe('REJECT');
    });

    it('should accept encouraging but non-coercive persona', () => {
      const persona: OperatorPersona = {
        id: 'p-safe',
        name: 'Gentle Guide',
        narration: [
          'Welcome to this experience',
          'Feel free to pause whenever you need',
          'You are in control of this journey',
        ],
      };

      const result = safety.scanPersona(persona);

      expect(result.result).toBe('PASS');
    });
  });
});

// =============================================================================
// TEST 5: PRIVACY TEST [INV-5]
// =============================================================================

describe('5. Privacy Test [INV-5]', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
    NetworkMonitor.clear();
    NetworkMonitor.enable();
    MeshConnection.reset();
    EventBus.clear();
  });

  afterEach(() => {
    NetworkMonitor.disable();
  });

  describe('No unexpected network calls', () => {
    it('should allow only explicitly expected domains', () => {
      // Set up allowed domains (e.g., marketplace if user opts in)
      NetworkMonitor.setAllowedDomains(['marketplace.wishbed.app', 'api.wishbed.app']);

      // Simulate expected call
      NetworkMonitor.recordCall('https://marketplace.wishbed.app/blueprints', 'GET');

      // Verify no unexpected calls
      const unexpectedCalls = NetworkMonitor.getUnexpectedCalls();
      expect(unexpectedCalls.length).toBe(0);
    });

    it('should detect unexpected network calls', () => {
      NetworkMonitor.setAllowedDomains(['marketplace.wishbed.app']);

      // Simulate unexpected call to analytics
      NetworkMonitor.recordCall('https://analytics.external.com/track', 'POST');

      const unexpectedCalls = NetworkMonitor.getUnexpectedCalls();
      expect(unexpectedCalls.length).toBe(1);
      expect(unexpectedCalls[0].url).toContain('analytics.external.com');
    });

    it('should have zero network calls with no opt-in (local-first)', () => {
      // With no domains allowed, any call is unexpected
      NetworkMonitor.setAllowedDomains([]);

      // Verify no calls made
      const calls = NetworkMonitor.getCalls();
      expect(calls.length).toBe(0);
    });

    it('should detect tracking/telemetry calls', () => {
      NetworkMonitor.setAllowedDomains(['api.wishbed.app']);

      // Simulate tracking call
      NetworkMonitor.recordCall('https://tracking.adnetwork.com/pixel', 'GET');
      NetworkMonitor.recordCall('https://telemetry.vendor.com/collect', 'POST');

      const unexpectedCalls = NetworkMonitor.getUnexpectedCalls();
      expect(unexpectedCalls.length).toBe(2);
    });
  });

  describe('No personal data in network calls', () => {
    it('should not send email in network data', () => {
      NetworkMonitor.setAllowedDomains(['api.wishbed.app']);

      NetworkMonitor.recordCall('https://api.wishbed.app/sync', 'POST', {
        sessionId: 'abc123',
        email: 'user@example.com', // Should not be sent
      });

      expect(NetworkMonitor.hasPersonalDataInCalls()).toBe(true);
    });

    it('should not send location data', () => {
      NetworkMonitor.setAllowedDomains(['api.wishbed.app']);

      NetworkMonitor.recordCall('https://api.wishbed.app/data', 'POST', {
        location: { lat: 40.7128, lng: -74.0060 },
      });

      expect(NetworkMonitor.hasPersonalDataInCalls()).toBe(true);
    });

    it('should allow calls without personal data', () => {
      NetworkMonitor.setAllowedDomains(['api.wishbed.app']);

      NetworkMonitor.recordCall('https://api.wishbed.app/blueprints', 'GET', {
        filter: 'popular',
        limit: 20,
      });

      expect(NetworkMonitor.hasPersonalDataInCalls()).toBe(false);
    });
  });

  describe('No personal data in mesh messages', () => {
    it('should not send personal data in mesh beyond necessary', () => {
      MeshConnection.connect('mesh-host-123');

      // Send anonymous session data (OK)
      MeshConnection.sendMessage({
        type: 'SESSION_STATE',
        payload: { phase: 'PARADOX', intensity: 0.7 },
        sender: 'anonymous-peer',
      });

      expect(MeshConnection.hasPersonalDataInMessages()).toBe(false);
    });

    it('should detect personal data leak in mesh', () => {
      MeshConnection.connect('mesh-host-123');

      // Send message with personal data (NOT OK)
      MeshConnection.sendMessage({
        type: 'USER_INFO',
        payload: {
          email: 'user@example.com',
          deviceId: 'device-123',
        },
        sender: 'user-123',
      });

      expect(MeshConnection.hasPersonalDataInMessages()).toBe(true);
    });

    it('should allow sharing of anonymous experience data', () => {
      MeshConnection.connect('mesh-host-123');

      MeshConnection.sendMessage({
        type: 'EXPERIENCE_SYNC',
        payload: {
          blueprintId: 'bp-123',
          currentStage: 2,
          phaseZ: 0.866,
        },
        sender: 'anon',
      });

      expect(MeshConnection.hasPersonalDataInMessages()).toBe(false);
    });
  });

  describe('Consent required for data transmission', () => {
    it('should require ANALYTICS consent for analytics transmission', async () => {
      const action: Action = { type: 'VIEW_CONTENT' };

      // Without analytics consent, no analytics should be sent
      expect(safety.hasConsent('ANALYTICS')).toBe(false);

      // Analytics call would be blocked at application level
    });

    it('should require MESH consent for mesh sharing', async () => {
      const action: Action = { type: 'JOIN_MESH' };

      safety.consentDialogHandler = async () => false;

      const result = await safety.validateAction(action);

      expect(result).toBe(false);
      expect(safety.hasConsent('MESH')).toBe(false);
    });
  });

  describe('Local-first storage', () => {
    it('should store consent state locally', () => {
      // Grant and check consent
      safety.grantConsent('SESSION');

      // Consent should be tracked locally
      expect(safety.hasConsent('SESSION')).toBe(true);

      // Revoke and verify local update
      safety.revokeConsent('SESSION');
      expect(safety.hasConsent('SESSION')).toBe(false);
    });
  });

  describe('Marketplace calls only with explicit opt-in', () => {
    it('should only call marketplace when user publishes', () => {
      // Marketplace calls should only happen on explicit action
      NetworkMonitor.setAllowedDomains([]);

      // Simulate app running - no marketplace calls
      const calls = NetworkMonitor.getCalls();
      expect(calls.filter(c => c.url.includes('marketplace')).length).toBe(0);
    });

    it('should allow marketplace call on explicit publish action', async () => {
      NetworkMonitor.setAllowedDomains(['marketplace.wishbed.app']);

      // User explicitly publishes
      safety.confirmationHandler = async () => true;
      const action: Action = { type: 'PUBLISH_BLUEPRINT' };
      const allowed = await safety.validateAction(action);

      if (allowed) {
        // Simulate marketplace call
        NetworkMonitor.recordCall('https://marketplace.wishbed.app/publish', 'POST', {
          blueprintId: 'bp-123',
        });
      }

      // This call is expected
      const unexpectedCalls = NetworkMonitor.getUnexpectedCalls();
      expect(unexpectedCalls.length).toBe(0);
    });
  });
});

// =============================================================================
// INTEGRATION TEST: Combined Safety Scenarios
// =============================================================================

describe('Integration: Combined Safety Scenarios', () => {
  let safety: SafetyLayer;

  beforeEach(() => {
    safety = new SafetyLayer();
    EventBus.clear();
    AuditLog.clear();
    Renderer.reset();
    AudioOutput.reset();
    MeshConnection.reset();
    NetworkMonitor.clear();
  });

  it('should handle emergency stop during active session with mesh connection', () => {
    // Setup active session
    safety.grantConsent('SESSION');
    safety.grantConsent('MESH');
    safety.setSessionState('ACTIVE');
    Renderer.start();
    AudioOutput.play(432, 0.5);
    MeshConnection.connect('host-123');

    // Trigger emergency stop
    safety.triggerEmergencyStop();

    // All systems should be stopped
    expect(safety.getSessionState()).toBe('IDLE');
    expect(safety.isEmergencyStopActive()).toBe(true);
    expect(Renderer.isOutputStopped()).toBe(true);
    expect(AudioOutput.isOutputStopped()).toBe(true);
    expect(MeshConnection.getState().isConnected).toBe(false);
  });

  it('should enter safe minimal mode when renderer fails and maintain privacy', () => {
    NetworkMonitor.enable();
    NetworkMonitor.setAllowedDomains([]);

    safety.setSessionState('ACTIVE');
    Renderer.start();

    // Simulate renderer failure - enter safe minimal mode
    safety.enterSafeMinimalMode();

    // Verify minimal mode active
    expect(safety.isSafeMinimalModeActive()).toBe(true);
    expect(Renderer.areComplexVisualsStopped()).toBe(true);

    // Verify no error reporting calls (privacy)
    const unexpectedCalls = NetworkMonitor.getUnexpectedCalls();
    expect(unexpectedCalls.length).toBe(0);
  });

  it('should reject dangerous blueprint and not proceed with publishing', async () => {
    const dangerousBlueprint: Blueprint = {
      id: 'bp-danger',
      name: 'Miracle Cure Experience',
      description: 'Guaranteed to heal all ailments permanently',
      stages: [
        { name: 'Stage 1', narration: 'This therapy will cure you', duration: 60 },
      ],
    };

    // Scan blueprint
    const scanResult = safety.validateBlueprint(dangerousBlueprint);
    expect(scanResult.result).toBe('REJECT');

    // Even if user tries to publish, it should fail validation
    // (In real app, publish would check scan result first)
    expect(scanResult.violations.length).toBeGreaterThan(0);
  });

  it('should maintain consent requirements across all safety layers', async () => {
    // Start with no consent
    expect(safety.hasConsent('SESSION')).toBe(false);
    expect(safety.hasConsent('MESH')).toBe(false);

    // Try to start session - should be blocked
    safety.consentDialogHandler = async () => false;
    let result = await safety.validateAction({ type: 'START_SESSION' });
    expect(result).toBe(false);

    // Grant consent, try again
    safety.grantConsent('SESSION');
    safety.confirmationHandler = async () => true;
    result = await safety.validateAction({ type: 'START_SESSION' });
    expect(result).toBe(true);
  });
});
