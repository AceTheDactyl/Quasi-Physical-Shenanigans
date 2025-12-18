# WishBed App — Technical Design Document (TDD)

## 1. Product Definition
WishBed App is a software controller for the WishBed System (bed-bound) and Walking WishBed (mobile-safe) symbolic experiences.

### Non-goals
- Medical treatment/diagnosis, guaranteed outcomes, or replacement for professional support.

## 2. Architecture
- UI Layer
- App Engine (session state machine)
- Safety Layer (consent gates, caps, emergency stop)
- Renderer Adapter (Preview2D / 3D Engine / AR Backend)
- Voice Interface (optional)
- Storage (LocalVault, export/import)
- Mesh (optional) + Marketplace (optional, governed)

## 3. Core Invariants
1) User agency and reversibility
2) Explicit consent before activation
3) Emergency stop always available and prioritized
4) Symbolic framing; avoid guaranteed-outcome claims
5) Local-first and privacy-minimizing by default

## 4. UX Flow
Onboarding → Education → Dashboard → Session Start → Active → Integration → Exit

## 5. Safety & Compliance
- Threat model and adversarial simulations
- Policy scanning for marketplace and persona content
- No dark patterns; transparent pricing; easy cancellation

## 6. MVP vs Full Build
See `mvp/MVP_vs_Full.md`.

## 7. Engineering Handoff
See `handoff/Engineer_Handoff.md`.
