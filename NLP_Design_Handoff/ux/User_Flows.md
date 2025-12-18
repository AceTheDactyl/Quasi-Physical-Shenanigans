# WishBed User Flows

Detailed user flow definitions for implementation.

---

## Flow Overview

```
┌────────────┐    ┌────────────┐    ┌────────────┐    ┌────────────┐
│  ONBOARD   │───>│  EDUCATE   │───>│  DASHBOARD │───>│  SESSION   │
│  (First    │    │  (Learn    │    │  (Home     │    │  (Active   │
│   Launch)  │    │   System)  │    │   Screen)  │    │   Use)     │
└────────────┘    └────────────┘    └────────────┘    └────────────┘
                                           │                │
                                           │                │
                                           v                v
                                    ┌────────────┐    ┌────────────┐
                                    │  SETTINGS  │    │ INTEGRATE  │
                                    │  (Config)  │    │  (Exit)    │
                                    └────────────┘    └────────────┘
```

---

## Flow 1: First Launch Onboarding

**Purpose**: Introduce system, collect initial consent, set expectations

**Steps**:
```
1. WELCOME SCREEN
   - Display: App name, brief tagline
   - Action: "Get Started" button
   - Skip: Not allowed on first launch

2. SYSTEM OVERVIEW
   - Display: What WishBed does (symbolic experiences)
   - Display: What WishBed is NOT (medical, therapeutic)
   - Action: "I Understand" button
   - Skip: Not allowed

3. DISCLAIMER ACKNOWLEDGMENT [REQ 0011]
   - Display: Full non-medical disclaimer
   - Display: Non-guarantee statement
   - Required: Checkbox "I acknowledge this is not medical treatment"
   - Action: "Continue" (enabled only when checked)
   - Skip: Not allowed

4. SAFETY INTRODUCTION
   - Display: Emergency stop explanation
   - Display: SafetyCaps explanation
   - Display: How to exit anytime
   - Action: "I Understand Safety Features"

5. CONSENT COLLECTION (Initial)
   - Display: Microphone permission explanation
   - Action: "Allow" or "Skip for now"
   - Note: Session consent collected later

6. COMPLETE
   - Display: "You're ready!"
   - Action: "Go to Dashboard"
   - Store: onboarding_complete = true
```

**Exit Conditions**:
- Complete: User reaches Dashboard
- Abandon: User closes app (restart onboarding next launch)

---

## Flow 2: Education Hub

**Purpose**: Provide detailed information about the system

**Access**: From Dashboard or Settings

**Sections**:
```
EDUCATION HUB
├── What is WishBed?
│   ├── Overview (text + diagrams)
│   ├── The Symbolic Experience (metaphor explanation)
│   └── Hardware Integration (optional reading)
│
├── How to Use
│   ├── Starting a Session
│   ├── During a Session
│   ├── Ending a Session
│   └── Voice Commands (if enabled)
│
├── Safety & Wellbeing
│   ├── Non-Medical Disclaimer [REQ 0011, 0012]
│   ├── Emergency Stop
│   ├── SafetyCaps Explained
│   └── When to Seek Professional Help
│
├── Privacy
│   ├── What Data We Store
│   ├── What We Never Collect
│   ├── Export Your Data
│   └── Delete Your Data
│
└── FAQ
    ├── Common Questions
    ├── Troubleshooting
    └── Contact Support
```

---

## Flow 3: Dashboard (Home)

**Purpose**: Central hub for all app actions

**Layout**:
```
┌─────────────────────────────────────────────────────────┐
│  [Settings Gear]                    [Education Book]    │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                    WISHBED                              │
│              [Connection Status]                        │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│          ┌─────────────────────────┐                   │
│          │                         │                   │
│          │     START SESSION       │                   │
│          │        [Button]         │                   │
│          │                         │                   │
│          └─────────────────────────┘                   │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Recent Sessions                                        │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐         │
│  │ Session 1  │ │ Session 2  │ │ Session 3  │         │
│  │ Dec 15     │ │ Dec 14     │ │ Dec 13     │         │
│  └────────────┘ └────────────┘ └────────────┘         │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**Actions**:
| Element | Tap Action |
|---------|------------|
| Settings Gear | Navigate to Settings |
| Education Book | Navigate to Education Hub |
| Connection Status | Show connection details |
| Start Session | Begin session flow |
| Recent Session Card | View session details |

---

## Flow 4: Session Start

**Purpose**: Initialize and start a session with consent

**Steps**:
```
1. MODE SELECTION
   - Display: "Bed Mode" vs "Walking Mode" (if available)
   - MVP: Only Bed Mode available
   - Action: Select mode

2. CONSENT CHECK [REQ 0005, 0006]
   - If session consent not granted:
     - Display: Session consent dialog
     - Required: Explicit "Start Session" confirmation
   - If already granted:
     - Skip to step 3

3. HARDWARE CHECK
   - Display: Connecting to UCF Hardware
   - Success: Proceed to step 4
   - Failure: Show fallback options
     - "Continue without hardware"
     - "Try again"
     - "Cancel"

4. INITIALIZATION [REQ 0008, 0009]
   - Display: Progress indicator
   - Display: "Preparing your experience..."
   - Lock: Start button disabled (prevent double-tap)
   - Duration: Variable (hardware dependent)

