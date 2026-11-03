"""Render saved Teleplot-style serial data as standalone HTML; stdlib only."""
import argparse
import html
import math
from collections import defaultdict
from pathlib import Path


def parse_log(text):
    channels = defaultdict(list)
    for line in text.splitlines():
        line = line.strip()
        if not line.startswith(">") or ":" not in line:
            continue
        name, raw = line[1:].split(":", 1)
        try:
            value = float(raw)
        except ValueError:
            continue
        if name and math.isfinite(value):
            channels[name].append(value)
    return channels


def chart(name, values):
    low, high = min(values), max(values)
    span = high - low or max(abs(high) * 0.1, 1)
    points = " ".join(
        f"{60 + 760 * i / max(len(values) - 1, 1):.2f},"
        f"{190 - 150 * (value - low) / span:.2f}"
        for i, value in enumerate(values)
    )
    title = html.escape(name)
    return (
        f"<section><h2>{title}</h2><p>{len(values)} reported samples; "
        f"min={low:g}, max={high:g}, last={values[-1]:g}</p>"
        '<svg viewBox="0 0 860 240" role="img" '
        f'aria-label="{title}">'
        '<path d="M60 25V195H825" fill="none" stroke="#8695ab"/>'
        f'<polyline points="{points}" fill="none" stroke="#2463eb" stroke-width="2"/>'
        f'<text x="60" y="18">{high:g}</text>'
        f'<text x="5" y="195">{low:g}</text>'
        '<text x="320" y="230">Reported sample index (not elapsed time)</text>'
        '</svg></section>'
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="UTF-8 text copied from Serial Monitor")
    parser.add_argument("--output", type=Path, default=Path("bai1_plot.html"))
    args = parser.parse_args()
    channels = parse_log(args.input.read_text(encoding="utf-8-sig"))
    if not channels:
        parser.error("No valid >name:value samples in input")
    plots = "".join(chart(name, values) for name, values in channels.items())
    document = (
        '<!doctype html><html lang="en"><meta charset="utf-8">'
        '<meta name="viewport" content="width=device-width,initial-scale=1">'
        '<title>TH2 Bai 1 - saved Serial log</title>'
        '<style>body{font:16px system-ui;max-width:980px;margin:32px auto;padding:0 20px;'
        'background:#f2f5fa;color:#14243a}section{background:white;padding:20px;'
        'margin:20px 0;border-radius:12px}svg{width:100%;height:auto}text{font-size:12px}</style>'
        '<h1>TH2 Bai 1 - saved Serial log</h1><p>Source: '
        + html.escape(args.input.name)
        + '. Values are from the supplied log. Hardware reports may skip intermediate ISR samples. '
        'Units: *_us = microseconds; bienTro = ADC counts; led = 0/1.</p>'
        + plots + '</html>'
    )
    args.output.write_text(document, encoding="utf-8")
    print(f"Wrote {args.output} ({len(channels)} channels)")


if __name__ == "__main__":
    main()
