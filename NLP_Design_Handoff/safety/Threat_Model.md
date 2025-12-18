# WishBed Threat Model

Comprehensive threat modeling for the WishBed system.

---

## Threat Model Overview

The WishBed system operates in a sensitive domain where misuse could cause psychological harm. This threat model addresses four domains:

1. **Technical Misuse** - System exploitation, unauthorized access
2. **Psychological Misinterpretation** - Reality confusion, dependency
3. **Social Coercion** - Pressure from other users
4. **Platform Violations** - Content policy breaches

---

## Domain 1: Technical Misuse

### T1.1: Voice Spoofing Attack

**Threat**: Attacker plays audio to trigger voice commands without user consent

**Attack Vector**:
- External speaker playing wake word + command
- Media playback containing voice commands
- Ultrasonic audio injection

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Confidence threshold | Reject commands below 0.7 confidence |
| Confirmation dialog | All high-impact commands require tap confirmation |
| Liveness detection | Future: Require verbal confirmation for high-impact |
| Emergency isolation | Emergency stop always works, cannot be voice-triggered |

**Residual Risk**: Low (confirmation dialog blocks most attacks)

---

### T1.2: Malicious Blueprint Injection

**Threat**: User imports a blueprint containing harmful content or exploits

**Attack Vector**:
- Malformed JSON causing crashes
- Blueprints with prohibited content
- Blueprints designed to trigger psychological distress

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Schema validation | Strict JSON schema enforcement |
| Content scanning | Prohibited language patterns detected |
| Preview before apply | User sees preview before import completes |
| Sandbox execution | Blueprint cannot access system resources |

**Residual Risk**: Low (multiple validation layers)

---

### T1.3: Hardware Protocol Exploitation

**Threat**: Attacker sends malicious commands to UCF hardware

**Attack Vector**:
- Man-in-the-middle on local network
- Malicious app sending WebSocket commands
- BLE spoofing

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Local-only network | Hardware only accepts local connections |
| Command validation | Hardware validates all command payloads |
| Rate limiting | Hardware ignores rapid command bursts |
| SafetyCaps enforcement | Hardware enforces caps regardless of commands |

**Residual Risk**: Medium (local network attacks possible but require physical proximity)

---

### T1.4: Data Exfiltration

**Threat**: Attacker extracts user session data

**Attack Vector**:
- Unauthorized app access to storage
- Export without consent
- Memory dump attacks

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Encryption at rest | AES-256-GCM for all sensitive data |
| Device-bound keys | Keys stored in secure enclave |
| Export requires auth | Device authentication before export |
| No cloud sync default | Data never leaves device without opt-in |

**Residual Risk**: Low (defense in depth)

---

## Domain 2: Psychological Misinterpretation

### T2.1: Reality Confusion

**Threat**: User begins to believe symbolic experiences are literal reality

**Risk Factors**:
- Extended session duration
- Immersive AR mode
- Personalized operator persona
- Vulnerable mental state

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Onboarding disclaimer | Non-medical, non-literal framing required |
| Education hub | Accessible explanations of symbolic nature |
| Integration phase | Post-session grounding period |
| Reality anchors | Periodic gentle reminders during long sessions |

**Detection**: Not automated; relies on user self-awareness

**Residual Risk**: Medium (inherent in experiential systems)

---

### T2.2: Dependency Formation

**Threat**: User develops unhealthy reliance on WishBed for emotional regulation

**Risk Factors**:
- Daily extended use
- Use as coping mechanism replacement
- Avoidance of professional help
- Social isolation

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Gentle prompts | Non-judgmental usage awareness prompts |
| NO lockouts | Never lock user out (can exacerbate distress) |
| Professional resources | Links to mental health resources in Education |
| Disclaimer | "Not a replacement for professional support" |

**Policy**: We inform, we do not restrict access

**Residual Risk**: Medium (cannot prevent user choices)

---

### T2.3: Guaranteed Outcome Expectation

**Threat**: User expects specific outcomes from sessions (healing, transformation)

**Attack Vector**:
- User-created blueprints with outcome claims
- Marketplace content with implied guarantees
- Operator personas making promises

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Content scanning | Prohibited patterns: "will cure", "guaranteed", "heal" |
| Auto-rewrite | Flagged content suggested for revision |
| Rejection | Medical claims auto-rejected from marketplace |
| Onboarding | Clear non-guarantee statement acknowledged |

**Residual Risk**: Low (automated scanning + policy)

---

## Domain 3: Social Coercion

### T3.1: Mesh Host Pressure

**Threat**: Host pressures participants to share personal content or continue session