5. SESSION ACTIVE
   - Transition to Session View
   - State: ACTIVE
```

**Consent Dialog UI**:
```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│            Ready to begin your session?                 │
│                                                         │
│  You can stop at any time using the emergency stop      │
│  button, which is always visible during your session.   │
│                                                         │
│  Remember: This is a symbolic experience, not medical   │
│  treatment.                                             │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │               START SESSION                      │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│                    [Cancel]                             │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Flow 5: Active Session

**Purpose**: Provide session controls and visualization

**Layout**:
```
┌─────────────────────────────────────────────────────────┐
│  [< Back]                      [EMERGENCY STOP]         │
│                                 (red, always visible)   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                                                         │
│              ┌───────────────────────┐                  │
│              │                       │                  │
│              │   VISUALIZATION       │                  │
│              │   AREA                │                  │
│              │                       │                  │
│              │   (Renderer output)   │                  │
│              │                       │                  │
│              └───────────────────────┘                  │
│                                                         │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Phase: [UNTRUE / PARADOX / TRUE]    z: 0.xxx          │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  [Pause]      [Intensity -/+]       [End Session]      │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**Controls**:
| Control | Action | Requires Confirmation |
|---------|--------|----------------------|
| Emergency Stop | Immediate termination | No |
| Back | Leave session | Yes |
| Pause | Pause/Resume | No |
| Intensity | Adjust SafetyCaps | No |
| End Session | Begin integration | Yes |

**Emergency Stop Behavior** [REQ INV-3]:
- Always visible (highest z-index)
- Minimum touch target: 44x44pt
- No confirmation dialog
- Terminates within 100ms
- Returns to Dashboard

---

## Flow 6: Session End / Integration

**Purpose**: Graceful exit with optional reflection

**Steps**:
```
1. END REQUEST
   - User taps "End Session"
   - Display: Confirmation dialog
     "End your session and begin integration?"
     [End Session] [Continue]

2. INTEGRATION PHASE [REQ 0010]
   - State: INTEGRATING
   - Display: Fade out visuals (500ms)
   - Display: Ramp down audio
   - Display: "Taking a moment to integrate..."

3. REFLECTION (Optional)
   - Display: "How was your experience?"
   - Options: Quick feedback (1-5 stars or skip)
   - Optional: Add notes

4. SESSION SUMMARY
   - Display: Duration, phase progression
   - Display: "Session complete"
   - Action: "Return to Dashboard"

5. RETURN
   - State: IDLE
   - Navigate: Dashboard
```

**Integration Screen**:
```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│                                                         │
│                   Taking a moment                       │
│                   to integrate...                       │
│                                                         │
│              [Gentle visual fade]                       │
│                                                         │
│                                                         │
│                                                         │
│                                                         │
│                    Duration: 10s                        │
│               (auto-proceed when complete)              │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Flow 7: Settings

**Purpose**: Configure app behavior and manage data

**Structure**:
```
SETTINGS
├── Safety
│   ├── SafetyCaps Configuration
│   │   ├── Brightness (slider 0-100%)
│   │   ├── Motion Intensity (slider 0-100%)
│   │   ├── Flicker Rate (slider, with warning)
│   │   └── Particle Density (slider 0-100%)
│   ├── Reset to Defaults
│   └── SafeMinimalMode Toggle
│
├── Consent Management
│   ├── Session Consent (grant/revoke)
│   ├── Microphone Consent (grant/revoke)
│   ├── Recording Consent (grant/revoke)
│   └── Analytics Consent (grant/revoke)
│
├── Voice (if enabled)
│   ├── Enable/Disable Voice Control
│   ├── Environment Profile
│   └── Wake Word Sensitivity
│
├── Hardware
│   ├── Connection Status
│   ├── Device ID
│   ├── Calibration
│   └── Reconnect
│
├── Data & Privacy
│   ├── View Stored Data
│   ├── Export Data
│   │   ├── JSON Format
│   │   ├── Narrative Text
│   │   └── Encrypted Archive
│   ├── Import Data
│   └── Delete All Data (destructive, requires confirmation)
│
├── About
│   ├── Version
│   ├── Licenses
│   └── Support
│
└── [Return to Dashboard]
```

---

## Error States

### Hardware Connection Lost

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│          Hardware connection lost                       │
│                                                         │
│  Your session can continue with limited features.       │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │               CONTINUE                           │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │               TRY RECONNECT                      │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│                  [End Session]                          │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Renderer Failure (SafeMinimalMode)

```
┌─────────────────────────────────────────────────────────┐
│  [EMERGENCY STOP]                                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                   Safe Mode Active                      │
│                                                         │
│           Visual features are reduced.                  │
│        Your session continues safely.                   │
│                                                         │
│              [Solid calm color]                         │
│                                                         │
│                                                         │
├─────────────────────────────────────────────────────────┤
│  [End Session]                                          │
└─────────────────────────────────────────────────────────┘
```

---

## Accessibility Requirements

| Requirement | Implementation |
|-------------|----------------|
| Touch targets | Minimum 44x44pt |
| Color contrast | WCAG AA minimum |
| Screen reader | All controls labeled |
| Reduce motion | Respect system preference |
| Emergency stop | Visible in all states |

---

**Document Version**: 1.0
