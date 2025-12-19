# WishBed App System Integration Test Simulation

> **Comprehensive integration analysis and test specification for the WishBed symbolic experience architecture.**

This document provides a detailed analysis of module integration, identifies gaps and ambiguities, recommends fixes, and outlines step-by-step implementation guidance for developers.

---

## Table of Contents

1. [Architecture and Module Overview](#architecture-and-module-overview)
2. [Module Responsibilities and Data Flow](#module-responsibilities-and-data-flow)
3. [Integration Gaps and Ambiguities](#integration-gaps-and-ambiguities)
4. [Recommended Fixes and Additions](#recommended-fixes-and-additions-to-unify-the-system)
5. [User-Facing Consequences of Integration Errors](#user-facing-consequences-of-integration-errors)
6. [Step-by-Step Developer Guide](#step-by-step-guide-for-developers-to-bridge-integration-gaps)

---

## Architecture and Module Overview

The WishBed App is composed of multiple interlinked modules, each with a distinct role in the symbolic experience architecture.

### Key Modules

| Module | Role |
|--------|------|
| **UI Layer** | User interface and interaction |
| **App Engine** | Session state machine orchestrator |
| **State Manager** | Symbolic state and hardware readings |
| **Safety Layer** | Protective checks and consent verification |
| **Voice Interface** | Voice commands and guidance |
| **Audio/Visual Renderer** | Sensory experience output |
| **Mesh Service** | Multi-user session networking |
| **Marketplace** | Community content distribution |
| **Field Simulator** | Symbolic state visualization |
| **Operator System** | Guided persona/AI helper |

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        WISHBED APP ARCHITECTURE                              │
│                                                                              │
│  ┌──────────┐     ┌──────────────┐     ┌───────────────┐                   │
│  │    UI    │────▶│  App Engine  │────▶│ Safety Layer  │                   │
│  │  Layer   │     │  (State FSM) │     │   (Guardian)  │                   │
│  └──────────┘     └──────────────┘     └───────────────┘                   │
│       │                  │                     │                            │
│       ▼                  ▼                     ▼                            │
│  ┌──────────┐     ┌──────────────┐     ┌───────────────┐                   │
│  │  Voice   │     │    State     │     │   Renderer    │                   │
│  │Interface │     │   Manager    │     │    Adapter    │                   │
│  └──────────┘     └──────────────┘     └───────────────┘                   │
│       │                  │                     │                            │
│       ▼                  ▼                     ▼                            │
│  ┌──────────┐     ┌──────────────┐     ┌───────────────┐                   │
│  │ Operator │     │   Hardware   │     │  WishBed      │                   │
│  │  System  │     │   Adapter    │     │  System       │                   │
│  └──────────┘     └──────────────┘     └───────────────┘                   │
│                          │                                                  │
│                          ▼                                                  │
│                   ┌──────────────┐                                          │
│                   │  UCF Device  │                                          │
│                   │  (ESP32)     │                                          │
│                   └──────────────┘                                          │
│                                                                              │
│  Optional Services:                                                          │
│  ┌──────────┐     ┌──────────────┐     ┌───────────────┐                   │
│  │   Mesh   │     │ Marketplace  │     │  LocalVault   │                   │
│  │ Service  │     │   (Content)  │     │  (Storage)    │                   │
│  └──────────┘     └──────────────┘     └───────────────┘                   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### UX Flow Lifecycle

The typical user experience proceeds through discrete phases:

```
Onboarding → Education → Dashboard (Idle) → Session Start → Active Experience → Integration → Exit
```

| Phase | Description | Active Modules |
|-------|-------------|----------------|
| **Onboarding** | User understands symbolic framing, consents | UI, Safety |
| **Education** | Learning system capabilities | UI, Voice |
| **Dashboard** | Idle state, session selection | UI, State Manager |
| **Session Start** | Initialize ritual session | Engine, Safety, Hardware |
| **Active Experience** | Real-time sensor/renderer interaction | All modules |
| **Integration** | Post-session reflection | UI, Field Simulator, LocalVault |
| **Exit** | Graceful shutdown | Engine, Safety |

---

## Module Responsibilities and Data Flow

### UI Layer

- Presents controls and feedback to the user
- Triggers engine actions (e.g., "Start Session" button)
- Displays prompts (consent dialogs, emergency stop) and status indicators
- Always interacts with Safety Layer for high-impact operations
- Reflects state changes from Engine

### App Engine (Session State Machine)

Core orchestrator managing session states and lifecycle:

```
States: IDLE → INITIALIZING → ACTIVE → INTEGRATING → SHUTDOWN
```

- Implemented as finite state machine with guarded transitions
- Guards always include safety checks (e.g., no ACTIVE without consent)
- Invokes other modules as needed
- Ensures no module bypasses Safety Layer

### State Manager

- Maintains current symbolic state and hardware readings
- Interfaces with Hardware Adapter for real-time sensor updates
- Provides unified state context for Engine and UI
- Holds current phase (UNTRUE, PARADOX, TRUE) based on z-coordinate
- Tracks flags like TRIAD unlock status

### Safety Layer

Enforces all safety, consent, and compliance invariants system-wide:

- Consent verification
- Content filtering
- Rate limiting
- Emergency intervention
- Safety caps on parameters (brightness, motion intensity)
- Reality anchoring (rewriting medical claims)

**Key Methods:**
- `checkConsent(action)` — Verify user consent for action
- `validateBlueprint(content)` — Scan content for policy violations
- `applySafetyCaps(params)` — Enforce safe parameter limits
- `triggerEmergencyStop()` — Immediate session termination
- `rewriteMedicalClaims(text)` — Transform literal claims to symbolic

### Voice Interface

- Operates in modes: wake-word listening, command capture, inactive
- Processes speech into intents using NLP
- Maps intents to allowed app actions
- Tags intents with impact level and confirmation requirements
- High-impact intents require explicit user confirmation via UI

### Audio/Visual Renderer

- Handles user's sensory experience
- Plugs into different backends (2D preview, 3D engine, AR mode)
- Modes: BED (stationary) and WALKING (AR with movement)
- Applies Safety Caps to all outputs
- Monitors tracking and context (pauses on tracking loss)

### Mesh Service (Optional)

- Manages multi-user session interactions
- Defines roles and permissions
- Provides "Interference Shield" for participant protection
- Guarantees immediate exit without confirmation
- Requires explicit consent for all shared content

### Marketplace (Optional)

- Governed content marketplace for Blueprints and Personas
- Content submission workflows with safety scanning
- Trust labels and provenance metadata
- No dark patterns or hidden costs

### Field Simulator

- Symbolic state visualization
- Displays hexagonal sensor grid and activation patterns
- Renders Kuramoto synchronization visually
- Subscribes to State Manager data

### Operator System (Ascension Intelligence)

- Guided persona or AI helper for narration
- Different personas provide different tones/scripts
- Personas cannot bypass safety or trigger actions
- Content must pass safety scanning

---

## Integration Gaps and Ambiguities

### 1. State Manager vs App Engine Responsibility

**Issue:** Separation between Engine (session state machine) and State Manager could be clearer.

**Ambiguity:** When a hardware STATE_UPDATE arrives, does the Hardware Adapter directly update the Engine's state machine, or update a State Manager that the Engine consults?

**Risk:** Duplicate state handling or race conditions.

**Recommendation:** Define explicit StateManager interface for reading/writing session variables and sensor values.

### 2. Blueprint (SessionConfiguration) Injection

**Issue:** Blueprint concept is central but runtime integration not fully specified.

**Gap:** No pseudocode for blueprint handling — how does Engine load and use blueprints?

**Questions:**
- Is blueprint compiled into state machine actions?
- Does Engine iterate through blueprint "stages"?

**Recommendation:** Define Blueprint Schema and loading pipeline (parse JSON → validate → create execution plan → execute).

### 3. UI and Voice Command Convergence

**Issue:** Both UI and Voice trigger actions, but convergence point unclear.

**Risk:** Voice might accidentally bypass confirmation that UI has.

**Recommendation:** Implement single Command Dispatcher with unified safety checks for both input modalities.

### 4. Walking Mode vs Bed Mode Integration

**Issue:** Mode switching not fully described.

**Questions:**
- How does Engine adapt when switching modes?
- What sensors are used in Walking mode without bed hardware?
- How does AR tracking data map to symbolic state?

**Recommendation:** Document Mode Integration Guide with explicit handling for each mode.

### 5. Field Simulator Utilization

**Issue:** Implementation not detailed — separate module or Renderer mode?

**Ambiguity:** Is it a diagnostic tool or user-facing visualization?

**Recommendation:** Define explicit interface and integration point.

### 6. Operator Persona Content Pipeline

**Issue:** How persona content is delivered during sessions not specified.

**Questions:**
- Does blueprint include persona-specific narration?
- Is persona just voice style or custom scripts?

**Recommendation:** Define Persona Script Interface with standard moments/phrases.

### 7. Marketplace to Local System Handoff

**Issue:** Import flow not fully documented.

**Gap:** Download → scan → store → display pipeline not specified.

**Recommendation:** Formalize content import workflow with explicit steps.

### 8. Safety Layer Invocation Consistency

**Issue:** Every path must use Safety checks, but implementation could miss some.

**Risk:** Hardware events triggering intense effects without safety consultation.

**Recommendation:** Audit all module boundaries for safety hooks.

### 9. Error and Interrupt Handling

**Issue:** Behavior under stress/errors not fully defined.

**Questions:**
- What happens on hardware disconnect mid-session?
- How is rapid session cycling handled?

**Recommendation:** Implement explicit error handling routines and resource cleanup.

---

## Recommended Fixes and Additions to Unify the System

### 1. Define Clear State Management Interface

```typescript
interface StateManager {
  // Phase management
  getPhase(): Phase;
  setPhase(phase: Phase): void;

  // Sensor state
  getSensor(fieldId: number): SensorReading;
  updateSensorState(data: StateUpdate): void;

  // Session state
  getSessionState(): SessionState;
  setSessionState(state: SessionState): void;

  // Event subscriptions
  subscribe(event: StateEvent, callback: Function): void;
}
```

### 2. Establish Blueprint Loading Workflow

```
1. Parsing: BlueprintParser reads JSON → structured object
2. Validation: SafetyLayer.validateBlueprint() → pass/fail
3. Execution Plan: Convert to state machine events/timeline
4. Module Integration: Map elements to Renderer/Voice/UI commands
```

### 3. Unify Command Handling via Central Dispatcher

```typescript
class CommandDispatcher {
  async executeCommand(action: AppAction, source: 'UI' | 'Voice'): Promise<void> {
    // Check if high-impact
    if (isHighImpact(action)) {
      const consent = await SafetyLayer.checkConsent(action);
      if (!consent) return; // Abort
    }

    // Execute via Engine
    await this.engine.execute(action);
  }
}
```

### 4. Mode Switch Handling (Bed ↔ Walking)

```typescript
async switchMode(newMode: 'BED' | 'WALKING'): Promise<void> {
  // 1. Adjust sensor inputs
  if (newMode === 'WALKING') {
    await this.hardwareAdapter.enableARTracking();
  } else {
    await this.hardwareAdapter.enableBedSensors();
  }

  // 2. Switch renderer backend
  await this.renderer.setOverlayMode(newMode);

  // 3. Re-calibrate safety caps for mode
  await SafetyLayer.applySafetyCaps(getModeSpecificCaps(newMode));
}
```

### 5. Implement Field Simulator

**Option A (User-facing):** Specialized Renderer mode for post-session replay.

**Option B (Developer tool):** Hardware Adapter simulation mode generating synthetic events.

### 6. Persona Integration via Voice Scripts

```typescript
interface PersonaScript {
  id: string;
  name: string;
  voiceProfile: VoiceProfile;
  phrases: {
    sessionStart: string;
    midSessionEncouragement: string;
    sessionEnd: string;
    // ... standard moments
  };
}
```

### 7. Content Import/Export Pipeline

```
Download Content
    ↓
Save to LocalVault (encrypted)
    ↓
Mark as "new/unverified"
    ↓
Run SafetyLayer.scanContent()
    ↓
If pass: Mark available, add Trust Label
If fail: Reject, notify user
```

### 8. Strengthen Safety Hooks

- Route all hardware events through Safety
- Implement Emergency Stop in multiple paths (UI, Voice, Hardware)
- Apply Reality Anchor at content display time
- Log all safety checks for audit

### 9. Explicit Error Handling Routines

```typescript
// Hardware disconnect handling
onHardwareDisconnect(): void {
  this.engine.pauseSession();
  SafetyLayer.enterSafeMinimalMode();
  this.ui.showReconnectDialog();
}

// Resource cleanup on session end
async cleanupSession(): Promise<void> {
  await this.hardwareAdapter.stopStreaming();
  await this.renderer.fadeOut();
  await this.voiceInterface.deactivate();
  this.clearAllTimers();
}
```

---

## User-Facing Consequences of Integration Errors

| Integration Gap | User Consequence |
|-----------------|------------------|
| **Voice bypasses safety** | Unauthorized actions via voice spoofing |
| **Consent gates failing** | User launched into intense experiences without consent |
| **Safety caps not applied** | Visual discomfort, headaches, potential seizures |
| **Unfiltered content** | Exposure to coercive or misleading messaging |
| **Exit blocked** | Panic, loss of control during distress |
| **State desynchronization** | Confusing, glitchy experience |
| **Feature inconsistency** | Unpredictable behavior, broken immersion |
| **Privacy breaches** | Data leakage without consent |

---

## Step-by-Step Guide for Developers to Bridge Integration Gaps

### Step 1: Establish Central State Machine and Manager

1. Code App Engine FSM with states: `IDLE, INITIALIZING, ACTIVE, INTEGRATING, SHUTDOWN`
2. Incorporate Safety Layer guard in each transition
3. Implement StateManager with sensor subscriptions
4. Connect state machine to StateManager
5. Handle Emergency Stop from any state
6. Write unit tests for state transitions

### Step 2: Integrate Safety Layer in All Paths

```typescript
// UI actions
if (await SafetyLayer.checkConsent(StartSession)) {
  engine.startSession();
}

// Voice intents
const intent = VoiceInterface.parseIntent(text);
if (intent.requiresConfirmation) {
  const confirmed = await SafetyLayer.requestConsent(intent);
  if (!confirmed) return;
}
engine.execute(intent.mappedAction);

// Content loading
const isValid = SafetyLayer.validateBlueprint(blueprintJson);
if (!isValid) {
  showValidationErrors();
  return;
}
```

### Step 3: Connect Hardware Adapter and Field Simulation

1. Implement UCFHardwareAdapter for WebSocket/BLE
2. On STATE_UPDATE, call `StateManager.setFieldState()`
3. Listen for events (TRIAD_UNLOCK, K_FORMATION_ACHIEVED)
4. Create Field Simulator mode in Renderer or Hardware Adapter
5. Test with real and simulated data

### Step 4: Implement Voice Interface and Operator Persona

1. Implement wake-word detection and intent parsing
2. Map phrases to actions with impact classification
3. Integrate Operator Persona for scripted lines
4. Route all voice-driven actions through safety checks
5. Test voice spoof scenario

### Step 5: Mesh Networking and Immediate Exit

1. Implement MeshSession with participants and roles
2. Share request shows Accept/Decline with equal prominence
3. Implement `leaveImmediately()` with no confirmation
4. Add Interference Shield functionality
5. Test mesh pressure scenario

### Step 6: Marketplace Integration and Content Safety

1. Implement listing, download, and publish flows
2. Run Safety scans on all downloaded content
3. Save to LocalVault with Trust Labels
4. Block uploads that fail safety scan
5. Test blueprint publishing scenario

### Step 7: Comprehensive Testing

#### Safety Compliance Tests

| Test | Validation |
|------|------------|
| Consent Gate | High-impact action blocked without confirmation |
| Emergency Stop | Session terminates within 100ms of trigger |
| SafeMinimalMode | Complex visuals stop on renderer failure |
| Medical Claim Scan | Blueprint with "cure" rejected |
| Privacy | No unexpected network calls or data leakage |

#### Adversarial Scenario Simulations

| Scenario | Test Approach |
|----------|---------------|
| Voice Spoof | Input dangerous command, verify confirmation required |
| Coercive Persona | Load persona with bad lines, verify rejection |
| Mesh Pressure | Simulate host/participant, verify immediate leave |
| Rapid Cycling | Start/stop sessions in loop, check for leaks |

#### End-to-End Integration Test

```
1. Onboarding → Education demo
2. Connect hardware/simulator
3. Experience phase changes (simulate z-coordinate crossing thresholds)
4. Trigger TRIAD unlock event
5. Finish session
6. View integration summary
7. Include voice commands and mode switching
```

### Step 8: Documentation and Developer Notes

- Write "How to add a new voice command safely" guide
- Create "Content Creation Guidelines" for blueprints/personas
- Maintain Integration Checklist for module sign-off
- Document regex patterns used for safety scanning

---

## Test Coverage Matrix

| Module | Safety Integration | State Integration | Event Handling | Error Recovery |
|--------|-------------------|-------------------|----------------|----------------|
| UI Layer | ✓ Consent dialogs | ✓ State reflection | ✓ User events | ✓ Error display |
| App Engine | ✓ Guard transitions | ✓ State machine | ✓ Event dispatch | ✓ Emergency stop |
| State Manager | ✓ Validation | ✓ Central store | ✓ Subscriptions | ✓ Recovery state |
| Safety Layer | ✓ Self-validation | ✓ State checks | ✓ Emergency | ✓ Fallback mode |
| Voice Interface | ✓ Confirmation | ✓ Intent mapping | ✓ Wake word | ✓ Low confidence |
| Renderer | ✓ Safety caps | ✓ Mode state | ✓ Tracking loss | ✓ Minimal mode |
| Hardware Adapter | ✓ Event safety | ✓ Sensor state | ✓ Disconnect | ✓ Reconnect |
| Mesh Service | ✓ Shield/exit | ✓ Participant state | ✓ Share requests | ✓ Graceful leave |
| Marketplace | ✓ Content scan | ✓ Library state | ✓ Download/upload | ✓ Rejection flow |

---

## See Also

- [WishBed App README](../README.md) — App overview
- [TDD Specification](./TDD.md) — Technical design document
- [UX Flow](../ux/UX_Flow.md) — User experience design
- [Safety Documentation](../safety/) — Adversarial scenarios and threat model
- [UCF Hardware Concepts](../../docs/UCF_HARDWARE_CONCEPTS.md) — Hardware firmware specification
- [Integration Guide](../../docs/INTEGRATION.md) — Communication protocols

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024-12-19 | Initial system integration test simulation |

---

**Test Status**: Ready for Implementation
**Coverage**: All modules analyzed
**Gaps Identified**: 9 major integration points
**Recommendations**: 9 unification fixes
