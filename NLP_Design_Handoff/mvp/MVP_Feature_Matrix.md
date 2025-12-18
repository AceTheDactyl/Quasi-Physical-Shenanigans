# MVP vs Full Build Feature Matrix

Clear delineation of MVP scope vs full build features.

---

## Build Phases Overview

```
Phase 1 (MVP)     Phase 2          Phase 3          Phase 4
─────────────────────────────────────────────────────────────
│ Bed Mode      │ Walking Mode   │ Mesh Sessions  │ Field Sim    │
│ Safety Layer  │ Voice Control  │ Marketplace    │ Archetype    │
│ Local Storage │ Recording      │ Creator Tools  │ Studio       │
│ Education Hub │ Export/Import  │ Trust System   │ Advanced AR  │
─────────────────────────────────────────────────────────────
```

---

## Feature Matrix

### Core System

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| App Engine State Machine | Yes | Yes | Yes | Yes | [0007-0010] |
| Safety Layer (all gates) | Yes | Yes | Yes | Yes | [0011-0015] |
| Emergency Stop | Yes | Yes | Yes | Yes | [INV-3] |
| Consent Management | Yes | Yes | Yes | Yes | [INV-2] |
| SafetyCaps Engine | Yes | Yes | Yes | Yes | [0013] |
| SafeMinimalMode | Yes | Yes | Yes | Yes | [0014] |

### Experience Modes

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| Bed Mode | Yes | Yes | Yes | Yes | [0002] |
| Walking Mode | No | Yes | Yes | Yes | [0003] |
| Preview2D Renderer | Yes | Yes | Yes | Yes | [0032] |
| 3D Engine Renderer | Optional | Yes | Yes | Yes | [0032] |
| AR Backend | No | No | Optional | Yes | [0032-0035] |
| Plane Detection | No | No | Optional | Yes | [0033] |
| Body Tracking | No | No | Optional | Yes | [0034] |

### Voice & Operator

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| Manual Controls Only | Yes | Optional | Optional | Optional | - |
| Wake Word Detection | No | Yes | Yes | Yes | [0016] |
| Voice Commands | No | Yes | Yes | Yes | [0017] |
| VAD/Noise Suppression | No | Yes | Yes | Yes | [0018] |
| Voice Confirmation | No | Yes | Yes | Yes | [0019] |
| Environment Profiles | No | Yes | Yes | Yes | [0020] |
| Neutral Operator | Yes | Yes | Yes | Yes | - |
| Custom Personas | No | No | Yes | Yes | [0028] |

### Storage & Privacy

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| LocalVault (encrypted) | Yes | Yes | Yes | Yes | [0036] |
| Session Recording | No | Yes | Yes | Yes | [0037] |
| JSON Export | No | Yes | Yes | Yes | [0038] |
| Narrative Text Export | No | Yes | Yes | Yes | [0038] |
| Encrypted Archive | No | Yes | Yes | Yes | [0038] |
| Import with Scan | No | Yes | Yes | Yes | [0039] |

### Mesh Sessions

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| Solo Sessions Only | Yes | Yes | Optional | Optional | - |
| Host Role | No | No | Yes | Yes | [0021] |
| CoHost Role | No | No | Yes | Yes | [0021] |
| Participant Role | No | No | Yes | Yes | [0021] |
| Shared Visuals (opt-in) | No | No | Yes | Yes | [0022] |
| Instant Leave | No | No | Yes | Yes | [0023] |
| InterferenceShield | No | No | Yes | Yes | [0024] |

### Marketplace

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| Blueprint Marketplace | No | No | Yes | Yes | [0027] |
| Persona Marketplace | No | No | Yes | Yes | [0028] |
| Safety Scanning | No | No | Yes | Yes | [0027-0029] |
| Trust Labels | No | No | Yes | Yes | [0030] |
| Report/Takedown | No | No | Yes | Yes | [0027] |
| Creator Verification | No | No | Yes | Yes | [0030] |

### Hardware Integration

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| WebSocket Connection | Yes | Yes | Yes | Yes | [UCF-001] |
| BLE Fallback | Yes | Yes | Yes | Yes | [UCF-002] |
| State Updates (10Hz) | Yes | Yes | Yes | Yes | [UCF-003] |
| Event Handling | Yes | Yes | Yes | Yes | [UCF-004] |
| Heartbeat/Reconnect | Yes | Yes | Yes | Yes | [UCF-005-006] |
| Phase Display | Yes | Yes | Yes | Yes | - |
| TRIAD Unlock | Yes | Yes | Yes | Yes | - |
| K-Formation | Optional | Yes | Yes | Yes | - |

