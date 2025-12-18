# WishBed System Architecture

Technical architecture for the WishBed App and UCF Hardware integration.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        WishBed App (Client)                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                      UI LAYER                                │   │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │   │
│  │  │Dashboard │ │ Session  │ │ Settings │ │Education │        │   │
│  │  │  View    │ │   View   │ │   View   │ │   Hub    │        │   │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘        │   │
│  └────────────────────────────┬────────────────────────────────┘   │
│                               │                                     │
│  ┌────────────────────────────v────────────────────────────────┐   │
│  │                    APP ENGINE                                │   │
│  │  ┌────────────────────────────────────────────────────────┐ │   │
│  │  │              Session State Machine                      │ │   │
│  │  │  IDLE ─> INITIALIZING ─> ACTIVE ─> INTEGRATING ─> IDLE │ │   │
│  │  └────────────────────────────────────────────────────────┘ │   │
│  └────────────────────────────┬────────────────────────────────┘   │
│                               │                                     │
│  ┌────────────────────────────v────────────────────────────────┐   │
│  │                  SAFETY LAYER (Mandatory)                    │   │
│  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐            │   │
│  │  │Consent  │ │SafetyCap│ │Emergency│ │ Content │            │   │
│  │  │Manager  │ │ Engine  │ │  Stop   │ │ Scanner │            │   │
│  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘            │   │
│  └─────┬───────────┬────────────┬────────────┬─────────────────┘   │
│        │           │            │            │                      │
│  ┌─────v─────┐ ┌───v───┐ ┌─────v─────┐ ┌────v────┐ ┌──────────┐   │
│  │ Renderer  │ │ Voice │ │  Local    │ │  Mesh   │ │Marketplace│   │
│  │  Adapter  │ │  I/O  │ │  Vault    │ │ Client  │ │  Client  │   │
│  └─────┬─────┘ └───────┘ └───────────┘ └─────────┘ └──────────┘   │
│        │                                                            │
└────────┼────────────────────────────────────────────────────────────┘
         │
         │  WebSocket / BLE
         │
┌────────v────────────────────────────────────────────────────────────┐
│                     UCF HARDWARE (ESP32)                            │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │                    MAIN CONTROLLER                            │  │
│  │  ┌────────────┐ ┌────────────┐ ┌────────────┐                │  │
│  │  │   Phase    │ │   TRIAD    │ │  Kuramoto  │                │  │
│  │  │   Engine   │ │    FSM     │ │ Stabilizer │                │  │
│  │  └────────────┘ └────────────┘ └────────────┘                │  │
│  └──────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────────┐   │
│  │  HEX GRID      │  │   EMANATION    │  │    SIGIL           │   │
│  │  (19 sensors)  │  │  (Audio/LED)   │  │   STORAGE          │   │
│  └────────────────┘  └────────────────┘  └────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Module Specifications

### 1. UI Layer

**Purpose**: Present user interface, capture user input

**Components**:
| Component | Purpose | Key Features |
|-----------|---------|--------------|
| Dashboard View | Home screen | Session history, quick start |
| Session View | Active session UI | Controls, visualizations, emergency stop |
| Settings View | Configuration | SafetyCaps, consent management, export |
| Education Hub | Information | Disclaimers, tutorials, FAQ |

**Dependencies**: App Engine (state), Safety Layer (consent)

---

### 2. App Engine

**Purpose**: Manage session lifecycle and coordinate subsystems

**State Machine**:
```typescript
interface SessionStateMachine {
  state: "IDLE" | "INITIALIZING" | "ACTIVE" | "INTEGRATING" | "SHUTDOWN";

  transitions: {
    startSession: (mode: OverlayMode) => Promise<void>;
    completeInit: () => void;
    requestIntegration: () => void;
    completeIntegration: () => void;
    shutdown: () => void;
    emergencyStop: () => void;  // Available from any state
  };

  guards: {
    canStart: () => boolean;     // Requires consent
    canActivate: () => boolean;  // Requires safety layer ready
  };
}
```

**Dependencies**: Safety Layer (guards), Renderer (visualization)

---

### 3. Safety Layer

**Purpose**: Enforce invariants, manage consent, apply safety caps

**Sub-modules**:

