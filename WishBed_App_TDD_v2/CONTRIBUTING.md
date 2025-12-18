# Contributing

## How to contribute
1. Open an issue using the templates in `.github/ISSUE_TEMPLATE/`
2. Propose changes with a PR using the PR template
3. Ensure changes preserve **invariants**:
   - Consent before activation
   - Reversibility
   - Emergency stop always available
   - Symbolic framing (no guaranteed outcome claims)
   - Privacy-first (local-first by default)

## Docs-first workflow
This repo is spec-led. Update docs/contracts before (or alongside) code changes.

## Safety review
Any change that touches:
- Voice commands
- Mesh sessions
- Marketplace listings
- Operator persona scripts
must include an update to `safety/` and test scenarios.
