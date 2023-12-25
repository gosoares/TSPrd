import sys

from executor import ExecutionOptions, execute_all_from

all_intra_moves = ["reinsertion1", "reinsertion2", "swap11", "swap12", "swap22", "2opt"]
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
        output_folder="incremental_moves/output_{}_{}".format(len(all_intra_moves), i),
        intra_moves=all_intra_moves,
        inter_moves=inter_moves,
    )
    all_options.append(options)

threads = int(sys.argv[1])
execute_all_from(all_options, threads)
