# Engineering Handoff Document

This document translates symbolic NLP specifications into concrete engineering tasks.

## Purpose

The WishBed Spread NLP uses intentionally abstract, symbolic language to define user experiences. This handoff document bridges that symbolic specification to implementable software engineering tasks.

## Critical Rules

1. **No module may bypass the Safety Layer** - All data flow must pass through safety validation
2. **Symbolic language must never become literal claims** - Implementation must maintain metaphorical framing
3. **Consent gates are blocking operations** - User must explicitly confirm before high-impact actions proceed

---

## NLP to Code Translation Guide

### Symbolic Term → Technical Implementation

| NLP Term | Engineering Implementation |
|----------|---------------------------|
| "Ritual" | `Session` object with defined start/end lifecycle |
| "Blueprint" | `SessionConfiguration` JSON schema with validation |
| "Operator" | Voice interface persona (tone-only, no capability changes) |
| "Overlay" | Visual rendering layer (2D/3D/AR) |
| "Interference Shield" | Session isolation with graceful fallback |
| "LocalVault" | Encrypted local storage (AES-256, device-only keys) |
| "Reality Anchor" | Non-medical disclaimer + symbolic framing enforcement |
| "SafeMinimalMode" | Fallback state with reduced visual/audio intensity |
| "Emergency Stop" | Immediate session termination, highest UI priority |

### Phase System → Hardware State

| Symbolic Phase | z-coordinate | Solfeggio Range | Hardware State |
|---------------|--------------|-----------------|----------------|
| UNTRUE | 0 ≤ z < 0.618 | 174-396 Hz | Foundation/Exploration |
| PARADOX | 0.618 ≤ z < 0.866 | 417-639 Hz | Transformation/Liminal |
| TRUE | 0.866 ≤ z ≤ 1.0 | 741-963 Hz | Integration/Realization |

---

## Module Implementation Mapping

### Module 1: App Engine [REQ 0007-0010]

**Symbolic Description**: "Session flows through preparation, activation, experience, and integration"

**Technical Implementation**:
```typescript
enum SessionState {
  IDLE = "idle",
  INITIALIZING = "initializing",
  ACTIVE = "active",
  INTEGRATING = "integrating",
  SHUTDOWN = "shutdown"
}

interface SessionTransition {
  from: SessionState;
  to: SessionState;
  guard: () => boolean;  // Must pass safety checks
  action: () => Promise<void>;
}
```

**Engineering Tasks**:
- [ ] Implement finite state machine with guarded transitions
- [ ] Add session persistence for crash recovery
- [ ] Implement initialization progress feedback (prevent double-tap)
- [ ] Add shutdown fade sequence with audio ramp-down

---

### Module 2: Safety Layer [REQ 0011-0015]

**Symbolic Description**: "All experiences pass through protective gates"

**Technical Implementation**:
```typescript
interface SafetyLayer {
  // Consent management
  checkConsent(action: HighImpactAction): Promise<boolean>;
  requestConsent(action: HighImpactAction): Promise<ConsentResult>;

  // Safety caps
  applyBrightnessCap(value: number): number;
  applyMotionCap(velocity: Vector3): Vector3;
  applyFlickerCap(frequency: number): number;

  // Emergency controls
  triggerEmergencyStop(): void;
  enterSafeMinimalMode(): void;

  // Reality anchoring
  validateBlueprint(blueprint: Blueprint): ValidationResult;
  rewriteMedicalClaims(text: string): string;
}
```

**Engineering Tasks**:
- [ ] Implement SafetyCaps with configurable thresholds
- [ ] Create ConsentManager with persistent consent state
- [ ] Add blueprint scanner for prohibited content patterns
- [ ] Implement SafeMinimalMode fallback renderer
- [ ] Ensure EmergencyStop has highest z-index and touch priority

---

### Module 3: Voice Interface [REQ 0016-0020]

**Symbolic Description**: "Voice responds to intent, not literal command"

