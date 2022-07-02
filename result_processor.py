import itertools
import re

import pandas as pd


def main():
    output_folder = "outputfinal"

    df = read_execution_data(output_folder)  # read the results of the executions
    df_agg = aggregate_data(df)  # group the 10 executions and calculate the relevant data (means, sums)

    gen_tables(df_agg)


def gen_tables(df_agg: pd.DataFrame):
    dfs = {idx: group.droplevel(level=0) for idx, group in df_agg.groupby(level=0)}  # separate by instance set and remove set from index

    solomon = dfs["Solomon"]
    solomon_opt = solomon.iloc[solomon.index.get_level_values('n') <= 20]
    solomon_nopt = solomon.iloc[solomon.index.get_level_values('n') > 20].drop(columns="opt")
    gen_solomon_nopt_tables(solomon_nopt)


def gen_solomon_nopt_tables(solomon_nopt: pd.DataFrame):
    solomon_nopt.index.names = [None for _ in range(len(solomon_nopt.index.names))]  # clear index names

    # insert blank columns for formatting
    solomon_nopt.insert(0, 'blank', "")
    solomon_nopt.insert(solomon_nopt.columns.get_loc("best_obj"), 'blank2', "")
    solomon_nopt.insert(solomon_nopt.columns.get_loc("avg_obj"), 'blank3', "")
    solomon_nopt.insert(solomon_nopt.columns.get_loc("exec_time"), 'blank4', "")

    for n, data in solomon_nopt.groupby(level=0):  # iterate a dataframe for each `n`
        save_table(f"solomon{n}_data", data.droplevel(0), gen_avg_footer(data))


def save_table(file: str, df: pd.DataFrame, footer: pd.DataFrame = None):
    tex = get_table_tex(df)
    if footer is not None:
        tex += get_table_tex(footer)
    tex += "\\bottomrule"
    # tex = ''.join(tex.rsplit("\\\\", 1))  # remove last `\\` from tex
    print(tex, file=open(f"output/{file}.tex", "w"))


def get_table_tex(df: pd.DataFrame):
    df = format_time_columns(df)  # format times
    tex: str = df.style.format(precision=2).hide(axis="columns") \
        .to_latex(clines="skip-last;data").replace("\\cline", "\\cmidrule")
    tex = tex.split('\n', 1)[1].replace("\\end{tabular}\n", "")  # remove \tabular from first and last lines
    return tex


def gen_avg_footer(df: pd.DataFrame):
    footer = df[["ref_time", "gap_best", "gap_avg", "exec_time", "sol_time"]].mean().rename(f"Avg.")
    fill_columns = [f"__fill_{i}" for i in range(len(df.index.names) - 2)]  # add columns to compensate if df has multiindex
    footer = pd.DataFrame([footer], columns=(fill_columns + list(df.columns))).fillna('')
    footer["ref_time"] = footer["ref_time"].astype(int)
    return footer


def gen_summary_table(df2: pd.DataFrame):
    df2.index.names = [None for _ in range(len(df2.index.names))]  # clear index names
    dfs: dict[str, pd.DataFrame] = dict(tuple(df2.groupby(level=0)))

    # summary of solomon data
    solomon_grouped = dfs["Solomon"].groupby(["n", "beta"])
    solomon = solomon_grouped[["exec_time", "sol_time"]].mean()  # mean of times for each (n, beta)
    # solomon["qnt"] = solomon_grouped["name"].count()
    solomon_opt = solomon.iloc[solomon.index.get_level_values('n') <= 20]
    solomon_nopt = solomon.iloc[solomon.index.get_level_values('n') > 20]

    # summary of tsplib data
    tsplib = dfs["TSPLIB"]
    tsplib_grouped = tsplib.groupby([pd.cut(tsplib['n'], bins=[49, 100, 150, 250, 500]), 'beta'])
    tsplib = tsplib_grouped[["exec_time", "sol_time"]].mean()
    # tsplib["qnt"] = tsplib_grouped["name"].count()

    # summary of atsplib data
    atsplib_grouped = dfs["aTSPLIB"].groupby("beta")
    atsplib = atsplib_grouped[["exec_time", "sol_time"]].mean()
    # atsplib["qnt"] = atsplib_grouped["name"].count()

    all_times = pd.concat([solomon_nopt, tsplib, atsplib])
    a = all_times.mean(axis=0).apply(format_time)
    mean_times_formatted = get_formatted_times(all_times.mean(axis=0))

    all_times_formatted = get_formatted_times(all_times)


