# 15-Minute Demo Checklist (per common course rubric)

1) Project overview (1 min)
- What is PROJ-11, what problem it solves, high-level architecture.

2) Input format & data directory (2 min)
- Show `data/input_week.json`
- Explain sources, storage, loads, profiles, simulation config

3) Live run (5 min)
- Run with TUI: `./bin/microgrid data/input_week.json --tui`
- Point out dispatch priority: renewables -> battery -> diesel
- Point out SoC constraints (20%..95%)
- Show diesel usage minimized when renewables/battery are sufficient

4) Code walkthrough (5 min)
- Data structures: `LinkedList`, `LinkedQueue` (pointer-based)
- Parser: `JsonLite`
- Controller logic: forecasting reserve SoC + dispatch step

5) Testing & quality (2 min)
- Run tests: `make test && ./bin/tests`
- Mention valgrind checks