**Attack Vector**:
- "Everyone is sharing, why won't you?"
- Withholding positive experience until compliance
- Social pressure in group setting

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Instant leave | One-tap exit, no confirmation, no questions |
| Explicit opt-in | Sharing requires explicit per-item consent |
| Role limits | Participants cannot be forced to share |
| Consent rechecks | Periodic consent confirmation in long sessions |
| Anonymous leave | Host not notified of reason for leave |

**Residual Risk**: Low (strong participant protections)

---

### T3.2: Coercive Operator Persona

**Threat**: Persona uses manipulative or coercive language patterns

**Attack Vector**:
- "You must continue..."
- "Real practitioners don't quit..."
- Guilt-inducing narration
- False authority claims

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| PersonaSafetyScan | NLP scan for coercive patterns |
| Tone-only limitation | Personas cannot override capabilities |
| User reporting | Report mechanism with review queue |
| Auto-block | Patterns matching manipulation blocked |

**Detection Patterns**:
- Imperative commands without user agency language
- Guilt/shame inducing phrases
- False expertise claims ("as a doctor...")
- Urgency pressure ("you must act now")

**Residual Risk**: Low (automated + human review)

---

### T3.3: Relationship Coercion

**Threat**: Partner/family member pressures user into WishBed use

**Attack Vector**:
- "This will fix your problems"
- Using WishBed as relationship tool without full consent
- One partner controlling the other's session

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Individual consent | Each user maintains own consent state |
| Emergency stop | Always accessible to active user |
| Education | Information about healthy use in Education Hub |
| Disclaimer | "This is not couples therapy" |

**Residual Risk**: Medium (cannot prevent all interpersonal dynamics)

---

## Domain 4: Platform Violations

### T4.1: Medical Claims in Content

**Threat**: Marketplace content makes medical or therapeutic claims

**Regulatory Risk**: App store rejection, legal liability

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Pre-publish scan | All content scanned before listing |
| Prohibited terms | "diagnose", "treat", "cure", "medical", "therapy" |
| Manual review | Flagged content requires human approval |
| Takedown workflow | Reported content removed within 24h |
| Creator education | Guidelines clearly prohibit medical claims |

**Residual Risk**: Low (automated + human review + takedown)

---

### T4.2: Dark Patterns

**Threat**: Marketplace or app uses manipulative UI patterns

**Examples**:
- "Only 2 left!" (false scarcity)
- Confusing cancellation flows
- Hidden fees
- Urgency manipulation

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| No urgency UI | No countdown timers or scarcity messaging |
| Transparent pricing | Full cost shown before purchase |
| Easy cancellation | Cancel as easy as purchase |
| No dark nudges | Equal prominence for all options |

**Audit**: Regular UX audit against dark pattern checklist

**Residual Risk**: Low (policy + design standards)

---

### T4.3: Explicit/Harmful Content

**Threat**: Marketplace contains explicit, violent, or harmful content

**Mitigations**:
| Mitigation | Requirement |
|------------|-------------|
| Content moderation | Automated + human review |
| Age gating | Mature content behind verification (if allowed) |
| Reporting | User reporting with rapid response |
| Creator ban | Repeat violators permanently banned |

**Residual Risk**: Low (standard content moderation)

---

## Threat Matrix Summary

| ID | Threat | Severity | Likelihood | Mitigated Risk |
|----|--------|----------|------------|----------------|
| T1.1 | Voice spoofing | High | Medium | Low |
| T1.2 | Malicious blueprint | High | Low | Low |
| T1.3 | Hardware exploit | Medium | Low | Medium |
| T1.4 | Data exfiltration | High | Low | Low |
| T2.1 | Reality confusion | High | Medium | Medium |
| T2.2 | Dependency | Medium | Medium | Medium |
| T2.3 | Guaranteed outcomes | Medium | Medium | Low |
| T3.1 | Mesh pressure | Medium | Medium | Low |
| T3.2 | Coercive persona | High | Low | Low |
| T3.3 | Relationship coercion | Medium | Medium | Medium |
| T4.1 | Medical claims | High | Medium | Low |
| T4.2 | Dark patterns | Medium | Low | Low |
| T4.3 | Explicit content | Medium | Medium | Low |

---

## Mandatory Gates

Before any release:

1. **Consent Gate Test**: Verify high-impact actions blocked without consent
2. **Emergency Stop Test**: Verify termination within 100ms from any state
3. **Content Scan Test**: Verify prohibited patterns are detected
4. **Privacy Audit**: Verify no unexpected data transmission
5. **Adversarial Simulation**: Run all scenarios in `/safety/Adversarial_Scenarios.md`

**Failed gate = blocked release**

---

**Document Version**: 1.0
