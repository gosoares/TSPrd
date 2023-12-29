import argparse
from pathlib import Path
from shutil import copy

from executor import ExecutionOptions, execute_all_from
from tables_gen import gen_tables


def main():
    parser = argparse.ArgumentParser(
        description="Script for running all instances of TSPrd with incremental moves."
    )
    subparsers = parser.add_subparsers(
        dest="command", help="Subcommands: generate, execute", required=True
    )

    parser_execute = subparsers.add_parser(
        "execute", help="Execute all instances with incremental moves."
    )
    parser_execute.add_argument(
        "threads",
        action="store",
        type=int,
        help="Maximum number of threads to execute concurrently.",
    )

    subparsers.add_parser("generate", help="Generate tables from the results.")

    args = parser.parse_args()
    if args.command == "execute":
        execute(args.threads)
    elif args.command == "generate":
        generate_tables()


def execute(threads: int):
    all_intra_moves = [
        "reinsertion1",
        "reinsertion2",
        "swap11",
        "swap12",
        "swap22",
        "2opt",
    ]
    all_inter_moves = ["relocation", "swap", "divideAndSwap"]

    all_options = []
    for i in range(1, len(all_intra_moves) + 1):
        intra_moves = all_intra_moves[:i]
        options = ExecutionOptions(
            output_folder="incremental_moves/output_{}_0".format(i),
            intra_moves=intra_moves,
            inter_moves=[],
        )
        all_options.append(options)

    for i in range(1, len(all_inter_moves) + 1):
        inter_moves = all_inter_moves[:i]
        options = ExecutionOptions(
            output_folder="incremental_moves/output_{}_{}".format(
                len(all_intra_moves), i
            ),
            intra_moves=all_intra_moves,
            inter_moves=inter_moves,
        )
        all_options.append(options)

    execute_all_from(all_options, threads)


def generate_tables():
    path = Path.cwd() / "incremental_moves"
    for dir in path.iterdir():
        if dir.is_dir() and dir.stem.startswith("output_"):
            output_folder = path / "tables" / dir.stem
            gen_tables(results_folder=dir, output_folder=output_folder, format="csv")
            copy(dir / "options.txt", output_folder / "0options.txt")


if __name__ == "__main__":
    main()