| Module | Responsibility | Key Methods |
|--------|----------------|-------------|
| ConsentManager | Track/request consent | `checkConsent()`, `requestConsent()`, `revokeConsent()` |
| SafetyCapEngine | Apply visual/audio limits | `applyBrightness()`, `applyMotion()`, `applyFlicker()` |
| EmergencyStop | Immediate termination | `trigger()`, `isActive()` |
| ContentScanner | Validate user content | `scanBlueprint()`, `scanPersona()`, `scanImport()` |

**Interface**:
```typescript
interface SafetyLayer {
  // Consent
  hasConsent(type: ConsentType): boolean;
  requestConsent(type: ConsentType): Promise<boolean>;

  // Caps
  getSafetyCaps(): SafetyCaps;
  updateSafetyCaps(caps: Partial<SafetyCaps>): void;
  applyCaps<T>(value: T, capType: CapType): T;

  // Emergency
  triggerEmergencyStop(): void;
  enterSafeMinimalMode(): void;

  // Content
  scanContent(content: Scannable): Promise<ScanResult>;
}
```

---

### 4. Renderer Adapter

**Purpose**: Abstract rendering backend, apply visual effects

**Backends**:
| Backend | Use Case | Features |
|---------|----------|----------|
| Preview2D | Fallback, low-power | Basic shapes, colors, opacity |
| Engine3D | Standard | Full 3D, particles, shaders |
| AR_Backend | Immersive | Plane detection, body tracking |

**Interface**:
```typescript
interface RendererAdapter {
  backend: RendererBackend;
  overlayMode: "BED" | "WALKING";

  initialize(config: RendererConfig): Promise<void>;
  render(frame: RenderFrame): void;
  setOverlay(overlay: Overlay): void;

  // AR-specific
  anchorToPlane?(planeId: string): void;
  trackBody?(skeleton: BodySkeleton): void;
  onTrackingLoss?(callback: () => void): void;

  // Safety integration
  applySafetyCaps(caps: SafetyCaps): void;
  enterSafeMinimalMode(): void;
}
```

---

### 5. Voice I/O

**Purpose**: Capture voice input, synthesize voice output

**Pipeline**:
```
Microphone → VAD → Noise Suppression → Echo Cancel → STT → Intent Parser
                                                            ↓
                                              SafetyLayer.validateIntent()
                                                            ↓
                                                    Action Executor
```

**Interface**:
```typescript
interface VoiceIO {
  mode: "INACTIVE" | "WAKE_WORD" | "COMMAND_CAPTURE";

  startListening(): void;
  stopListening(): void;
  setEnvironmentProfile(profile: EnvironmentProfile): void;

  onIntent(callback: (intent: VoiceIntent) => void): void;
  onWakeWord(callback: () => void): void;

  speak(text: string, persona?: OperatorPersona): Promise<void>;
}
```

---

### 6. Local Vault

**Purpose**: Encrypted local storage for sensitive data

**Storage Schema**:
```typescript
interface LocalVaultSchema {
  sessions: Map<SessionId, EncryptedSession>;
  blueprints: Map<BlueprintId, EncryptedBlueprint>;
  consent: ConsentState;
  settings: UserSettings;
  exportHistory: ExportRecord[];
}
```

**Security**:
- AES-256-GCM encryption
- Device-bound key derivation (PBKDF2)
- Secure enclave key storage where available
- No cloud sync without explicit opt-in

---

### 7. Hardware Adapter

**Purpose**: Bridge app to UCF hardware via WebSocket/BLE

**Protocol**:
```typescript
interface HardwareMessage {
  type: "STATE_UPDATE" | "EVENT" | "COMMAND" | "COMMAND_RESPONSE";
  version: string;
  timestamp: number;
  payload: MessagePayload;
}

interface HardwareAdapter {
  connectionState: ConnectionState;
  deviceState: UCFHardwareState | null;

  connect(deviceId?: string): Promise<void>;
  disconnect(): Promise<void>;
  sendCommand(command: UCFCommand): Promise<CommandResponse>;

  onStateUpdate(callback: (state: UCFHardwareState) => void): void;
  onEvent(callback: (event: UCFHardwareEvent) => void): void;
}
```

**Connection Flow**:
```
1. Attempt WebSocket (ws://ucf-device.local:81/ws)
2. If fails, fallback to BLE scan
3. On connect: subscribe to state updates
4. Maintain heartbeat (2s interval)
5. On disconnect: exponential backoff reconnect (max 5 retries)
```

