# PROJ-11: Renewable Energy Microgrid Balancing Simulator (CSE 211)

This project implements a discrete-time microgrid simulator that balances renewable generation (solar/wind),
battery storage, and diesel backup to continuously serve loads while minimizing diesel usage and avoiding
aggressive battery cycling.

## Constraints (Important)
- **NO STL containers**: do not use `std::vector`, `std::map`, `std::list`, etc.
- Uses custom, pointer-based data structures under `include/data_structures/`:
  - `LinkedList<T>` (stores profiles, sources, loads)
  - `LinkedQueue<T>` (second custom DS for rubric compliance + unit tests)
- Reads inputs from the `data/` folder using **relative paths**.
- Provides a **TUI frontend** that visualizes per-step energy flows.
- Includes **unit + edge + integration** tests and a simple test runner.
- Includes UML diagrams (PlantUML text) in `docs/uml/`.
- Build system: **Makefile** with `make`, `make test`, `make clean`.

## Build
```bash
make
```

## Run
```bash
./bin/microgrid data/input_sample.json
./bin/microgrid data/input_sample.json --tui
./bin/microgrid data/input_week.json --tui
```

## Test
```bash
make test
./bin/tests
```

## Valgrind (optional but recommended)
```bash
valgrind --leak-check=full --show-leak-kinds=all ./bin/tests
valgrind --leak-check=full --show-leak-kinds=all ./bin/microgrid data/input_sample.json
```
