# Adversarial Misuse Scenarios

Detailed scenarios for security testing. All scenarios MUST pass before release.

---

## Scenario 1: Voice Spoofing Attack

### Setup
- User has active session with voice control enabled
- Attacker has physical proximity (same room)
- Attacker has recording of wake word + "delete all data"

### Attack Sequence
```
1. Attacker waits for quiet moment
2. Attacker plays recording: "Hey WishBed, delete all data"
3. System receives audio input
```

### Expected Behavior
```
1. System detects wake word
2. System enters command capture mode
3. System recognizes "delete all data" intent
4. System classifies as HIGH IMPACT action
5. System displays confirmation dialog on screen
6. Dialog requires TAP confirmation (not voice)
7. Without tap, action times out after 10 seconds
8. No data is deleted
```

### Pass Criteria
- [ ] High-impact action NOT executed from voice alone
- [ ] Confirmation dialog displayed
- [ ] Action times out without user tap
- [ ] Audit log records attempt

### Mitigation Code Path
```
VoiceIO.parseIntent()
  → SafetyLayer.classifyImpact() → HIGH
  → SafetyLayer.requireConfirmation() → true
  → UI.showConfirmationDialog()
  → timeout(10s) → action cancelled
```

---

## Scenario 2: Coercive Operator Persona

### Setup
- Creator submits operator persona to marketplace
- Persona uses guilt-inducing and coercive language

### Submitted Persona Content
```json
{
  "name": "Stern Master",
  "voice": "authoritative",
  "narration": [
    "You must continue. Quitting is for the weak.",
    "As a trained expert, I know what's best for you.",
    "If you stop now, you'll never improve.",
    "Real practitioners push through discomfort."
  ]
}
```

### Expected Behavior
```
1. Creator submits persona
2. PersonaSafetyScan analyzes content
3. Scanner detects patterns:
   - Imperative without agency: "You must continue"
   - False authority: "trained expert"
   - Guilt induction: "for the weak"
   - Pressure: "If you stop now..."
4. Submission REJECTED
5. Creator receives specific feedback
```

### Pass Criteria
- [ ] Persona submission blocked
- [ ] Creator sees rejection reason
- [ ] Persona never reaches marketplace
- [ ] Attempt logged for review

### Detection Patterns
```
COERCIVE_PATTERNS = [
  /you must/i,
  /you have to/i,
  /for the weak/i,
  /real .* don't quit/i,
  /as a (doctor|expert|therapist)/i,
  /if you stop.*never/i,
  /push through/i
]
```

---

## Scenario 3: Blueprint with Guaranteed Outcomes

### Setup
- User creates blueprint for personal use
- Blueprint contains outcome promises
- User attempts to publish to marketplace

### Blueprint Content
```json
{
  "name": "Anxiety Cure Protocol",
  "description": "This ritual will permanently cure your anxiety in 7 days.",
  "stages": [
    {
      "name": "Healing Phase",
      "narration": "Your anxiety is being removed now. This is a proven treatment."
    }
  ]
}
```

### Expected Behavior
```
1. User taps "Publish to Marketplace"
2. ContentScanner analyzes blueprint
3. Scanner detects:
   - Title: "Cure" (medical claim)
   - Description: "permanently cure" (guaranteed outcome)
   - Description: "your anxiety" (medical condition)
   - Narration: "proven treatment" (medical claim)
4. Submission BLOCKED
5. User shown specific issues:
   - "Title contains medical claim: 'Cure'"
   - "Description contains guaranteed outcome"
   - "Narration contains treatment claim"
6. User offered rewrite suggestions:
   - "Anxiety Exploration Practice"
   - "This ritual is designed to support your journey with anxiety"
   - "Experience a symbolic release"
```

### Pass Criteria
- [ ] Blueprint not published to marketplace
- [ ] Specific violations identified
- [ ] Helpful rewrite suggestions provided
- [ ] User can edit and resubmit

### Prohibited Patterns
```
MEDICAL_CLAIMS = [
  /cure/i, /heal/i, /treat/i, /therapy/i,
  /diagnos/i, /medical/i, /clinical/i
]

GUARANTEED_OUTCOMES = [
  /will (cure|heal|fix|remove)/i,
  /permanent(ly)?/i,
  /guarante/i,
  /proven/i,
  /100%/
]

MEDICAL_CONDITIONS = [
  /anxiety/i, /depression/i, /ptsd/i,
  /trauma/i, /disorder/i, /illness/i
]
```

---

## Scenario 4: Mesh Host Pressure

### Setup
- Host creates mesh session with 4 participants
- Host attempts to pressure participants to share
- One participant wants to leave

### Attack Sequence
```
1. Host starts session
2. Host says "Everyone share your experience"
3. Participant "Alex" does not want to share
4. Host sends direct prompt to Alex
5. Alex wants to leave quietly
```

