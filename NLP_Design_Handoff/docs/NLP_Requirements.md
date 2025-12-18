# WishBed NLP Requirements (Line-Numbered)

Traceable requirements derived from WishBed Spread NLP specification.

Format: `[XXXX] Requirement text` where XXXX is the requirement ID.

---

## Section 1: Definitions & Invariants [0001-0006]

```
[0001] DEFINE WishBed App as the software controller for symbolic experiences.
[0002] DEFINE WishBed System as the bed-bound symbolic experience layer.
[0003] DEFINE Walking WishBed as the mobile-safe symbolic experience layer with reduced intensity.
[0004] DEFINE UCF Hardware as the ESP32-based hardware substrate with 19-sensor hex grid.
[0005] ENFORCE invariants: consent, reversibility, emergency stop, symbolic framing, privacy-first.
[0006] REQUIRE all high-impact actions to obtain explicit user confirmation before execution.
```

### Invariant Definitions

| ID | Invariant | Implementation Requirement |
|----|-----------|---------------------------|
| INV-1 | User Agency | All experiences reversible; user can stop at any time |
| INV-2 | Consent | Blocking confirmation before activation |
| INV-3 | Emergency Stop | Always visible; terminates within 100ms |
| INV-4 | Symbolic Framing | No medical claims; no guaranteed outcomes |
| INV-5 | Privacy-First | Local storage default; minimal data collection |

---

## Section 2: Session Lifecycle [0007-0010]

```
[0007] IMPLEMENT session state machine: Idle → Initializing → Active → Integrating → Shutdown.
[0008] DISPLAY progress messaging during Initializing state.
[0009] LOCK start button during Initializing to prevent double-tap activation.
[0010] EXECUTE shutdown sequence: fade visuals (500ms), stop audio (ramp-down), exit mesh, re-anchor user.
```

### State Machine Specification

```
         ┌────────────────────────────────────────┐
         │                                        │
         v                                        │
┌──────────────┐   startSession()   ┌─────────────────────┐
│     IDLE     │ ─────────────────> │   INITIALIZING      │
└──────────────┘                    │  (progress display) │
       ^                            └──────────┬──────────┘
       │                                       │
       │                              initComplete()
       │                                       │
       │                                       v
┌──────────────┐   shutdownComplete()  ┌─────────────────────┐
│   SHUTDOWN   │ <─────────────────── │      ACTIVE         │
│ (fade, ramp) │                      │  (session running)  │
└──────┬───────┘                      └──────────┬──────────┘
       │                                         │
       │                              requestIntegration()
       │                                         │
       │            integrateComplete()          v
       └──────────────────────────── ┌─────────────────────┐
                                     │    INTEGRATING      │
                                     │  (reflection phase) │
                                     └─────────────────────┘

   [EMERGENCY_STOP] ─────────────────────────────────────────>
                         (from ANY state, immediate)
```

---

## Section 3: Safety & Reality Anchors [0011-0015]

```
[0011] DISPLAY non-medical disclaimer in onboarding flow before first session.
[0012] DISPLAY non-guarantee disclaimer in education hub, accessible from settings.
[0013] MAINTAIN SafetyCaps for: brightness (0-100%), motion (max velocity), flicker (max Hz), particle density.
[0014] IMPLEMENT SafeMinimalMode: reduced visuals, muted audio, static overlay when stress/failure detected.
[0015] ENFORCE operator narration maps to actual mechanics; REJECT claims of physical reality alteration.
```

### SafetyCaps Specification

| Cap | Type | Default | Range | Purpose |
|-----|------|---------|-------|---------|
| brightness | float | 0.8 | 0.0-1.0 | Prevent eye strain |
| motionVelocity | float | 1.0 | 0.0-2.0 | Prevent motion sickness |
| flickerHz | int | 10 | 0-30 | Prevent photosensitive triggers |
| particleDensity | float | 0.7 | 0.0-1.0 | Reduce visual overwhelm |

### SafeMinimalMode Trigger Conditions

