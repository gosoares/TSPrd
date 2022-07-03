import re
from itertools import chain

import pandas as pd


def get_instances_names():
    # return an iterable of tuples (instance_set, instance_name)
    solomon_instances = [("Solomon", f"{n}/{name}") for n in [10, 15, 20, 50, 100] for name in ["C101", "C201", "R101", "RC101"]]
    tsplib_names = ["eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100", "kroE100", "rd100", "eil101",
                    "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150", "kroA150", "kroB150", "pr152", "u159", "rat195",
                    "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226", "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439",
                    "pcb442", "d493"]
    tsplib_instances = [["TSPLIB", name] for name in tsplib_names]
    atsplib_instances = [["aTSPLIB", name] for name in ["ftv33", "ft53", "ftv70", "kro124p", "rbg403"]]
    return chain(solomon_instances, tsplib_instances, atsplib_instances)


def get_instances_execs():
    # return an iterable of tuples (instance_set, n, instance_name, beta, exec_id)
    instances = get_instances_names()
    betas = ["0.5", "1", "1.5", "2", "2.5", "3"]
    exec_ids = range(1, 11)

    r = re.compile(r"\d+")
    instances = [(iset, int(r.search(name).group()), name, beta, exec_id) for iset, name in instances for beta in betas for exec_id in exec_ids]
    return instances


def gen_instances_df():
    # (instance_set, n, instance_name, beta, exec_id)
    instances = get_instances_execs()
    return pd.DataFrame(instances, columns=["set", "n", "name", "beta", "exec_id"])


def read_execution_data(results_folder: str):
    df = gen_instances_df()  # each row of this dataframe represents one of ten execution of each instance n_instance_beta

    # read ref obj (obj from archetti paper)
    ref_data = pd.read_csv("../instances/reference_data.csv", dtype={"beta": str, "opt": "Int64", "ref_time": "Int64"})
    df = df.merge(ref_data, how="left", on=["set", "name", "beta"], copy=False)

    # read obj, exec_time, sol_time for each execution row
    df[["obj", "exec_time", "sol_time"]] = df.apply(
        lambda row: read_instance_result(results_folder, row["set"], row["name"], row["beta"], row["exec_id"]),
        axis='columns', result_type='expand')
    df["obj"] = df["obj"].astype(int)
    df.insert(df.columns.get_loc("obj") + 1, "obj_gap_ref", calculate_gap(df["obj"], df["ref_obj"]))

    # remove n/ from solomon instances names, as it is not needed anymore
    df["name"] = df["name"].apply(lambda x: x.split("/")[-1])

    return df


def aggregate_data(df: pd.DataFrame):
    # calculate avg and best of obj and avg of times
    grouped = df.groupby(["set", "n", "beta", "name"])
    df_agg = grouped.agg(
        {"opt": "first", "ref_obj": "first", "ref_time": "first", "obj": ["min", "mean"], "obj_gap_ref": ["min", "mean"], "exec_time": "mean",
         "sol_time": "mean"})
    df_agg.columns = ["opt", "ref_obj", "ref_time", "best_obj", "avg_obj", "gap_best", "gap_avg", "exec_time", "sol_time"]
    df_agg.insert(df_agg.columns.get_loc("best_obj") + 1, "gap_best", df_agg.pop("gap_best"))  # change pos of gap_best column
    return df_agg


def read_instance_result(output_path: str, instance_set: str, instance: str, beta: str, exec_id: int):
    file = f"{output_path}/{instance_set}/{instance}_{beta}_{exec_id}.txt"
    with open(file, 'r') as f:
        _, exec_time = f.readline().split()
        _, sol_time = f.readline().split()
        _, obj = f.readline().split()
        return int(obj), int(exec_time), int(sol_time)


def calculate_gap(value: pd.Series, ref: pd.Series) -> float:
    return (100 * value / ref) - 100
