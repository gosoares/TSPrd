import argparse
from itertools import chain
from pathlib import Path
from typing import Iterable

import pandas as pd
from pandas.core.groupby import DataFrameGroupBy

from tables_saving import Table, save_table_csv, save_table_tex
from tsprd_data import aggregate_data, read_execution_data, split_instance_sets


def main():
    parser = argparse.ArgumentParser(
        description="Generate table from the executor instance execution results folder."
    )
    parser.add_argument(
        "results_folder",
        action="store",
        type=str,
        help="Folder with the TSPrd instance execution results.",
    )
    parser.add_argument(
        "-o",
        "--output_folder",
        action="store",
        type=str,
        help="Folder to save the generated tables.",
        required=False,
    )
    parser.add_argument(
        "--tex",
        action="store_true",
        help="Save the tables in tex format instead of csv.",
    )
    args = parser.parse_args()

    results_folder = Path(args.results_folder)
    output_folder = Path(
        args.output_folder
        if args.output_folder
        else Path.cwd() / "tables" / results_folder.stem
    )

    gen_tables(results_folder, output_folder, "tex" if args.tex else "csv")
    print("Finished!")


def gen_tables(results_folder: Path, output_folder: Path, format: str = "csv"):
    for table in get_tables(results_folder):
        if format == "tex":
            save_table_tex(table, output_folder)
        elif format == "csv":
            save_table_csv(table, output_folder)
        else:
            raise ValueError(f"Invalid format: {format}")


def get_tables(results_folder: Path) -> Iterable[Table]:
    # read the results of the executions
    df = read_execution_data(results_folder)
    # group the 10 executions and calculate the relevant data (means, sums)
    df_agg = aggregate_data(df)
    (
        solomon_opt,
        solomon_mixed,
        solomon50,
        solomon100,
        solomon_nref,
        tsplib,
        atsplib,
    ) = split_instance_sets(df_agg)
    solomon50_opt = solomon50[~solomon50["opt"].isna()]
    solomon50_nopt = solomon50[solomon50["opt"].isna()]
    solomon50.drop(columns=["opt"], inplace=True)

    yield from get_solomon_tables(solomon_opt, solomon_mixed, solomon50, solomon100)
    yield from get_tsplib_tables(tsplib)
    yield from get_atsplib_table(atsplib)
    yield from get_opt_summary_table(solomon_opt, solomon50_opt)
    yield from get_nopt_summary_table(solomon50_nopt, solomon100, tsplib, atsplib)
    yield from get_nref_summary_tables(solomon_nref)


def get_solomon_tables(
    solomon_opt: pd.DataFrame,
    solomon_mixed: pd.DataFrame,
    solomon50: pd.DataFrame,
    solomon100: pd.DataFrame,
):
    # iterate a dataframe for each `n`
    for n, df in chain(
        solomon_opt.groupby(level=0),
        solomon_mixed.groupby(level=0),
        solomon50.groupby(level=0),
        solomon100.groupby(level=0),
    ):
        for check in ["ref_obj", "ref_time", "ref_gap", "lb", "ub"]:
            if check in df and df[check].isna().all():
                df.drop(columns=check, inplace=True)
        df = df.droplevel(0)
        yield Table(
            name=f"solomon{n}",
            caption=f"Results for Solomon instances with n = {n}",
            df=df,
        )


def get_tsplib_tables(tsplib: pd.DataFrame):
    tsplib = tsplib.sort_index(level=["beta", "n"]).droplevel("n")

    tsplib_betas, tsplib_dfs = zip(*tuple(tsplib.groupby(level="beta")))
    for i in range(0, 6, 2):
        beta1, beta2 = tsplib_betas[i : i + 2]
        tsplib_two = pd.concat(
            [tsplib_dfs[i].droplevel("beta"), tsplib_dfs[i + 1].droplevel("beta")],
            axis="columns",
            keys=[beta1, beta2],
        )
        yield Table(
            name=f"tsplib_{beta1}_{beta2}",
            caption=f"Results for TSPLIB instances with $\\beta \\in {{{beta1}, {beta2}}}$",
            df=tsplib_two,
            add_options="\\setlength{\\tabcolsep}{3pt} \\tiny",
        )