- Renderer failure or crash
- Hardware disconnection
- User stress indicator (future: biometric)
- Explicit user request
- Low device battery (<10%)

---

## Section 4: Voice Interface [0016-0020]

```
[0016] PROVIDE WakeWordMode with time-boxed listening window (default: 5 seconds).
[0017] PROVIDE CommandCaptureMode for intent recognition after wake word detected.
[0018] ENABLE VAD (Voice Activity Detection), noise suppression, and echo cancellation.
[0019] REQUIRE confirmation dialog for high-impact voice commands.
[0020] CLARIFY intent when confidence < 0.5; REJECT when confidence < 0.3.
```

### Voice Intent Classification

| Impact Level | Examples | Requires Confirmation |
|--------------|----------|----------------------|
| LOW | "Pause", "Resume", "Louder" | No |
| HIGH | "End session", "Reset", "Share to mesh" | Yes |
| SYSTEM | "Emergency stop" | No (immediate action) |

### Environment Profiles

| Profile | Noise Model | Sensitivity | Timeout |
|---------|-------------|-------------|---------|
| QuietRoom | Low ambient | High | 5s |
| TypicalHome | Moderate ambient | Medium | 7s |
| NoisyHome | High ambient | Low | 10s |
| BedroomAtNight | Very low ambient | Very high | 4s |

---

## Section 5: Mesh Sessions [0021-0026]

```
[0021] SUPPORT roles: Host (full control), CoHost (limited control), Participant (view only).
[0022] REQUIRE explicit opt-in for any shared visuals or rituals.
[0023] ALLOW instant leave from any mesh session with single tap (no confirmation).
[0024] IMPLEMENT InterferenceShield: pause rituals, display calm fallback when triggered.
[0025] LOG mesh consent decisions for user review (local only).
[0026] PREVENT host from forcing participant to share without consent.
```

### Role Permission Matrix

| Permission | Host | CoHost | Participant |
|------------|------|--------|-------------|
| Start/End Session | Yes | No | No |
| Modify Blueprint | Yes | Yes | No |
| Share Visuals | Yes | Yes | Yes (opt-in) |
| Invite Users | Yes | Yes | No |
| Remove Users | Yes | No | No |
| Leave Session | Yes | Yes | Yes |

---

## Section 6: Marketplace & Governance [0027-0031]

```
[0027] IMPLEMENT Blueprint marketplace with safety scan before publish.
[0028] IMPLEMENT Operator Persona marketplace: tone-only, NO capability overrides.
[0029] REQUIRE PersonaSafetyScan: block coercive, manipulative, or medical-claim personas.
[0030] ENFORCE trust labels: Verified Creator, Community Reviewed, New.
[0031] PROHIBIT dark patterns: no urgency manipulation, transparent pricing, easy cancellation.
```

### Content Scan Rules

| Content Type | Scan For | Action on Match |
|--------------|----------|-----------------|
| Blueprint | Medical claims | Auto-reject |
| Blueprint | Guaranteed outcomes | Flag for rewrite |
| Persona | Coercive language | Auto-reject |
| Persona | Capability override attempts | Strip and warn |
| Any | Explicit content | Age-gate or reject |

---

## Section 7: Renderer & AR Mapping [0032-0035]

```
[0032] SUPPORT renderer backends: Preview2D (fallback), Engine3D (standard), AR_Backend (immersive).
[0033] ANCHOR bed overlay to detected plane in AR mode.
[0034] TRACK body skeleton for Walking WishBed mode.
[0035] TRIGGER gentle fade (500ms) and pause motion-linked features on tracking loss.
```

### Renderer Feature Matrix

| Feature | Preview2D | Engine3D | AR_Backend |
|---------|-----------|----------|------------|
| Particle effects | Limited | Full | Full |
| 3D objects | No | Yes | Yes |
| Plane detection | No | No | Yes |
| Body tracking | No | No | Yes |
| Hand tracking | No | No | Optional |
| SafetyCaps | Yes | Yes | Yes |