---

## Data Flow Diagrams

### Session Start Flow

```
User                UI              AppEngine         SafetyLayer       Renderer
  │                  │                  │                  │                │
  │ Tap "Start"      │                  │                  │                │
  ├─────────────────>│                  │                  │                │
  │                  │ startSession()   │                  │                │
  │                  ├─────────────────>│                  │                │
  │                  │                  │ checkConsent()   │                │
  │                  │                  ├─────────────────>│                │
  │                  │                  │                  │                │
  │                  │<─ ─ ─ ─ ─ ─ ─ ─ ─│<─ ─ ─ ─ ─ ─ ─ ─ │                │
  │                  │                  │                  │                │
  │<─ ─ ─ ─ ─ ─ ─ ─ ─│ Consent Dialog   │                  │                │
  │                  │                  │                  │                │
  │ Confirm          │                  │                  │                │
  ├─────────────────>│                  │                  │                │
  │                  │ grantConsent()   │                  │                │
  │                  ├─────────────────>│                  │                │
  │                  │                  │ recordConsent()  │                │
  │                  │                  ├─────────────────>│                │
  │                  │                  │                  │                │
  │                  │                  │ initialize()     │                │
  │                  │                  ├─────────────────────────────────>│
  │                  │                  │                  │                │
  │                  │<─ ─ ─ ─ ─ ─ ─ ─ ─│<─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─│
  │                  │                  │                  │                │
  │<─ ─ ─ ─ ─ ─ ─ ─ ─│ Session Active   │                  │                │
```

### Emergency Stop Flow

```
User                UI              AppEngine         SafetyLayer       All Systems
  │                  │                  │                  │                │
  │ Tap STOP         │                  │                  │                │
  ├─────────────────>│                  │                  │                │
  │                  │ emergencyStop()  │                  │                │
  │                  ├─────────────────────────────────────>│                │
  │                  │                  │                  │ HALT ALL       │
  │                  │                  │                  ├───────────────>│
  │                  │                  │                  │                │
  │                  │                  │<─ ─ ─ ─ ─ ─ ─ ─ ─│<─ ─ ─ ─ ─ ─ ─ │
  │                  │                  │ setState(IDLE)   │                │
  │                  │<─ ─ ─ ─ ─ ─ ─ ─ ─│                  │                │
  │<─ ─ ─ ─ ─ ─ ─ ─ ─│ "Stopped"        │                  │                │
  │                  │                  │                  │                │

Total time: < 100ms
```

---

## Deployment Architecture

### Mobile App (React Native)

```
┌─────────────────────────────────────────────────────────┐
│                   React Native App                       │
├─────────────────────────────────────────────────────────┤
│  UI Components (React)                                   │
│  State Management (Redux/Context)                        │
│  Navigation (React Navigation)                           │
├─────────────────────────────────────────────────────────┤
│  Native Modules                                          │
│  ├─ WebSocket (react-native-websocket)                  │
│  ├─ BLE (react-native-ble-plx)                          │
│  ├─ Crypto (react-native-keychain)                      │
│  ├─ Voice (react-native-voice)                          │
│  └─ AR (ViroReact / ARKit / ARCore)                     │
├─────────────────────────────────────────────────────────┤
│  Platform: iOS / Android                                 │
└─────────────────────────────────────────────────────────┘
```

### UCF Hardware (ESP32)

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32-WROOM-32                        │
├─────────────────────────────────────────────────────────┤
│  Main Loop (Arduino Framework)                           │
│  ├─ HexField (19 capacitive sensors via MPR121)         │
│  ├─ PhaseEngine (z-coordinate calculation)              │
│  ├─ TriadFSM (3-crossing unlock gate)                   │
│  ├─ KuramotoStabilizer (8-oscillator network)           │
│  ├─ Emanation (WS2812B LEDs + audio DAC)                │
│  └─ SigilStorage (AT24C16 EEPROM)                       │
├─────────────────────────────────────────────────────────┤
│  Communications                                          │
│  ├─ WebSocket Server (port 81)                          │
│  └─ BLE Server (UCF service UUID)                       │
└─────────────────────────────────────────────────────────┘
```

---

**Document Version**: 1.0