def aggregate_data(df: pd.DataFrame):
    # calculate avg and best of obj and avg of times
    grouped = df.groupby(["set", "n", "beta", "name"])
    df_agg = grouped.agg(
        {"opt": "first", "ref_obj": "first", "ref_time": "first", "obj": ["min", "mean"], "obj_gap_ref": ["min", "mean"], "exec_time": "mean",
         "sol_time": "mean"})
    df_agg.columns = ["opt", "ref_obj", "ref_time", "best_obj", "avg_obj", "gap_best", "gap_avg", "exec_time", "sol_time"]
    df_agg.insert(df_agg.columns.get_loc("best_obj") + 1, "gap_best", df_agg.pop("gap_best"))  # change pos of gap_best column
    return df_agg


def read_execution_data(output_folder: str):
    df = gen_instances_df()  # each row of this dataframe represents one of ten execution of each instance n_instance_beta

    # read ref obj (obj from archetti paper)
    ref_data = pd.read_csv("instances/reference_data.csv", dtype={"beta": str, "opt": "Int64", "ref_time": "Int64"})
    df = df.merge(ref_data, how="left", on=["set", "name", "beta"], copy=False)

    # read obj, exec_time, sol_time for each execution row
    df[["obj", "exec_time", "sol_time"]] = df.apply(
        lambda row: read_instance_result(output_folder, row["set"], row["name"], row["beta"], row["exec_id"]),
        axis='columns', result_type='expand')
    df["obj"] = df["obj"].astype(int)
    df.insert(df.columns.get_loc("obj") + 1, "obj_gap_ref", (100 * df["obj"] / df["ref_obj"]) - 100)

    # remove n/ from solomon instances names, as it is not needed anymore
    df["name"] = df["name"].apply(lambda x: x.split("/")[-1])

    return df


def get_formatted_times(dfi: pd.DataFrame):
    df = dfi[["exec_time", "sol_time"]]
    return format_time_columns(df)


def format_time_columns(df: pd.DataFrame):
    df["exec_time"] = df["exec_time"].apply(format_time)
    df["sol_time"] = df["sol_time"].apply(format_time)
    return df


def format_time(time):
    time = time / 1000.0  # ms to s
    return "{:.2f}".format(time) if time >= 0.01 else "<0.01"


def read_instance_result(output_path: str, instance_set: str, instance: str, beta: str, exec_id: int):
    file = f"{output_path}/{instance_set}/{instance}_{beta}_{exec_id}.txt"
    with open(file, 'r') as f:
        _, exec_time = f.readline().split()
        _, sol_time = f.readline().split()
        _, obj = f.readline().split()
        return int(obj), int(exec_time) * (1201.0 / 1976.0), int(sol_time) * (1201.0 / 1976.0)


def gen_instances_df():
    # read (set, name, n) for all instances
    solomon_instances = [["Solomon", n, f"{n}/{name}"] for n in [10, 15, 20, 50, 100] for name in ["C101", "C201", "R101", "RC101"]]
    tsplib_names = ["eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100", "kroE100", "rd100", "eil101",
                    "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150", "kroA150", "kroB150", "pr152", "u159", "rat195",
                    "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226", "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439",
                    "pcb442", "d493"]
    tsplib_instances = [["TSPLIB", int(re.search(r'(\d+)p?$', name).groups()[0]), name] for name in tsplib_names]
    atsplib_instances = [["aTSPLIB", int(re.search(r'(\d+)p?$', name).groups()[0]), name] for name in ["ftv33", "ft53", "ftv70", "kro124p", "rbg403"]]

    all_instances = itertools.chain(solomon_instances, tsplib_instances, atsplib_instances)
    all_instances = [[*x, beta, exec_id] for x in all_instances for beta in ["0.5", "1", "1.5", "2", "2.5", "3"] for exec_id in
                     range(1, 11)]  # add (beta, exec_id)

    return pd.DataFrame(all_instances, columns=["set", "n", "name", "beta", "exec_id"])


if __name__ == "__main__":
    main()
