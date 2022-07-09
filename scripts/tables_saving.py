from collections.abc import Callable
from itertools import chain
from pathlib import Path

import pandas as pd
import re


def sb_s(s: str = ""):
    s = f"\\\\{s}" if s else s
    return f"\\shortstack{{strictly\\\\better{s}}}"


GAP = "gap (\\%)"
NOPT = "\\# opt"

c_transl = {  # for each column put the name and the alignment
    "n": ("n", "c"), "n_inst": ("\\# inst", "r"), "n_ref_opt": (NOPT, "c"), "n_opt_best": (NOPT, "c"), "n_opt_avg": (NOPT, "c"),
    "n_ref_sb_best": (sb_s("(best run)"), "c"), "n_ref_sb_avg": (sb_s("(all runs)"), "c"), "n_sb_best": (sb_s(), "c"), "n_sb_avg": (sb_s(), "c"),
    "beta": ("$\\beta$", "c"), "name": ("inst", "l"), "opt": ("opt", "r"), "ref_obj": ("obj", "r"), "ref_gap": (GAP, "c"), "ref_time": ("TB", "r"),
    "best_obj": ("obj", "r"), "gap_best": (GAP, "c"), "avg_obj": ("obj", "r"), "gap_avg": (GAP, "c"), "exec_time": ("TT", "r"),
    "sol_time": ("TB", "r")
}


def save_table(name: str, caption: str, df: pd.DataFrame, add_options: str = "\\tiny"):
    df = with_blank_columns(df)

    tex = get_table_tex(df, name, caption)
    reg = re.compile(r"(\\toprule)((?:.|\n)*)(\\bottomrule)")
    body_tex = reg.search(tex).groups()[1] # get content between \toprule and \bottomrule
    tex = reg.sub(r"\1\n__BODY__\n\g<3>", tex)  # get the outer part of \toprule and \bottomrule

    header_tex = gen_table_headers(df)
    footer_tex = gen_table_footer(df)
    body_tex = header_tex + body_tex + footer_tex
    tex = tex.replace("__BODY__", body_tex)
    tex = tex.replace("\\begin{tabular}", f"{add_options}\n\\begin{{tabular}}", 1)

    if name.startswith("tsplib"):
        tex = re.sub(r"(\\begin{tabular}(?:.|\n)*\\end{tabular})", r"\\resizebox{\\textwidth}{!}{\n\1\n}", tex)

    Path("output").mkdir(parents=True, exist_ok=True)
    print(tex, file=open(f"output/{name}.tex", "w"))


def with_blank_columns(df: pd.DataFrame):
    insert_before = ["ref_obj", "best_obj", "avg_obj", "exec_time", "n_ref_opt",
                     "n_opt_best", "n_opt_avg", "n_ref_sb_best", "n_sb_best", "n_sb_avg"]
    columns = df.columns
    blank_id = 1
    multi_index = isinstance(columns, pd.MultiIndex)
    new_columns = []

    for column in columns:
        cname = column[-1] if multi_index else column
        if cname in insert_before:
            nm = f"__blank_{blank_id}__"
            c = (*column[:-1], nm) if multi_index else nm
            new_columns.append(c)
            blank_id += 1
        new_columns.append(column)

    return df.reindex(new_columns, axis=1, fill_value="")


def get_table_tex(df: pd.DataFrame, name: str = None, caption: str = None):
    df = format_time_columns(df.copy())  # format times
    alignments = [c_transl.get(c, ('', 'c'))[1] for c in chain(df.index.names, df.columns.get_level_values(-1))]
    alignments = ''.join(alignments)

    df.index.names = [None for _ in range(len(df.index.names))]  # clear index names
    tex: str = df.style.format(precision=2, na_rep="").hide(axis="columns") \
        .format_index(lambda x: f"\\shortstack{{{x.left + 1}\\\\\\~{{}}\\\\{x.right}}}" if isinstance(x, pd.Interval) else x) \
        .to_latex(clines="skip-last;data", hrules=True, label=f"tab:{name}", caption=caption, position="htb!", position_float="centering",
                  column_format=alignments) \
        .replace("\\cline", "\\cmidrule")
    tex = re.sub(r"(?:\\cmidrule{\d+-\d+}\s?)+\n(\\bottomrule)", r"\1", tex.strip(" \n"))  # remove \cmidrule{} from before \bottomrule
    return tex


