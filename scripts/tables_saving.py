from pathlib import Path

import pandas as pd


def save_table(file: str, df: pd.DataFrame, footer: pd.DataFrame = None):
    df.index.names = [None for _ in range(len(df.index.names))]  # clear index names
    tex = get_table_tex(df)
    if footer is not None:
        if df.index.nlevels == 1:
            tex += "\\midrule\n"
        footer_tex = get_table_tex(footer)
        tex += footer_tex
    tex += "\\bottomrule"
    Path("output").mkdir(parents=True, exist_ok=True)
    print(tex, file=open(f"output/{file}.tex", "w"))


def get_table_tex(df: pd.DataFrame):
    df = format_time_columns(df)  # format times
    tex: str = df.style.format(precision=2).hide(axis="columns") \
        .to_latex(clines="skip-last;data").replace("\\cline", "\\cmidrule")
    tex = tex.split('\n', 1)[1].replace("\\end{tabular}\n", "")  # remove \tabular from first and last lines
    return tex


def format_time_columns(df: pd.DataFrame):
    columns = df.columns.get_level_values(-1).isin(["exec_time", "sol_time"])
    df.iloc[:, columns] = df.iloc[:, columns].apply(lambda x: x.apply(format_time), axis='columns', result_type='expand')
    return df


def format_time(time):
    if not time:
        return ''
    time = time / 1000.0  # ms to s
    return "{:.2f}".format(time) if time >= 0.01 else "<0.01"
