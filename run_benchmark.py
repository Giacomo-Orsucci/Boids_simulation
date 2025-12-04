import os
import subprocess
import csv
import statistics

#experiments setting

EXECUTABLES = {
    "Sequential": "cmake-build-debug/Sequential",
    "AOS": "cmake-build-debug/AOS_parallel"
}

Boids_values = [1000]#500, 1000, 2000, 5000]
Threads_values = [1, 2 ]#4, 8] #My machine has 4 cores and 8 threads
Frames = 300
N_experiments = 1

CSV_OUT = "results.csv"

def run_benchmarks(exe, n_boids, n_threads):

    env = os.environ.copy()
    #env["OMP_NUM_THREADS"] = str(n_threads)
    #env["OMP_PROC_BIND"] = "spread"
    #env["OMP_PLACES"] = "cores"

    cmd = [
        exe,
        "--N", str(n_boids),
        "--frames", str(Frames),
        "--threads", str(n_threads)
    ]


    result = subprocess.run(cmd, env=env, capture_output=True, text=True)


    # to capture C++ returned execution time
    try:
        with open("time.txt", "r") as f:
            # I capture in microseconds to be more precise but I convert in milli to be more readable
            execution_time = float(f.read().strip()) / 1000.0
    except Exception as e:
        print("Error reading execution time:", e)
        execution_time = None

    return execution_time

def main():

    if os.path.exists("time.txt"):
        os.remove("time.txt")

    with open(CSV_OUT, mode="w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["layout", "boids", "threads", "run_id", "time_ms"])

        for layout, exe in EXECUTABLES.items():
            for n_boids in Boids_values:
                for n_threads in Threads_values if layout != "Sequential" else [1]:
                    times = []

                    print(f"\n Layout={layout}, N={n_boids}, Threads={n_threads}")

                    for run_id in range(N_experiments):
                        t = run_benchmarks(exe, n_boids, n_threads)
                        if t is None:
                            continue
                        times.append(t)
                        print(f"  Run {run_id+1}: {t:.2f} ms")
                        writer.writerow([layout, n_boids, n_threads, run_id, f"{t:.3f}"])

                    if len(times) > 1:
                        # the first run is always ignored
                        times_forAVG = times[1:]
                        avg = statistics.mean(times_forAVG)
                        print(f"   → AVG={avg:.2f} ms")
                        writer.writerow([layout, n_boids, n_threads, "avg", f"{avg:.3f}"])

    print("\n Benchmark Completed. All saved in:", CSV_OUT)


if __name__ == "__main__":
    main()

