import argparse
import concurrent.futures
import subprocess
from itertools import chain
from os.path import exists


def main(output_folder: str, n_threads: int):
    build_project()
    instances = get_instances_desc()

    with concurrent.futures.ThreadPoolExecutor(max_workers=n_threads) as executor:
        total_elements = len(instances)
        current_element = 0
        for r in executor.map(lambda i: execute_instance(*i, output_folder), instances):
            current_element += 1
            print("\rProcessing: {}/{}...       {}       ".format(current_element, total_elements, r), end='')
        print()


def build_project():
    subprocess.run(["mkdir -p ../cmake-build-release"], stdout=subprocess.DEVNULL, shell=True)
    subprocess.run(["cmake -DCMAKE_BUILD_TYPE=\"Release\" -B../cmake-build-release .."], stdout=subprocess.DEVNULL, shell=True)
    subprocess.run(["make -C../cmake-build-release TSPrd"], stdout=subprocess.DEVNULL, shell=True)


def execute_instance(iset, name, beta, exec_id, output_folder):  # (set, name, beta, exec_id)
    instance = "{}/{}_{}".format(iset, name, beta)
    output_file = "{}/{}_{}.txt".format(output_folder, instance, exec_id)
    if not exists(output_file):
        process = subprocess.run(["../bin/TSPrd {} {}".format(instance, output_file)], stdout=subprocess.DEVNULL, shell=True)
        if process.returncode != 0:
            print(instance, file=open("{}/errors.txt".format(output_folder), 'a'))
            print("error while running {}.".format(instance))
    return "{} {}".format(instance, exec_id)


def get_instances_desc():
    # read (set, name, beta, exec_id) for all instances
    solomon_instances = [["Solomon", "{}/{}".format(n, name)] for n in [10, 15, 20, 25, 30, 50, 100] for name in ["C101", "C201", "R101", "RC101"]]
    tsplib_names = ["eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100", "kroE100", "rd100", "eil101",
                    "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150", "kroA150", "kroB150", "pr152", "u159", "rat195",
                    "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226", "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439",
                    "pcb442", "d493"]
    tsplib_instances = [["TSPLIB", name] for name in tsplib_names]
    atsplib_instances = [["aTSPLIB", name] for name in ["ftv33", "ft53", "ftv70", "kro124p", "rbg403"]]
    instances = chain(solomon_instances, atsplib_instances, tsplib_instances)
    instances = [[*x, beta, exec_id] for x in instances for beta in ["0.5", "1", "1.5", "2", "2.5", "3"] for exec_id in range(1, 11)]
    return list(instances)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run all instances of TSPrd.')
    parser.add_argument("output_folder_", action="store", type=str, help="Which folder to save the output files.")
    parser.add_argument("n_threads_", action="store", type=int, nargs="?", default=10, help="Maximum number of threads to use concurrently.")
    args = parser.parse_args()

    main(args.output_folder_, args.n_threads_)