def get_atsplib_table(atsplib: pd.DataFrame):
    atsplib = atsplib.sort_index(level=["beta", "n"]).droplevel("n")
    yield Table(name="atsplib", caption="Results for aTSPLIB instances", df=atsplib)


def get_opt_summary_table(solomon_opt: pd.DataFrame, solomon50_opt):
    grouped = pd.concat([solomon_opt.query("n <= 20"), solomon50_opt]).groupby(
        ["n", "beta"]
    )
    sol_opt = grouped.agg(
        n_inst=("exec_time", "count"),
        ref_gap=("ref_gap", "mean"),
        ref_time=("ref_time", "mean"),
        gap_best=("gap_best", "mean"),
        gap_avg=("gap_avg", "mean"),
        exec_time=("exec_time", "mean"),
        sol_time=("sol_time", "mean"),
    )
    sol_opt["ref_time"] = sol_opt["ref_time"].apply(
        lambda x: "-" if pd.isna(x) else int(x)
    )

    # count how many each equal the optimal result
    sol_opt.insert(
        sol_opt.columns.get_loc("ref_gap"),
        "n_ref_opt",
        grouped.apply(lambda d: d[d["ref_obj"] == d["opt"]].shape[0]),
    )
    sol_opt.insert(
        sol_opt.columns.get_loc("gap_best"),
        "n_opt_best",
        grouped.apply(lambda d: d[d["best_obj"] == d["opt"]].shape[0]),
    )
    sol_opt.insert(
        sol_opt.columns.get_loc("gap_avg"),
        "n_opt_avg",
        grouped.apply(lambda d: d[(d["avg_obj"] - d["opt"]).abs() < 0.0001].shape[0]),
    )

    yield Table(
        name="summary_opt",
        caption="Comparison of aggregated results for instances with known optimal solutions",
        df=sol_opt,
        add_options="\\fontsize{9pt}{11pt}\\selectfont",
    )


def get_nopt_summary_table(
    solomon50: pd.DataFrame,
    solomon100: pd.DataFrame,
    tsplib: pd.DataFrame,
    atsplib: pd.DataFrame,
):
    solomon_agg = get_solomon_nopt_summary(pd.concat([solomon50, solomon100]))
    tsplib_agg = get_tsplib_summary(tsplib)
    atsplib_agg = get_atsplib_summary(atsplib)

    summary_nopt = pd.concat(
        [solomon_agg, tsplib_agg, atsplib_agg], keys=["Solomon", "TSPLIB", "aTSPLIB"]
    )
    summary_nopt[["n_sb_best", "n_sb_avg"]] = (
        summary_nopt[["n_sb_best", "n_sb_avg"]].fillna(0).astype(int)
    )
    yield Table(
        name="summary_nopt",
        caption="Comparison of aggregated results for instances with unknown optimal solutions",
        df=summary_nopt,
        add_options="\\fontsize{8pt}{10pt}\\selectfont \\setlength{\\tabcolsep}{4pt}",
    )


def get_solomon_nopt_summary(solomon_nopt: pd.DataFrame):
    sol_grouped = solomon_nopt.groupby(["n", "beta"])
    sol_agg = nopt_summary_agg(sol_grouped)
    return sol_agg


def get_tsplib_summary(tsplib: pd.DataFrame):
    tsplib_grouped = tsplib.groupby(
        [
            pd.cut(tsplib.index.get_level_values("n"), bins=[49, 100, 150, 250, 500]),
            "beta",
        ],
        observed=False,
    )
    tsplib_agg = nopt_summary_agg(tsplib_grouped)
    return tsplib_agg