**Technical Implementation**:
```typescript
interface VoiceInterface {
  mode: "WAKE_WORD" | "COMMAND_CAPTURE" | "INACTIVE";

  // Audio pipeline
  enableVAD(): void;           // Voice Activity Detection
  enableNoiseSuppression(): void;
  enableEchoCancellation(): void;

  // Intent processing
  parseIntent(transcript: string): VoiceIntent;
  confirmHighImpact(intent: VoiceIntent): Promise<boolean>;

  // Environment profiles
  setProfile(profile: "QuietRoom" | "TypicalHome" | "NoisyHome" | "BedroomAtNight"): void;
}

interface VoiceIntent {
  phrase: string;
  confidence: number;      // 0.0-1.0
  impact: "LOW" | "HIGH";
  requiresConfirmation: boolean;
  mappedAction: AllowedAction | null;
}
```

**Engineering Tasks**:
- [ ] Implement wake word detection with time-boxed listening window
- [ ] Add confidence threshold (recommend 0.7 minimum)
- [ ] Create confirmation dialog for high-impact intents
- [ ] Implement clarification prompt for low-confidence (<0.5) results
- [ ] Add environment profile calibration wizard

---

### Module 4: Renderer Adapter [REQ 0032-0035]

**Symbolic Description**: "Visual experience anchors to physical space"

**Technical Implementation**:
```typescript
interface RendererAdapter {
  backend: "PREVIEW_2D" | "ENGINE_3D" | "AR_BACKEND";

  // Mode management
  setOverlayMode(mode: "BED" | "WALKING"): void;

  // AR specifics
  anchorToPlane(planeId: string): void;
  trackBody(skeleton: BodySkeleton): void;

  // Safety integration
  onTrackingLoss(): void;  // Fade and pause motion-linked features
  applySafetyCaps(caps: SafetyCaps): void;
}
```

**Engineering Tasks**:
- [ ] Implement Preview2D for non-AR fallback
- [ ] Add plane detection for bed anchoring
- [ ] Implement body tracking for Walking mode
- [ ] Add tracking loss handler with graceful fade (500ms)
- [ ] Ensure all visual effects respect SafetyCaps

---

### Module 5: Storage & Privacy [REQ 0036-0038]

**Symbolic Description**: "Memory lives locally, travels encrypted"

**Technical Implementation**:
```typescript
interface LocalVault {
  // Encryption
  encrypt(data: Uint8Array, key: CryptoKey): Promise<Uint8Array>;
  decrypt(data: Uint8Array, key: CryptoKey): Promise<Uint8Array>;

  // Key management (device-bound)
  deriveKey(deviceSecret: string): Promise<CryptoKey>;

  // Session storage
  saveSession(session: SessionRecording): Promise<void>;
  loadSession(id: string): Promise<SessionRecording>;

  // Export/Import
  exportArchive(format: "JSON" | "ENCRYPTED"): Promise<Blob>;
  importArchive(archive: Blob): Promise<ImportPreview>;
  scanImport(archive: Blob): Promise<ScanResult>;
}
```

**Engineering Tasks**:
- [ ] Implement AES-256-GCM encryption with device-bound keys
- [ ] Add session recording with opt-in consent gate
- [ ] Create JSON export with narrative text option
- [ ] Implement import preview + safety scan before applying

---

### Module 6: Mesh Sessions [REQ 0021-0026] (Full Build)

**Symbolic Description**: "Shared space with protected boundaries"

**Technical Implementation**:
```typescript
interface MeshSession {
  roles: Map<UserId, "HOST" | "COHOST" | "PARTICIPANT">;

  // Permissions
  canShare(user: UserId, content: ShareableContent): boolean;
  canModifySession(user: UserId): boolean;

  // Consent
  requestSharing(user: UserId): Promise<boolean>;
  revokeSharing(user: UserId): void;

  // Safety
  enableInterferenceShield(): void;
  leaveImmediately(): void;  // Always available, no confirmation needed
}
```

**Engineering Tasks**:
- [ ] Implement role-based permission matrix
- [ ] Add explicit opt-in for any shared visuals
- [ ] Create InterferenceShield (calm fallback + ritual pause)
- [ ] Ensure "Leave" is always one tap, no confirmation

---

### Module 7: Marketplace [REQ 0027-0031] (Full Build)

