# Non-Negotiable System Invariants

These invariants MUST be preserved in all implementations. Violation of any invariant is a blocking issue.

---

## INV-1: User Agency Always

**Statement**: All experiences are reversible and user-controlled.

**Implementation Requirements**:
- No action may lock user into irreversible state
- User can pause, modify, or exit at any time
- No "point of no return" in any flow
- All session data can be deleted by user

**Validation Test**:
```
GIVEN user is in any state
WHEN user requests stop/pause/exit
THEN system complies within 100ms
AND no data is lost without explicit user choice
```

---

## INV-2: Consent Before Activation

**Statement**: Every high-impact action requires explicit user confirmation.

**Implementation Requirements**:
- High-impact actions blocked until consent granted
- Consent dialogs must be clear, not manipulative
- "Cancel" must be equally prominent as "Confirm"
- Consent state persisted but revocable

**High-Impact Actions**:
| Action | Consent Type | Revocable |
|--------|-------------|-----------|
| Start session | Session consent | Yes |
| Enable microphone | Microphone consent | Yes |
| Record session | Recording consent | Yes |
| Join mesh | Mesh consent | Yes |
| Share to marketplace | Publishing consent | No* |

*Cannot unpublish already-downloaded content

**Validation Test**:
```
GIVEN high-impact action is triggered
WHEN consent has NOT been granted
THEN action is BLOCKED
AND consent dialog is displayed
AND action proceeds ONLY after explicit confirmation
```

---

## INV-3: Emergency Stop Priority

**Statement**: Emergency stop is always visible, always functional, highest priority.

**Implementation Requirements**:
- Emergency stop button visible in all active states
- Z-index higher than all other UI elements
- Touch target minimum 44x44 points
- No confirmation required for emergency stop
- Terminates session within 100ms

**Emergency Stop Sequence**:
```
1. IMMEDIATELY halt all audio output
2. IMMEDIATELY halt all visual animations
3. IMMEDIATELY disconnect from mesh (if connected)
4. TRANSITION to Idle state
5. DISPLAY confirmation of stop
6. PRESERVE session data for later review (if recording enabled)
```

**Validation Test**:
```
GIVEN system is in any active state
WHEN emergency stop is triggered
THEN all output stops within 100ms
AND system transitions to Idle
AND no confirmation dialog blocks the stop
```

---

## INV-4: Symbolic Framing Only

**Statement**: No literal claims, no guaranteed outcomes, no medical language.

**Implementation Requirements**:
- All UI copy uses experiential/metaphorical language
- Prohibited words list enforced in user-generated content
- Blueprints scanned before publishing
- Operator personas cannot override this framing

**Prohibited Language Patterns**:
| Pattern | Replacement Suggestion |
|---------|----------------------|
| "cure", "heal", "treat" | "explore", "experience" |
| "guaranteed", "will work" | "may support", "designed to" |
| "diagnosis", "medical" | [rejected, cannot replace] |
| "therapy", "treatment" | "practice", "ritual" |
| "scientifically proven" | [rejected, cannot replace] |

**Validation Test**:
```
GIVEN content contains prohibited language
WHEN content is submitted (UI text, blueprint, persona)
THEN content is BLOCKED or auto-rewritten
AND user is notified of the issue
```

---

## INV-5: Privacy-First

**Statement**: Local-first storage, minimal data collection, user controls data.

**Implementation Requirements**:
- All sensitive data stored locally by default
- No server transmission without explicit opt-in
- Encryption at rest (AES-256)
- User can export all their data
- User can delete all their data permanently

**Data Classification**:
| Data Type | Storage | Transmission | User Control |
|-----------|---------|--------------|--------------|
| Session recordings | Local encrypted | Never (unless export) | Full delete |
| Blueprints | Local encrypted | Opt-in marketplace | Full delete |
| Consent state | Local | Never | Full delete |
| Usage analytics | Local aggregate | Opt-in only | Disable/delete |
| Crash reports | Local | Opt-in only | Disable/delete |

**Validation Test**:
```
GIVEN user has NOT opted into data sharing
WHEN any operation completes
THEN NO data is transmitted to external servers
AND all data is encrypted at rest
AND data deletion is complete and unrecoverable
```

---

## Invariant Enforcement Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     UI Layer                            │
│    [All actions must pass through Safety Layer]         │
└────────────────────────┬────────────────────────────────┘
                         │
                         v
┌─────────────────────────────────────────────────────────┐
│                   SAFETY LAYER                          │
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │ Consent     │  │ Emergency   │  │ Content     │     │
│  │ Manager     │  │ Stop        │  │ Scanner     │     │
│  │ [INV-2]     │  │ [INV-3]     │  │ [INV-4]     │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                         │
│  ┌─────────────┐  ┌─────────────┐                      │
│  │ Privacy     │  │ Agency      │                      │
│  │ Guard       │  │ Monitor     │                      │
│  │ [INV-5]     │  │ [INV-1]     │                      │
│  └─────────────┘  └─────────────┘                      │
│                                                         │
└────────────────────────┬────────────────────────────────┘
                         │
                         v
┌─────────────────────────────────────────────────────────┐
│              App Engine / Subsystems                    │
│    [Only receives validated, consented actions]         │
└─────────────────────────────────────────────────────────┘
```

---

## Invariant Violation Response

If any invariant is violated in production:

1. **Immediate**: Trigger SafeMinimalMode
2. **Log**: Record violation details locally
3. **Notify**: Alert user of degraded state
4. **Block**: Prevent further actions until resolved
5. **Report**: Optional crash/error report (with consent)

---

**Document Version**: 1.0
**Classification**: Non-Negotiable