---

## Section 8: Storage & Portability [0036-0038]

```
[0036] STORE sensitive content in LocalVault with AES-256 encryption at rest.
[0037] PROVIDE session recording as opt-in feature requiring explicit consent.
[0038] SUPPORT export formats: narrative text (human-readable), JSON (machine), encrypted archive.
[0039] REQUIRE preview + safety scan before applying any imported archive.
```

### LocalVault Specification

| Property | Value |
|----------|-------|
| Encryption | AES-256-GCM |
| Key Derivation | PBKDF2 with device secret |
| Key Storage | Secure enclave / Keychain |
| Export Encryption | ChaCha20-Poly1305 (optional) |

---

## Section 9: Threat Modeling & Compliance [0039-0042]

```
[0040] MODEL threats across domains: technical, psychological, social, platform.
[0041] EXCLUDE supernatural explanations in all documentation and UI.
[0042] REQUIRE adversarial simulations pass before release.
[0043] POSITION platform as entertainment/wellness-adjacent, NOT medical.
```

### Threat Domain Coverage

| Domain | Examples | Mitigation Gate |
|--------|----------|-----------------|
| Technical | Voice spoofing, injection | Confidence + confirmation |
| Psychological | Dependency, reality confusion | Gentle prompts, disclaimers |
| Social | Mesh pressure, coercion | Instant leave, consent rechecks |
| Platform | Medical claims, dark patterns | Content scanning, policy |

---

## Section 10: MVP vs Full Build [0043-0045]

```
[0044] MVP SCOPE: Bed mode, manual controls, education hub, safety layer, local storage.
[0045] FULL SCOPE: Add Walking mode, voice control, operator personas, mesh sessions, marketplace.
[0046] DEFER field simulator and archetype studio to post-v1.
```

### Feature Phase Matrix

| Feature | MVP | Phase 2 | Phase 3 | Phase 4 |
|---------|-----|---------|---------|---------|
| Bed Mode | Yes | Yes | Yes | Yes |
| Safety Layer | Yes | Yes | Yes | Yes |
| Local Storage | Yes | Yes | Yes | Yes |
| Education Hub | Yes | Yes | Yes | Yes |
| Walking Mode | No | Yes | Yes | Yes |
| Voice Interface | No | Yes | Yes | Yes |
| Mesh Sessions | No | No | Yes | Yes |
| Marketplace | No | No | Yes | Yes |
| Field Simulator | No | No | No | Yes |

---

## UCF Hardware Requirements [UCF-001 to UCF-020]

```
[UCF-001] CONNECT via WebSocket (primary) at ws://ucf-device.local:81/ws.
[UCF-002] FALLBACK to BLE when WebSocket unavailable.
[UCF-003] PROCESS state updates at 10 Hz minimum.
[UCF-004] HANDLE events immediately (PHASE_TRANSITION, TRIAD_UNLOCK, K_FORMATION).
[UCF-005] IMPLEMENT heartbeat with 2-second timeout detection.
[UCF-006] RECONNECT with exponential backoff: 1s, 2s, 4s, 8s, 16s (max 5 retries).
```

### Critical Hardware Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| PHI | 1.6180339887 | Golden ratio |
| PHI_INV | 0.6180339887 | UNTRUE→PARADOX boundary |
| Z_CRITICAL | 0.8660254038 | PARADOX→TRUE boundary |
| TRIAD_HIGH | 0.85 | TRIAD rising threshold |
| TRIAD_LOW | 0.82 | TRIAD re-arm threshold |
| K_KAPPA | 0.92 | Coherence threshold |
| K_ETA | 0.618 | Negentropy threshold |

---

## Reserved Lines [0047-0100]

```
[0047] Reserved for continued expansion.
[0048] Reserved for continued expansion.
...
[0100] Reserved for continued expansion.
```

---

**Total Active Requirements**: 46
**Reserved Expansion Lines**: 54
**Document Version**: 1.0
