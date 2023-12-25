import argparse
import concurrent.futures
import subprocess
import time
from dataclasses import dataclass
from os import cpu_count, getloadavg, makedirs
from os.path import exists
from typing import Self

from tsprd_data import Instance, get_instances_execs

TIME_LIMIT = int(10 * 60 * (2021.0 / 1618.0) + 0.5)


@dataclass
class ExecutionOptions:
    output_folder: str
    intra_moves: list[str] | None = None
    inter_moves: list[str] | None = None


@dataclass
class ExecutionItem:
    instance_name: str
    instance_file: str
    exec_id: int
    options: ExecutionOptions

    def __post_init__(self):
        self.output_file = "{}/{}_{}.txt".format(
            self.options.output_folder, self.instance_name, self.exec_id
        )

        run_command = "./build/TSPrd {} -o {} -t {}".format(
            self.instance_file, self.output_file, TIME_LIMIT
        )
        if self.options.intra_moves is not None:
            if self.options.intra_moves:
                run_command += " --intraMoves {}".format(
                    " ".join(self.options.intra_moves)
                )
            else:  # if empty
                run_command += " --intraMoves none"

        if self.options.inter_moves is not None:
            if self.options.inter_moves:
                run_command += " --interMoves {}".format(
                    " ".join(self.options.inter_moves)
                )
            else:  # if empty
                run_command += " --interMoves none"
        self.run_command = run_command

    @classmethod
    def from_instance(
        cls,
        instance: Instance,
        options: ExecutionOptions,
    ) -> Self:
        return cls(
            instance_name=instance.full_name,
            instance_file="../instances/{}.dat".format(instance.full_name),
            exec_id=instance.exec_id,
            options=options,
        )


def execute_all_from(options: list[ExecutionOptions], n_threads: int):
    execs = setup_executions(options)
    execute_all(execs, n_threads)


def setup_executions(
    options: list[ExecutionOptions],
) -> list[ExecutionItem]:
    build_project()
    return [o for opt in options for o in setup_execution(opt, build=False)]


def setup_execution(
    options: ExecutionOptions, build: bool = True
) -> list[ExecutionItem]:
    if build:
        build_project()
    makedirs(options.output_folder, exist_ok=True)
    save_git_commit_hash(options.output_folder)
    with open("{}/options.txt".format(options.output_folder), "w") as f:
        print(options.__dict__, file=f)

    return [ExecutionItem.from_instance(i, options) for i in get_instances_execs()]


def execute_all(execs: list[ExecutionItem], n_threads: int):
    with concurrent.futures.ThreadPoolExecutor(max_workers=n_threads) as executor:
        total_elements = len(execs)
        current_element = 0
        start_time = time.time()
        print(" Executed  |   Time   |  Load   | Last Instance")
        for exec in executor.map(execute_instance, execs):
            total_cores = cpu_count()
            current_element += 1
            current_time = time.time() - start_time

            print(
                "\r {:4d}/{:4d} | {:02d}:{:02d}:{:02d} | {:2.2f}/{:<2d} | {}".format(
                    current_element,
                    total_elements,
                    int(current_time / 3600),
                    int(current_time / 60 % 60),
                    int(current_time % 60),
                    getloadavg()[0],
                    total_cores,
                    exec.instance_name,
                ),
                end="   ",
                flush=True,
            )


def save_git_commit_hash(output_folder: str):
    git_hash = (
        subprocess.check_output(["git", "rev-parse", "HEAD"]).decode("ascii").strip()
    )
    print(git_hash, file=open("{}/git-commit.hash".format(output_folder), "w"))


def build_project():
    subprocess.run(
        ["rm -rf ./build 2> /dev/null"],
        stdout=subprocess.DEVNULL,
        shell=True,
    )
    status_sum = subprocess.run(
        ["mkdir -p ./build"], stdout=subprocess.DEVNULL, shell=True
    ).returncode
    status_sum += subprocess.run(
        ['cmake -DCMAKE_BUILD_TYPE="Release" -B ./build ..'],
        stdout=subprocess.DEVNULL,
        shell=True,
    ).returncode
    status_sum += subprocess.run(
        ["make -C ./build TSPrd"], stdout=subprocess.DEVNULL, shell=True
    ).returncode
    if status_sum > 0:  # some error happened
        raise Exception("There was an error building the project.")


def execute_instance(exec: ExecutionItem):
    makedirs(exec.output_file.rsplit("/", 1)[0], exist_ok=True)

    if not exists(exec.output_file):  # skip if it was already executed and saved
        process = subprocess.run(
            [exec.run_command], stdout=subprocess.DEVNULL, shell=True
        )

        if process.returncode != 0:
            print(
                exec.instance_name,
                file=open("{}/errors.txt".format(exec.options.output_folder), "a"),
            )
            raise Exception("Error while running {}".format(exec.instance_name))

    return exec


def main():
    parser = argparse.ArgumentParser(description="Run all instances of TSPrd.")
    parser.add_argument(
        "output_folder",
        action="store",
        type=str,
        help="Which folder to save the output files.",
    )
    parser.add_argument(
        "n_threads",
        action="store",
        type=int,
        nargs="?",
        default=10,
        help="Maximum number of threads to execute concurrently.",
    )
    args = parser.parse_args()
    execute_all_from([ExecutionOptions(args.output_folder)], args.n_threads)


if __name__ == "__main__":
    main()