**Symbolic Description**: "Community sharing with protective review"

**Technical Implementation**:
```typescript
interface Marketplace {
  // Content submission
  submitBlueprint(blueprint: Blueprint): Promise<SubmissionResult>;
  submitPersona(persona: OperatorPersona): Promise<SubmissionResult>;

  // Safety scanning
  scanContent(content: MarketplaceContent): Promise<ScanResult>;
  applyTrustLabel(content: MarketplaceContent): TrustLabel;

  // Governance
  reportContent(contentId: string, reason: ReportReason): void;
  takedownContent(contentId: string, reason: TakedownReason): void;

  // Anti-dark-patterns
  showPricing(content: MarketplaceContent): TransparentPricing;
  enableEasyCancellation(): void;
}
```

**Engineering Tasks**:
- [ ] Implement PersonaSafetyScan (blocks coercive tone)
- [ ] Create blueprint validator (blocks medical claims, guaranteed outcomes)
- [ ] Add trust labeling system
- [ ] Implement report/takedown workflow
- [ ] Ensure no urgency manipulation in purchase flows

---

## Hardware Integration [REQ UCF-*]

### WebSocket Protocol

```typescript
// Default endpoint
const WS_ENDPOINT = "ws://ucf-device.local:81/ws";

// Message types
type MessageType = "STATE_UPDATE" | "EVENT" | "COMMAND" | "COMMAND_RESPONSE";

// Update rate: 10 Hz for state, immediate for events
```

### Key Hardware Events

| Event | App Response |
|-------|-------------|
| `PHASE_TRANSITION` | Update UI phase indicator, adjust audio tier |
| `TRIAD_UNLOCK` | Enable advanced features, provide haptic feedback |
| `K_FORMATION_ACHIEVED` | Trigger peak experience mode |
| `SYNCHRONIZATION_LOST` | Graceful degradation, continue without sync |

### Critical Constants

```typescript
const UCF_CONSTANTS = {
  PHI: 1.6180339887,          // Golden ratio
  PHI_INV: 0.6180339887,      // UNTRUE→PARADOX boundary
  Z_CRITICAL: 0.8660254038,   // PARADOX→TRUE boundary (√3/2)

  TRIAD_HIGH: 0.85,           // Rising threshold
  TRIAD_LOW: 0.82,            // Re-arm threshold
  TRIAD_PASSES: 3,            // Required crossings

  K_KAPPA: 0.92,              // Coherence threshold
  K_ETA: 0.618,               // Negentropy threshold
  K_R: 7,                     // Minimum resonance
};
```

---

## Testing Requirements

### Safety Compliance Tests

Every build MUST pass:
1. **Consent Gate Test**: High-impact action blocked without consent
2. **Emergency Stop Test**: Session terminates within 100ms of trigger
3. **SafeMinimalMode Test**: Fallback activates on renderer failure
4. **Medical Claim Scan**: Blueprint with "cure/heal/treat" is rejected
5. **Privacy Test**: No data transmitted without explicit opt-in

### Adversarial Testing

Before release, simulate:
1. Voice spoofing → Confirm blocked by confidence + confirmation
2. Coercive persona → Confirm blocked by PersonaSafetyScan
3. Mesh pressure → Confirm instant leave works
4. Rapid session cycling → Confirm no resource leaks

---

## Implementation Priority

### MVP (Phase 1)
1. App Engine (state machine)
2. Safety Layer (all gates)
3. Bed Mode renderer (Preview2D)
4. Local storage (encrypted)
5. Education hub

### Full Build (Phases 2-4)
1. Voice interface
2. Walking mode
3. AR backend
4. Mesh sessions
5. Marketplace

---

## Sign-off Checklist

Before marking module complete:

- [ ] All requirements traced (cite [XXXX] in code comments)
- [ ] Safety Layer integration verified
- [ ] No medical/guaranteed-outcome language in UI
- [ ] Emergency stop accessible from all states
- [ ] Privacy audit passed (no unexpected data collection)
- [ ] Adversarial scenario tested

---

**Document Version**: 1.0
**Last Updated**: December 2024
**Specification Source**: WishBed Spread NLP v2