def gen_table_footer(df: pd.DataFrame):
    # avg footer
    columns = set(filter(lambda x: "gap" in x or x.endswith("_time"), df.columns.get_level_values(-1)))
    index = ("Avg.",) + (" ",) * (len(df.index.names) - 1) if len(df.index.names) > 1 else "Avg."
    footer = df.iloc[:, df.columns.get_level_values(-1).isin(columns)].mean(numeric_only=True).rename(index)
    if "ref_time" in footer:
        footer["ref_time"] = footer["ref_time"].astype(int) if pd.api.types.is_numeric_dtype(df["ref_time"]) else "-"
    df_footer = footer.to_frame().reindex(df.columns, fill_value="").transpose()

    # sum footer
    columns = list(filter(lambda c: c.startswith("n_"), df.columns.get_level_values(-1)))
    if columns:
        index = ("Total",) + (" ",) * (len(df.index.names) - 1) if len(df.index.names) > 1 else "Total"
        footer = df.iloc[:, df.columns.get_level_values(-1).isin(columns)].sum().astype("Int64")
        df_footer.loc[index, :] = footer

    # footer_tex = "\n" if df.index.nlevels == 1 else ""
    footer_tex = get_table_tex(df_footer)
    footer_tex = re.search(r"\\toprule((.|\n)*)\\bottomrule", footer_tex).groups()[0]
    return footer_tex.strip(" \n")


def format_time_columns(df: pd.DataFrame):
    columns = df.columns.get_level_values(-1).isin(["exec_time", "sol_time"])
    df.iloc[:, columns] = df.iloc[:, columns].apply(
        lambda x: x.apply(format_time), axis='columns', result_type='expand')
    return df


def format_time(time):
    if pd.isna(time):
        return time
    time = time / 1000.0  # ms to s
    return "{:.2f}".format(time) if time >= 0.01 else "<0.01"


def gen_table_headers(df: pd.DataFrame):
    columns = list(chain(df.index.names, df.columns.get_level_values(-1)))
    n_columns = len(columns)

    tex = ""
    if df.columns.nlevels == 2:  # beta level
        n_plus = len(df.index.names)
        level_columns = list(dict.fromkeys(df.columns.get_level_values(0)))
        cs = []
        for level_column in level_columns:
            level_column_range = df.columns.get_loc(level_column)
            cs.append((f"$\\beta = {level_column}$", n_plus +
                       level_column_range.start + 1, n_plus + level_column_range.stop - 1))
        tex = tex_header(n_columns, cs)

    start = 0
    first_row, second_row = [], []
    while start < n_columns:
        first_ref = index_first(columns, lambda x: x and "ref_" in x, start)
        last_ref = index_firsts_last(columns, lambda x, _: "ref_" in x, first_ref)
        first_hgs = index_first(columns, lambda x: x in ("best_obj", "n_opt_best", "n_sb_best"), last_ref)
        first_best = first_hgs
        first_avg = index_first(columns, lambda x: x in ("avg_obj", "n_opt_avg", "n_sb_avg"), first_best)
        rhc = ("gap_avg", "exec_time", "sol_time")  # remaining hgs columns
        last_hgs = index_firsts_last(columns, lambda x, i: x in rhc or (x.startswith("__") and (i + 1 == len(columns) or columns[i + 1] in rhc)),
                                     first_avg + 1)
        start = last_hgs + 1

        first_row.extend([("\\archils{}", first_ref, last_ref), ("\\myalg{}", first_hgs, last_hgs)])
        second_row.extend([("best run", first_best, first_best + 1), ("all runs", first_avg, first_avg + 1)])

    tex += tex_header(n_columns, first_row)
    tex += tex_header(n_columns, second_row)
    translated_columns = ["" if not c or c.startswith("__") else c_transl[c][0] for c in columns]
    tex += " & ".join(translated_columns) + " \\\\ \n"
    return tex


def tex_header(n_columns: int, texts: list[tuple[str, int, int]]):  # texts: lists of (text, start_pos, end_pos)
    line = (" & " * texts[0][1]) + multicolumn(*texts[0])
    for i in range(1, len(texts)):
        line += (" & " * (texts[i][1] - texts[i - 1][2])) + multicolumn(*texts[i])
    line += (" & " * (n_columns - texts[-1][2] - 1)) + " \\\\ \n"

    for _, start, end in texts:
        line += f"\\cmidrule{{{start + 1}-{end + 1}}} "
    line += "\n"
    return line


def multicolumn(text: str, _from: int, to: int):
    n = to - _from + 1
    if n > 1:
        return f"\\multicolumn{{{n}}}{{c}}{{{text}}}"
    elif n == 1:
        return text
    else:
        return ""


def index_first(it: list, condition: Callable, start: int = 0):
    # return the index of the first element that satisfies condition
    for i in range(start, len(it)):
        if condition(it[i]):
            return i
    return -1


def index_firsts_last(it: list, condition: Callable, start: int = 0):
    # return the (index-1) of the first element that does not satisfies the condition
    for i in range(start, len(it)):
        if not condition(it[i], i):
            return i - 1
    return len(it) - 1
