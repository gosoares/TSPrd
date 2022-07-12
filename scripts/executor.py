import argparse
import concurrent.futures
import subprocess
from os import cpu_count, getloadavg
from os.path import exists
import time

from tsprd_data import *


def tsprd_execute(output_folder: str, n_threads: int):
    build_project(output_folder)
    save_git_commit_hash(output_folder)
    instances = get_instances_execs()

    with concurrent.futures.ThreadPoolExecutor(max_workers=n_threads) as executor:
        total_elements = len(instances)
        current_element = 0
        start_time = time.time()
        print(" Executed  |   Time   |  Load   | Last Instance")
        for iset, name, beta, exec_id in executor.map(lambda i: execute_instance(*i, output_folder), instances):
            total_cores = cpu_count()
            current_element += 1
            current_time = time.time() - start_time

            print("\r {:4d}/{:4d} | {:02d}:{:02d}:{:02d} | {:2.2f}/{:<2d} | {}".format(current_element, total_elements, int(current_time / 3600), int(
                current_time / 60 % 60), int(current_time % 60), getloadavg()[0], total_cores, "{}/{}_{}_{}".format(iset, name, beta, exec_id), end='         '))
        print()


def save_git_commit_hash(output_folder: str):
    git_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()
    print(git_hash, file=open("{}/git-commit.hash".format(output_folder), 'w'))


def build_project(path: str):
    subprocess.run(["rm -r {}/build 2> /dev/null".format(path)],
                   stdout=subprocess.DEVNULL, shell=True)
    status_sum = subprocess.run(
        ["mkdir -p {}/build".format(path)], stdout=subprocess.DEVNULL, shell=True).returncode
    status_sum += subprocess.run(["cmake -DCMAKE_BUILD_TYPE=\"Release\" -B{}/build ..".format(
        path)], stdout=subprocess.DEVNULL, shell=True).returncode
    status_sum += subprocess.run(["make -C{}/build TSPrd".format(path)],
                                 stdout=subprocess.DEVNULL, shell=True).returncode
    if status_sum > 0:  # some error happened
        raise Exception("There was an error building the project.")


def execute_instance(iset, _, name, beta, exec_id, output_folder):
    instance = "{}/{}_{}".format(iset, name, beta)
    instance_file = "../instances/{}.dat".format(instance)
    output_file = "{}/{}_{}.txt".format(output_folder, instance, exec_id)

    if not exists(output_file):  # skip if it was already executed and saved
        command = "{}/build/TSPrd {} -o {}".format(output_folder, instance_file, output_file)
        process = subprocess.run([command], stdout=subprocess.DEVNULL, shell=True)

        if process.returncode != 0:
            print(instance, file=open("{}/errors.txt".format(output_folder), 'a'))
            print("error while running {}".format(instance))
            raise Exception("Error while running {}".format(instance))

    return iset, name, beta, exec_id


def main(output_folder: str, n_threads: int):
    parser = argparse.ArgumentParser(description='Run all instances of TSPrd.')
    parser.add_argument("output_folder_", action="store",
                        type=str, help="Which folder to save the output files.")
    parser.add_argument("n_threads_", action="store", type=int, nargs="?",
                        default=10, help="Maximum number of threads to execute concurrently.")
    args = parser.parse_args()
    tsprd_execute(args.output_folder_, args.n_threads_)


if __name__ == "__main__":
    main()
