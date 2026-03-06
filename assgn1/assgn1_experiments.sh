#!/bin/bash
#SBATCH -A ASC23013
#SBATCH -J assgn1_pi
#SBATCH -o assgn1_pi.%j.out
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 128
#SBATCH -p development
#SBATCH -t 00:10:00

cd $SLURM_SUBMIT_DIR

gcc -O3 -fopenmp compute_pi.c -o compute_pi

N=10000000
MAX=$SLURM_CPUS_PER_TASK

echo "Node: $(hostname)"
echo "CPUs allocated: $MAX"
echo ""

# Q2(a): thread placement
export OMP_NUM_THREADS=$MAX
echo "=== Q2(a): Thread placement (MAX threads) ==="
for PL in cores sockets; do
  for BIND in close spread; do
    export OMP_PLACES=$PL
    export OMP_PROC_BIND=$BIND
    echo "PLACES=$OMP_PLACES PROC_BIND=$OMP_PROC_BIND THREADS=$OMP_NUM_THREADS"
    ./compute_pi $N
    echo ""
  done
done

# Q2(c-i): scaling
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_SCHEDULE=static
echo "=== Q2(c-i): Scaling (cores/close) ==="
for T in 1 2 4 8 16 32 64 128; do
  export OMP_NUM_THREADS=$T
  echo "Threads=$T"
  ./compute_pi $N
  echo ""
done

# Q2(c-ii): scheduling
export OMP_NUM_THREADS=$MAX
export OMP_PLACES=cores
export OMP_PROC_BIND=close
echo "=== Q2(c-ii): Scheduling (MAX threads, cores/close) ==="
for S in static dynamic; do
  for C in 10 100 1000; do
    export OMP_SCHEDULE="${S},${C}"
    echo "OMP_SCHEDULE=$OMP_SCHEDULE"
    ./compute_pi $N
    echo ""
  done
done
