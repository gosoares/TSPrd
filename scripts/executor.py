import argparse
import concurrent.futures
import subprocess
from os.path import exists

from tsprd_data import *


def main(output_folder: str, n_threads: int):
    build_project()
    instances = get_instances_execs()

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


def execute_instance(iset, _, name, beta, exec_id, output_folder):
    instance = "{}/{}_{}".format(iset, name, beta)
    output_file = "{}/{}_{}.txt".format(output_folder, instance, exec_id)
    if not exists(output_file):
        process = subprocess.run(["../bin/TSPrd {} {}".format(instance, output_file)], stdout=subprocess.DEVNULL, shell=True)
        if process.returncode != 0:
            print(instance, file=open("{}/errors.txt".format(output_folder), 'a'))
            print("error while running {}.".format(instance))
    return "{} {}".format(instance, exec_id)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run all instances of TSPrd.')
    parser.add_argument("output_folder_", action="store", type=str, help="Which folder to save the output files.")
    parser.add_argument("n_threads_", action="store", type=int, nargs="?", default=10, help="Maximum number of threads to use concurrently.")
    args = parser.parse_args()

    main(args.output_folder_, args.n_threads_)
