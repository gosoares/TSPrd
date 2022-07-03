import argparse
from collections.abc import Iterable

from tables_saving import *
from tsprd_data import *


def main():
    parser = argparse.ArgumentParser(description="Generate table from the executor instance execution results folder.")
    parser.add_argument("results_folder", action="store", type=str, help="Folder with the TSPrd instance execution results.")
    args = parser.parse_args()

    gen_tables(args.results_folder)


def gen_tables(results_folder: str):
    df = read_execution_data(results_folder)  # read the results of the executions
    df_agg = aggregate_data(df)  # group the 10 executions and calculate the relevant data (means, sums)

    solomon, tsplib, atsplib = [group.droplevel(level=0) for _, group in df_agg.groupby(level=0)]

    solomon_opt = solomon.iloc[solomon.index.get_level_values('n') <= 20].copy()
    solomon_nopt = solomon.iloc[solomon.index.get_level_values('n') > 20].drop(columns="opt")
    # calculate gaps in relation to the optimal result
    solomon_opt.insert(solomon_opt.columns.get_loc("ref_obj") + 1, "ref_gap", calculate_gap(solomon_opt["ref_obj"], solomon_opt["opt"]))
    solomon_opt["gap_best"] = calculate_gap(solomon_opt["best_obj"], solomon_opt["opt"])
    solomon_opt["gap_avg"] = calculate_gap(solomon_opt["avg_obj"], solomon_opt["opt"])

    tsplib.drop(columns=["opt", "ref_time"], inplace=True)
    atsplib.drop(columns=["opt", "ref_time"], inplace=True)

    gen_solomon_tables(solomon_opt, solomon_nopt)
    gen_tsplib_tables(tsplib)
    gen_atsplib_tables(atsplib)
    gen_opt_summary_table(solomon_opt)
    gen_nopt_summary_table(solomon_nopt, tsplib, atsplib)


def gen_solomon_tables(solomon_opt: pd.DataFrame, solomon_nopt: pd.DataFrame):
    solomon_opt = insert_blank_columns(solomon_opt.copy(), pos=1, before=("best_obj", "avg_obj", "exec_time"))
    solomon_nopt = insert_blank_columns(solomon_nopt.copy(), pos=0, before=("best_obj", "avg_obj", "exec_time"))

    for n, df in chain(solomon_opt.groupby(level=0), solomon_nopt.groupby(level=0)):  # iterate a dataframe for each `n`
        if df["ref_time"].isnull().all():
            df.drop(columns="ref_time", inplace=True)
        df = df.droplevel(0)
        save_table(f"solomon{n}", df, gen_avg_footer(df))


def gen_tsplib_tables(tsplib: pd.DataFrame):
    tsplib = tsplib.sort_index(level=["beta", "n"]).droplevel("n")
    insert_blank_columns(tsplib, before=["ref_obj"])
    tsplib = tsplib.stack(level=0).unstack(level=1).transpose()
    int_columns = tsplib.columns.get_level_values(-1).isin(["ref_obj", "best_obj"])
    tsplib.iloc[:, int_columns] = tsplib.iloc[:, int_columns].astype(int)

    tsplib_beta = tuple(tsplib.groupby(level="beta", axis="columns"))
    for i in range(0, 6, 2):
        tsplib_two = pd.concat([tsplib_beta[i][1], tsplib_beta[i + 1][1]], axis="columns")
        save_table(f"tsplib_{tsplib_beta[i][0]}_{tsplib_beta[i + 1][0]}", tsplib_two, gen_avg_footer(tsplib_two))


def gen_atsplib_tables(atsplib: pd.DataFrame):
    atsplib = atsplib.sort_index(level=["beta", "n"]).droplevel("n")
    insert_blank_columns(atsplib, pos=1, before=("best_obj", "avg_obj", "exec_time"))
    save_table(f"atsplib", atsplib, gen_avg_footer(atsplib))


def gen_opt_summary_table(solomon_opt: pd.DataFrame):
    sol_opt = solomon_opt.groupby(["n", "beta"]) \
        .agg(count=("exec_time", "count"), ref_gap=("ref_gap", "mean"), ref_time=("ref_time", "mean"), gap_best=("gap_best", "mean"),
             gap_avg=("gap_avg", "mean"), exec_time=("exec_time", "mean"), sol_time=("sol_time", "mean"))
    sol_opt["ref_time"] = sol_opt["ref_time"].apply(lambda x: "-" if pd.isna(x) else int(x))

    # count how many each equal the optimal result
    sol_opt.insert(sol_opt.columns.get_loc("ref_gap"), "ref_n_opt", solomon_opt.query("ref_obj == opt").groupby(sol_opt.index.names).size())
    sol_opt.insert(sol_opt.columns.get_loc("gap_best"), "best_n_opt", solomon_opt.query("best_obj == opt").groupby(sol_opt.index.names).size())
    sol_opt.insert(sol_opt.columns.get_loc("gap_avg"), "avg_n_opt", solomon_opt.query("avg_obj == opt").groupby(sol_opt.index.names).size())

    insert_blank_columns(sol_opt, before=("ref_n_opt", "best_n_opt", "avg_n_opt", "exec_time"))
    avg_footer = gen_avg_footer(sol_opt)
    sum_footer = gen_sum_footer(sol_opt)
    footer = pd.concat([avg_footer, sum_footer])
    save_table("summary_opt", sol_opt, footer)


