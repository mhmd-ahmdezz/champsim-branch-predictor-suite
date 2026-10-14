# ChampSim Branch Predictor Suite
This project implements and evaluates 11 different branch prediction algorithms using ChampSim, a trace-based microarchitectural simulator. It compares traditional predictors against advanced designs like Perceptron and TAGE across different benchmarks.

## Implemented Predictors

The repository contains implementations of the following branch prediction schemes:
* Always Taken / Always Not Taken
* BTFN (Backward Taken, Forward Not Taken)
* Last-Time Predictor (it is a counter-based but without hysteresis)
* Bimodal Predictor
* Three-bit counter-based Predictor (the same as bimodal but with 3-bit for hysteresis)
* Two-Level Global History Predictor
* Gshare Predictor
* Two-Level Local History Predictor
* Tournament Predictor (Hybrid predictor: Local+global as in Alpha 21264)
* Perceptron Predictor
* TAGE (Tagged Geometric History Length) Predictor

## Evaluation Results

The table below summarizes the performance of each predictor tested on the perlbench_s and gcc_s workloads. Metrics include Branch Prediction Accuracy, Mispredictions Per Kilo-Instructions (MPKI), and Average Reorder Buffer (ROB) Occupancy at Mispredict.

| Predictor | Workload | Accuracy (%) | MPKI | Avg ROB Occupancy at Mispredict |
| :--- | :--- | :--- | :--- | :--- |
| Always Not Taken | perlbench_s-210B | 69.61% | 49.77 | 7.719 |
| Always Taken | perlbench_s-210B | 57.37% | 69.83 | 8.452 |
| BTFN | perlbench_s-210B | 72.76% | 44.62 | 7.828 |
| Last-Time | perlbench_s-210B | 96.98% | 4.941 | 85.64 |
| Bimodal | perlbench_s-210B | 97.26% | 4.489 | 118.7 |
| Two-Level Global | perlbench_s-210B | 97.66% | 3.840 | 127.8 |
| Gshare | perlbench_s-210B | 97.69% | 3.787 | 128.2 |
| Two-Level Local | perlbench_s-210B | 98.08% | 3.145 | 107.6 |
| Tournament | perlbench_s-210B | 98.26% | 2.843 | 120.4 |
| Perceptron | perlbench_s-210B | 97.98% | 3.315 | 147.6 |
| TAGE | perlbench_s-210B | 98.31% | 2.769 | 127.1 |
| Tournament | gcc_s-2226B | 99.57% | 1.521 | 229.3 |
| TAGE | gcc_s-2226B | 99.56% | 1.556 | 215.8 |

