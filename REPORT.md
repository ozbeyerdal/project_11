# Design & Logic (PROJ-11)

## Model
At each discrete time step Δt:
- Compute total renewable generation (solar + wind)
- Compute total load demand
- Enforce energy balance with dispatch:
  1) Renewables serve the load first
  2) Surplus renewables charge the battery (limited by charge rate and SoC max 95%)
  3) Remaining deficit is served by battery discharge (limited by discharge rate and a forecast-based reserve SoC; never below 20%)
  4) Diesel generators serve any remaining deficit (limited by diesel capacity)
  5) Any remaining deficit is recorded as unmet load (should be 0 if diesel capacity is enough)

## Forecasting (Simple, but explicit)
To avoid discharging the battery too aggressively, a short look-ahead forecast is computed:
- For the next 8 steps, estimate net deficit = load_forecast - renew_forecast
- Convert predicted deficit energy to a *reserve SoC* in [min_soc, 0.8]
- When discharging, the controller keeps SoC >= reserve SoC

This implements the "look ahead" idea mentioned in the assignment without requiring heavy optimization.

## Battery degradation proxy
We maintain a simple score:
- Penalty when SoC goes below 30%
- Penalty for large single-step discharge drops (>10% SoC)
This discourages deep and sudden discharges.

## Data structures
`LinkedList<T>` is used instead of STL containers to store variable-sized collections.
It is pointer-based (node-linked) to satisfy the rubric's "no array-based DS" rule.
(sources, loads, profiles).

## Frontend
A terminal UI prints each step and ASCII bars to visualize:
- Load
- Renewables
- Diesel usage

