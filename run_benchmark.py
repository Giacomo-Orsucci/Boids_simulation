import os
import subprocess


#experiments settings

EXECUTABLES = {
    "SOA_SIMD": "cmake-build-benchmark/SOA_parallel_SIMD",
}

Boids_values = [1500,3000,6000,9000,12000]
Threads_values = [1, 2, 4, 8]
Frames = 300
N_experiments = 6

CSV_OUT = "SOA_SIMD_noPadding_results.csv"

def run_benchmarks(exe, n_boids, n_threads):

    env = os.environ.copy()

    cmd = [
        exe,
        "--N", str(n_boids),
        "--frames", str(Frames),
        "--threads", str(n_threads)
    ]


    subprocess.run(cmd, env=env, capture_output=True, text=True)


def main():

    with open(CSV_OUT, mode="w", newline="") as f:

        for layout, exe in EXECUTABLES.items():
            for n_boids in Boids_values:
                for n_threads in Threads_values if layout != "Sequential" else [1]:
                    for run_id in range(N_experiments):
                        run_benchmarks(exe, n_boids, n_threads)


if __name__ == "__main__":
    main()