def gen_nopt_summary_table(solomon_nopt: pd.DataFrame, tsplib: pd.DataFrame, atsplib: pd.DataFrame):
    tsplib_agg = gen_tsplib_summary(tsplib)
    summary_nopt = pd.concat([tsplib_agg], keys=["TSPLIB"])
    summary_nopt.insert(summary_nopt.columns.get_loc("n_ref_sb_avg") + 1, "ref_time", "-")
    save_table("summary_nopt", summary_nopt)


def gen_tsplib_summary(tsplib: pd.DataFrame):
    def tsplib_grouping(df: pd.DataFrame):
        return [pd.cut(df.index.get_level_values("n"), bins=[49, 100, 150, 250, 500]), "beta"]

    tsplib_agg = tsplib.groupby(tsplib_grouping(tsplib)).agg(count=("ref_obj", "count"), gap_best=("gap_best", "mean"), gap_avg=("gap_avg", "mean"),
                                                             exec_time=("exec_time", "mean"), sol_time=("sol_time", "mean"))

    ref_pos = tsplib_agg.columns.get_loc("gap_best")  # position to insert next few columns
    tsplib_agg.insert(ref_pos, "n_ref_sb_best", gen_count_column(tsplib, "ref_obj < best_obj", tsplib_grouping))
    tsplib_agg.insert(ref_pos + 1, "n_ref_sb_avg", gen_count_column(tsplib, "ref_obj < avg_obj", tsplib_grouping))
    tsplib_agg.insert(ref_pos + 2, "n_sb_best", gen_count_column(tsplib, "best_obj < ref_obj", tsplib_grouping))
    tsplib_agg.insert(tsplib_agg.columns.get_loc("gap_avg"), "n_sb_avg", gen_count_column(tsplib, "avg_obj < ref_obj", tsplib_grouping))

    tsplib_agg[["n_ref_sb_best", "n_ref_sb_avg", "n_sb_best", "n_sb_avg"]] = tsplib_agg[["n_ref_sb_best", "n_ref_sb_avg", "n_sb_best", "n_sb_avg"]] \
        .fillna(0).astype(int)

    insert_blank_columns(tsplib_agg, before=("n_ref_sb_best", "n_sb_best", "n_sb_avg", "exec_time"))
    return tsplib_agg


def gen_count_column(df: pd.DataFrame, condition: str, grouping) -> pd.Series:
    # count how many rows in each `grouping` of `df` respect the `condition`
    sb = df.query(condition)
    if callable(grouping):
        grouping = grouping(sb)
    return sb.groupby(grouping).size()


def insert_blank_columns(df: pd.DataFrame, pos: int | Iterable[int] = (), before: Iterable[str] = (), after: Iterable[str] = ()):
    pos = pos if isinstance(pos, Iterable) else (pos,)
    pos = chain(pos, (df.columns.get_loc(c) for c in before), (df.columns.get_loc(c) + 1 for c in after))
    for i, p in enumerate(sorted(pos, reverse=True)):
        df.insert(p, f"blank{i}", "")
    return df


def gen_avg_footer(df: pd.DataFrame):
    footer = df.iloc[:, df.columns.get_level_values(-1).isin(["gap_best", "gap_avg", "exec_time", "sol_time"])].mean().rename(f"Avg.")
    fill_columns = [f"__fill_{i}" for i in range(len(df.index.names) - 1)]  # add columns to compensate if df has multiindex
    footer = pd.DataFrame([footer], columns=(fill_columns + list(df.columns))).fillna('')
    if type(df.columns) is pd.MultiIndex:
        footer.columns = pd.MultiIndex.from_tuples(footer.columns)

    if "ref_gap" in footer:
        footer["ref_gap"] = df["ref_gap"].mean()
    if "ref_time" in footer:
        footer["ref_time"] = df["ref_time"].mean().astype(int) if pd.api.types.is_numeric_dtype(df["ref_time"]) else "-"
    return footer


def gen_sum_footer(df: pd.DataFrame):
    footer = df[["count", "ref_n_opt", "best_n_opt", "avg_n_opt"]].sum().astype("Int64").rename("Total")
    fill_columns = [f"__fill_{i}" for i in range(len(df.index.names) - 1)]  # add columns to compensate if df has multiindex
    footer = pd.DataFrame([footer], columns=(fill_columns + list(df.columns))).fillna('')
    return footer


if __name__ == "__main__":
    main()
