from collections.abc import Iterable
from itertools import chain

from pylatex import Document, TikZ, TikZDraw, TikZCoordinate, TikZNode, TikZOptions, NoEscape

CH = 0.3  # columns height
FC_ = 12  # how much to divide the bin values
BH = 0.4  # bins height


def main():
    gen_tl_tex((0, 130), [(5, 15), (50, 100)], "divinv/original_tl")
    gen_tl_tex((0, 130), [(5, 15), (20, 40), (50, 95)], "divinv/case1_tl")
    gen_tl_tex((0, 130), [(5, 15), (50, 85), (85, 125)], "divinv/case2_tl")
    gen_tl_tex((0, 130), [(5, 15), (15, 55), (55, 90)], "divinv/case2_inv_tl")

    gen_tl_tex((0, 200), [(5, 95)], "ex_sol/a_tl")
    gen_tl_tex((0, 200), [(10, 65), (75, 140), (140, 165)], "ex_sol/b_tl")


def gen_tl_tex(bounds: tuple[int, int], bins: Iterable[tuple[int, int]], name: str):
    fc = FC_ / (bounds[1] - bounds[0])

    doc = Document(data=NoEscape("\\usetikzlibrary{shapes.geometric} \\usetikzlibrary{arrows.meta,arrows}"))
    with doc.create(TikZ()) as pic:
        pic.append(TikZDraw([TikZCoordinate(bounds[0] * fc, 0), '--',
                   TikZCoordinate(bounds[1] * fc, 0)], options=TikZOptions("->")))

        pic.append(TikZDraw([TikZCoordinate(bounds[0] * fc, -CH / 2), '--', TikZCoordinate(bounds[0] * fc, CH / 2)], ))
        for b in set(chain(*bins)):
            pic.append(TikZDraw([TikZCoordinate(b * fc, -CH / 2), '--', TikZCoordinate(b * fc, CH / 2)], ))
            pic.append(TikZNode(text=int(b), at=TikZCoordinate(b * fc, -CH * 1.3)))

        for bi in bins:
            pic.append(TikZDraw([TikZCoordinate((bi[0] - 0.5) * fc, BH), '--', TikZCoordinate((bi[1] + 0.5) * fc, BH)],
                                options=TikZOptions("Square-Square")))

        pic.generate_tex(f"output_tl/{name}")

    # doc.generate_pdf(f"output_tl/{name}_", clean_tex=True)


if __name__ == '__main__':
    main()