### Education & Onboarding

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 | Requirement |
|---------|-----|---------|---------|---------|-------------|
| Onboarding Flow | Yes | Yes | Yes | Yes | [0011] |
| Disclaimer Acknowledgment | Yes | Yes | Yes | Yes | [0011] |
| Education Hub | Yes | Yes | Yes | Yes | [0012] |
| FAQ | Yes | Yes | Yes | Yes | - |
| Troubleshooting | Yes | Yes | Yes | Yes | - |

---

## MVP Scope Definition

### Included in MVP

1. **Core Experience**
   - Bed Mode only
   - Manual controls (buttons, sliders)
   - Preview2D or basic 3D renderer
   - SafetyCaps (brightness, motion, flicker, density)

2. **Safety System**
   - Full consent management
   - Emergency stop
   - SafeMinimalMode fallback
   - All invariants enforced

3. **Hardware Integration**
   - WebSocket + BLE connection
   - Phase detection and display
   - TRIAD unlock
   - Basic commands

4. **Education**
   - Complete onboarding flow
   - Disclaimer acknowledgment
   - Education hub with all safety information

5. **Storage**
   - Encrypted local storage
   - Settings persistence
   - Consent state persistence

### Explicitly Excluded from MVP

1. **Walking Mode** - Phase 2
2. **Voice Control** - Phase 2
3. **Session Recording** - Phase 2
4. **Export/Import** - Phase 2
5. **Mesh Sessions** - Phase 3
6. **Marketplace** - Phase 3
7. **Custom Personas** - Phase 3
8. **AR Backend** - Phase 4
9. **Field Simulator** - Phase 4

---

## Technical Dependencies

### MVP Dependencies

```
Required for MVP:
├── React Native (or equivalent)
├── WebSocket client
├── BLE library
├── AsyncStorage (or SQLite)
├── Crypto library (AES-256)
└── Basic 2D/3D rendering
```

### Phase 2 Additions

```
Phase 2 adds:
├── Speech-to-text engine
├── Voice synthesis
├── VAD library
├── File system access
└── Export formatting
```

### Phase 3 Additions

```
Phase 3 adds:
├── Real-time messaging (WebRTC/Socket.io)
├── Content moderation pipeline
├── Payment integration
└── Backend services
```

### Phase 4 Additions

```
Phase 4 adds:
├── ARKit/ARCore integration
├── Body tracking SDK
├── Advanced shader support
└── Machine learning models
```

---

## MVP Acceptance Criteria

Before MVP release, verify:

### Functional Requirements
- [ ] User can complete onboarding
- [ ] User can acknowledge disclaimer
- [ ] User can start Bed Mode session
- [ ] Session state machine transitions correctly
- [ ] SafetyCaps apply to rendering
- [ ] Emergency stop terminates within 100ms
- [ ] Hardware connects via WebSocket
- [ ] Hardware falls back to BLE if needed
- [ ] Phase transitions display correctly

### Safety Requirements
- [ ] All 5 invariants pass testing
- [ ] Consent gates block without consent
- [ ] SafeMinimalMode activates on failure
- [ ] No medical claims in any UI text
- [ ] Emergency stop visible in all active states

### Privacy Requirements
- [ ] All data encrypted at rest
- [ ] No network transmission without opt-in
- [ ] Data can be deleted permanently

### Adversarial Requirements
- [ ] Scenario 1 (Voice Spoofing) - N/A for MVP
- [ ] Scenario 5 (Overuse Prompt) - Pass
- [ ] Scenario 6 (Malicious Import) - N/A for MVP

---

## Migration Notes

### MVP → Phase 2

```
New modules:
- VoiceIO (voice input/output)
- SessionRecorder (recording consent + storage)
- ExportManager (format conversion)
- ImportManager (scan + preview)

Modified modules:
- AppEngine (add voice intent handling)
- SafetyLayer (add voice confirmation)
- LocalVault (add session storage schema)
```

### Phase 2 → Phase 3

```
New modules:
- MeshClient (real-time connection)
- MarketplaceClient (content browsing/publishing)
- ContentScanner (safety scanning service)
- PersonaManager (persona loading/validation)

Modified modules:
- SafetyLayer (add PersonaSafetyScan)
- Renderer (add shared visual layer)

New infrastructure:
- Backend service for mesh signaling
- Content moderation service
- Marketplace database
```

---

## Timeline Notes

**Important**: This document intentionally does not include time estimates. Implementation timelines should be determined by the engineering team based on:
- Available resources
- Technical complexity assessment
- Dependency resolution
- Testing requirements

Each phase should be considered complete only when all acceptance criteria pass, regardless of calendar time.

---

**Document Version**: 1.0
