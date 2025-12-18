# WishBed NLP Design Handoff

Engineer-ready technical design handoff derived from the WishBed Spread NLP specification.

This archive translates symbolic system language into:
- Line-numbered requirements
- Pseudocode scaffolding
- Modular architecture
- Safety and compliance constraints

## Folder Structure

```
/docs        → Line-numbered NLP requirements
/pseudocode  → High-level logic for core systems
/architecture→ Non-negotiable invariants and system design
/ux          → User flow definitions
/safety      → Threat modeling & misuse prevention
/mvp         → MVP vs Full feature separation
/handoff     → Engineering translation notes
```

## How to Use This Archive

1. **Start here** - Read this README completely
2. **Read the handoff** - `/handoff/Engineer_Handoff.md` contains the engineering translation guide
3. **Review requirements** - `/docs/NLP_Requirements.md` has line-numbered traceable requirements
4. **Study architecture** - `/architecture/` contains invariants and system design
5. **Implement from pseudocode** - `/pseudocode/` provides scaffolding for real implementation
6. **Validate safety** - Every module must pass through `/safety/` constraints

## Engineering Rules (Non-Negotiable)

1. **No module may bypass the Safety Layer**
2. **Consent, reversibility, and emergency stop are mandatory**
3. **Symbolic language must never become literal claims**
4. **All high-impact actions require confirmation**
5. **Privacy-first: local storage, minimal data collection**

## System Overview

**WishBed App** is a software controller for symbolic experiential systems:

| Component | Description |
|-----------|-------------|
| WishBed System | Bed-bound symbolic experience layer |
| Walking WishBed | Mobile-safe symbolic experience layer (reduced intensity) |
| UCF Hardware | ESP32-based hardware substrate (19-sensor hex grid) |

### Architecture Stack

```
┌─────────────────────────────────────────┐
│          UI Layer                       │
│  (Dashboard, Session, Settings, Edu)    │
└────────────┬────────────────────────────┘
             │
┌────────────v────────────────────────────┐
│       App Engine (State Machine)        │
│  States: Idle → Initializing → Active   │
│          → Integrating → Shutdown       │
└────────────┬────────────────────────────┘
             │
┌────────────v────────────────────────────┐
│      Safety Layer (MANDATORY)           │
│  - Consent Gates                        │
│  - Emergency Stop (always prioritized)  │
│  - SafetyCaps (brightness, motion)      │
│  - Reality Anchors (non-medical)        │
└────────────┬────────────────────────────┘
             │
         ┌───┴───┬──────────┬────────────┐
         │       │          │            │
    ┌────v──┐ ┌──v──┐ ┌────v──┐ ┌──────v──┐
    │Renderer│ │Voice│ │Storage│ │Optional:│
    │Adapter │ │I/O  │ │Vault  │ │Mesh/MP  │
    └────────┘ └─────┘ └───────┘ └─────────┘
```

## Requirement Traceability

All requirements use the format `[XXXX]` where XXXX is a 4-digit requirement ID.

| Range | Domain |
|-------|--------|
| [0001-0006] | Definitions & Invariants |
| [0007-0010] | Session Lifecycle |
| [0011-0015] | Safety & Reality Anchors |
| [0016-0020] | Voice Interface |
| [0021-0026] | Mesh Sessions |
| [0027-0031] | Marketplace & Governance |
| [0032-0035] | Renderer & AR Mapping |
| [0036-0038] | Storage & Portability |
| [0039-0042] | Threat Modeling & Compliance |
| [0043-0045] | MVP vs Full Build |

## Quick Reference: Core Invariants

```
[INV-1] User agency always: All experiences are reversible and user-controlled
[INV-2] Consent before activation: Every high-impact action requires confirmation
[INV-3] Emergency stop priority: Always visible, always functional, highest priority
[INV-4] Symbolic framing only: No literal claims, no guaranteed outcomes
[INV-5] Privacy-first: Local-first storage, minimal data collection
```

## Recommended Audience

- Software engineers building mobile/web implementations
- Tech leads preparing MVP development plans
- QA engineers validating safety compliance
- Security reviewers assessing threat models

## Related Resources

- `../WishBed_App_TDD_v2/` - Full Technical Design Document
- `../WishBed_App_TDD_v2/contracts/` - TypeScript interfaces and schemas
- `../unified-consciousness-hardware/` - UCF ESP32 firmware reference

---

**This version is suitable for actual implementation planning.**