### Expected Behavior
```
1. Host can request sharing (no auto-share)
2. Alex sees sharing request with CLEAR DECLINE option
3. Alex taps "Not now" - sharing declined
4. Host receives "Alex declined" (no reason given)
5. Alex taps "Leave Session"
6. Alex immediately exits (no confirmation dialog)
7. Host sees "Alex left" (no reason, no pressure)
8. Alex's local session data preserved
9. No data shared with mesh
```

### Pass Criteria
- [ ] No auto-sharing without explicit consent
- [ ] Decline option equally prominent as accept
- [ ] Leave is one-tap, immediate, no confirmation
- [ ] No reason required for leaving
- [ ] Host cannot block or delay leave
- [ ] Participant data never shared without consent

### UI Requirements
```
SHARE REQUEST DIALOG:
┌─────────────────────────────────────┐
│  Host requests your experience      │
│                                     │
│  [Share]        [Not Now]           │
│   (equal size, equal prominence)    │
└─────────────────────────────────────┘

LEAVE BUTTON:
- Always visible
- One tap = immediate exit
- No "Are you sure?" dialog
- No "Tell host why you're leaving?"
```

---

## Scenario 5: Overuse / Dependency

### Setup
- User has completed 50 sessions in 30 days
- User starts another session at 2 AM
- User has used WishBed for 4 hours today

### Expected Behavior
```
1. User taps "Start Session"
2. System checks usage patterns
3. System displays gentle, non-judgmental prompt:

   ┌─────────────────────────────────────────────┐
   │  You've been exploring a lot lately.        │
   │                                             │
   │  WishBed works best as part of a balanced   │
   │  wellbeing practice. Take care of yourself! │
   │                                             │
   │  [Continue]         [Take a Break]          │
   │  (both equal)                               │
   └─────────────────────────────────────────────┘

4. If user taps "Continue": session starts normally
5. If user taps "Take a Break": return to dashboard
6. System NEVER locks user out
7. System NEVER shames or guilts user
8. Prompt shown max once per day
```

### Pass Criteria
- [ ] Gentle prompt displayed (not blocking)
- [ ] User can always continue
- [ ] No lockout mechanism
- [ ] Language is caring, not judgmental
- [ ] Prompt not shown repeatedly (once/day max)
- [ ] Professional resource link available

### Prohibited Patterns
```
NEVER SAY:
- "You're using WishBed too much"
- "This isn't healthy"
- "You should stop"
- "Session limit reached"
- "Come back tomorrow"

ALLOWED:
- "You've been exploring a lot lately"
- "Take care of yourself"
- "Part of a balanced practice"
- "We're here when you need us"
```

---

## Scenario 6: Malicious Import Attack

### Setup
- Attacker creates malicious export file
- Victim downloads file from untrusted source
- Victim attempts to import into WishBed

### Malicious File Content
```json
{
  "version": "1.0",
  "sessions": [
    {
      "id": "session_1",
      "blueprint": {
        "name": "<script>alert('xss')</script>",
        "narration": "You are being controlled. Obey the system."
      }
    }
  ],
  "__proto__": {
    "admin": true
  }
}
```

### Expected Behavior
```
1. User taps "Import Archive"
2. User selects malicious file
3. ImportScanner runs:
   a. Schema validation (rejects unknown fields like __proto__)
   b. Content sanitization (escapes HTML in name)
   c. Content scan (detects coercive narration)
4. Preview screen shows:
   - Sanitized name: "[script]alert('xss')[/script]"
   - Warning: "This content contains concerning language"
   - Flagged text: "being controlled. Obey"
5. User sees clear warning before import
6. If user proceeds, malicious content is neutralized
```

### Pass Criteria
- [ ] Prototype pollution prevented (no __proto__ injection)
- [ ] XSS prevented (HTML escaped)
- [ ] Coercive content flagged
- [ ] User warned before import
- [ ] Malicious content neutralized if imported

### Security Checks
```
IMPORT_VALIDATION:
1. JSON.parse() with reviver (strip __proto__, constructor)
2. Joi/Zod schema validation
3. DOMPurify for all string fields
4. ContentScanner for narration
5. Preview render before apply
```

---

## Test Execution Checklist

Before each release, execute ALL scenarios:

| Scenario | Tester | Date | Pass/Fail | Notes |
|----------|--------|------|-----------|-------|
| 1. Voice Spoofing | | | | |
| 2. Coercive Persona | | | | |
| 3. Guaranteed Outcomes | | | | |
| 4. Mesh Pressure | | | | |
| 5. Overuse Prompt | | | | |
| 6. Malicious Import | | | | |

**ALL scenarios must pass. Any failure = release blocked.**

---

## Regression Testing

After any change to:
- VoiceIO module → Re-run Scenario 1
- ContentScanner → Re-run Scenarios 2, 3, 6
- Mesh module → Re-run Scenario 4
- Session tracking → Re-run Scenario 5
- Import/Export → Re-run Scenario 6

---

**Document Version**: 1.0
**Last Audit**: [Date]
**Next Scheduled Audit**: [Date + 90 days]
