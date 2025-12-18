# Pseudocode Scaffolding

High-level logic for core WishBed systems. Use these as implementation guides.

---

## Files

| File | Purpose | Priority |
|------|---------|----------|
| `AppEngine.pseudo` | Session state machine | MVP |
| `SafetyLayer.pseudo` | Safety gates and caps | MVP |
| `OperatorSystem.pseudo` | Voice intent handling | Phase 2 |
| `hardware/hardware-adapter.pseudo` | UCF hardware bridge | MVP |

---

## Usage

1. Read pseudocode to understand module responsibilities
2. Translate to TypeScript/target language
3. Maintain requirement traceability (cite [XXXX] in comments)
4. Ensure all safety gates are preserved

---

## Conventions

```
MODULE Name           -- Module/class definition
  STATE A | B | C     -- Enum/union type
  PRIVATE var         -- Private member
  PUBLIC var          -- Public member

  FUNCTION name():    -- Method definition
    REQUIRE condition -- Precondition (throw if false)
    IF ... THEN       -- Conditional
    AWAIT operation   -- Async operation
    RETURN value      -- Return statement
  END FUNCTION

END MODULE
```

---

## Related

- `/architecture/System_Architecture.md` - Where these modules fit
- `/docs/NLP_Requirements.md` - Requirement IDs to cite
- `/handoff/Engineer_Handoff.md` - Translation guide