def get_atsplib_summary(atsplib: pd.DataFrame):
    atsplib_grouped = atsplib.groupby(
        [pd.cut(atsplib.index.get_level_values("n"), bins=[32, 403]), "beta"],
        observed=False,
    )
    atsplib_agg = nopt_summary_agg(atsplib_grouped)
    return atsplib_agg


def nopt_summary_agg(df_grouped: DataFrameGroupBy):
    df_agg = df_grouped.agg(
        n_inst=("ref_obj", "count"),
        gap_best=("gap_best", "mean"),
        gap_avg=("gap_avg", "mean"),
        exec_time=("exec_time", "mean"),
        sol_time=("sol_time", "mean"),
    )

    df_agg.insert(
        df_agg.columns.get_loc("gap_best"),
        "n_sb_best",
        df_grouped.apply(lambda d: d[d["best_obj"] < d["ref_obj"]].shape[0]),
    )
    df_agg.insert(
        df_agg.columns.get_loc("gap_avg"),
        "n_sb_avg",
        df_grouped.apply(lambda d: d[d["avg_obj"] < d["ref_obj"]].shape[0]),
    )
    ref_time_column = (
        df_grouped["ref_time"].mean().astype(int)
        if "ref_time" in df_grouped.describe()
        else "-"
    )
    df_agg.insert(df_agg.columns.get_loc("n_inst") + 1, "ref_time", ref_time_column)
    return df_agg


def get_nref_summary_tables(solomon_nref: pd.DataFrame):
    nref_opt = solomon_nref[~solomon_nref["opt"].isna()]
    nref_nopt = solomon_nref[solomon_nref["opt"].isna()]
    yield from get_nref_opt_summary_table(nref_opt)
    yield from get_nref_nopt_summary_table(nref_nopt)


def nref_grouped_agg(nref: pd.DataFrame):
    grouped = nref.groupby(["n", "beta"])
    agg = grouped.agg(
        n_inst=("exec_time", "count"),
        gap_best=("gap_best", "mean"),
        gap_avg=("gap_avg", "mean"),
        exec_time=("exec_time", "mean"),
        sol_time=("sol_time", "mean"),
    )
    return grouped, agg


def get_nref_opt_summary_table(nref_opt: pd.DataFrame):
    grouped, agg = nref_grouped_agg(nref_opt)
    agg.insert(
        agg.columns.get_loc("gap_best"),
        "n_opt_best",
        grouped.apply(lambda d: d[d["best_obj"] == d["opt"]].shape[0]),
    )
    agg.insert(
        agg.columns.get_loc("gap_avg"),
        "n_opt_avg",
        grouped.apply(lambda d: d[(d["avg_obj"] - d["opt"]).abs() < 0.0001].shape[0]),
    )

    yield Table(
        name="summary_nref_opt",
        caption="Results for remaining instances with known optimal solutions",
        df=agg,
        add_options="\\fontsize{9pt}{11pt}\\selectfont",
    )


def get_nref_nopt_summary_table(nref_nopt: pd.DataFrame):
    grouped, agg = nref_grouped_agg(nref_nopt)
    agg.insert(
        agg.columns.get_loc("gap_best"),
        "n_sb_best",
        grouped.apply(lambda d: d[d["best_obj"] < d["ub"]].shape[0]),
    )
    agg.insert(
        agg.columns.get_loc("gap_avg"),
        "n_sb_avg",
        grouped.apply(lambda d: d[d["avg_obj"] < d["ub"]].shape[0]),
    )

    yield Table(
        name="summary_nref_nopt",
        caption="Comparison of aggregated results for instances with unknown optimal solutions",
        df=agg,
        add_options="\\fontsize{8pt}{10pt}\\selectfont \\setlength{\\tabcolsep}{4pt}",
    )


if __name__ == "__main__":
    main()
