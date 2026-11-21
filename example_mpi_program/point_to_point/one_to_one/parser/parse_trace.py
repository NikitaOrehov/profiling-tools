#!/usr/bin/env python3
"""
Парсер трассировок MPI для сводной таблицы.

Использование:
    python parse_trace.py --size "4 мб" trace_rank0.txt trace_rank1.txt
    python parse_trace.py --size "1 байт" --csv rank0.txt rank1.txt
    python parse_trace.py --size "4 кб" --tsv rank0.txt rank1.txt

Вывод по умолчанию — подробный отчёт.
--csv / --tsv   — одна строка для копирования в Word/Excel.
"""
import re
import sys
import os
import argparse

PATTERN = re.compile(
    r'^(\w+)\s+BEGIN\s+(\d+)(?:\s+[-\d]+)*\s+END\s+(\d+)\s*$'
)


def parse_file(path):
    events = {'Send': [], 'Recv': [], 'Isend': [], 'Irecv': [], 'Waitall': []}
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('SYSTEM_START_US'):
                continue
            m = PATTERN.match(line)
            if not m:
                continue
            op, start, end = m.group(1), int(m.group(2)), int(m.group(3))
            if op in events:
                events[op].append((start, end, end - start))
    return events


def detect_type(events):
    has_block = len(events['Send']) > 0 or len(events['Recv']) > 0
    has_nonblock = len(events['Isend']) > 0 or len(events['Irecv']) > 0
    if has_block and not has_nonblock:
        return 'blocking'
    if has_nonblock and not has_block:
        return 'nonblocking'
    return 'mixed'


def compute_metrics(events, trace_type):
    starts, ends = [], []
    for k in ['Send', 'Recv', 'Isend', 'Irecv']:
        for s, e, d in events[k]:
            starts.append(s)
            ends.append(e)

    first_begin = min(starts) if starts else None
    last_transmit_end = max(ends) if ends else None

    if trace_type == 'nonblocking' and events['Waitall']:
        last_end = events['Waitall'][-1][1]
    else:
        last_end = last_transmit_end

    duration = (last_end - first_begin) if (first_begin is not None and last_end is not None) else None

    return {
        'first_begin': first_begin,
        'last_end': last_end,
        'last_transmit_end': last_transmit_end,
        'duration': duration,
        'events': events,
    }


def combine_metrics(metrics_list):
    first_begin = min(m['first_begin'] for m in metrics_list if m['first_begin'] is not None)
    last_end = max(m['last_end'] for m in metrics_list if m['last_end'] is not None)
    duration = last_end - first_begin

    combined = {'Send': [], 'Recv': [], 'Isend': [], 'Irecv': [], 'Waitall': []}
    for m in metrics_list:
        for k in combined:
            combined[k].extend(m['events'][k])

    return {
        'first_begin': first_begin,
        'last_end': last_end,
        'duration': duration,
        'events': combined,
    }


def fmt_avg(lst):
    if not lst:
        return '—'
    return f"{sum(d for _, _, d in lst) / len(lst):.1f}"


def print_report(combined, trace_type, size_label):
    print(f"\n{'=' * 60}")
    print(f"Размер: {size_label}")
    print(f"Тип:    {'блокирующая (Send/Recv)' if trace_type == 'blocking' else 'неблокирующая (Isend/Irecv)'}")
    print(f"{'=' * 60}")
    print(f"Первая передача BEGIN:   {combined['first_begin']} мкс")
    print(f"Конец последней:         {combined['last_end']} мкс")
    print(f"Длительность:            {combined['duration']} мкс")
    print()
    if trace_type == 'blocking':
        print(f"Send:    count={len(combined['events']['Send']):4d}  "
              f"avg={fmt_avg(combined['events']['Send']):>10s}  "
              f"total={sum(d for _, _, d in combined['events']['Send'])}")
        print(f"Recv:    count={len(combined['events']['Recv']):4d}  "
              f"avg={fmt_avg(combined['events']['Recv']):>10s}  "
              f"total={sum(d for _, _, d in combined['events']['Recv'])}")
    else:
        print(f"Isend:   count={len(combined['events']['Isend']):4d}  "
              f"avg={fmt_avg(combined['events']['Isend']):>10s}  "
              f"total={sum(d for _, _, d in combined['events']['Isend'])}")
        print(f"Irecv:   count={len(combined['events']['Irecv']):4d}  "
              f"avg={fmt_avg(combined['events']['Irecv']):>10s}  "
              f"total={sum(d for _, _, d in combined['events']['Irecv'])}")
        print(f"Waitall: count={len(combined['events']['Waitall']):4d}  "
              f"avg={fmt_avg(combined['events']['Waitall']):>10s}  "
              f"total={sum(d for _, _, d in combined['events']['Waitall'])}")


def main():
    parser = argparse.ArgumentParser(description='Парсер трассировок MPI')
    parser.add_argument('files', nargs='+', help='Файлы трассировки')
    parser.add_argument('--size', default='?', help='Метка размера, например "4 мб"')
    parser.add_argument('--type', choices=['auto', 'blocking', 'nonblocking'],
                        default='auto', help='Тип версии (по умолчанию авто)')
    parser.add_argument('--csv', action='store_true', help='Вывести CSV-строку')
    parser.add_argument('--tsv', action='store_true', help='Вывести TSV-строку')
    args = parser.parse_args()

    all_metrics = []
    detected_types = set()
    for path in args.files:
        if not os.path.exists(path):
            print(f"Ошибка: файл не найден: {path}", file=sys.stderr)
            sys.exit(1)
        events = parse_file(path)
        t = detect_type(events)
        detected_types.add(t)
        all_metrics.append(compute_metrics(events, t))

    if args.type == 'auto':
        if 'mixed' in detected_types or len(detected_types) > 1:
            print(f"Предупреждение: неоднозначный тип {detected_types}, использую 'nonblocking'",
                  file=sys.stderr)
            trace_type = 'nonblocking'
        else:
            trace_type = detected_types.pop()
    else:
        trace_type = args.type

    combined = combine_metrics(all_metrics)

    if args.csv or args.tsv:
        sep = ',' if args.csv else '\t'
        algorithm = 'Send/Recv' if trace_type == 'blocking' else 'Isend/Irecv'
        if trace_type == 'blocking':
            avg_send = fmt_avg(combined['events']['Send'])
            avg_recv = fmt_avg(combined['events']['Recv'])
            avg_wait = '—'
        else:
            avg_send = fmt_avg(combined['events']['Isend'])
            avg_recv = fmt_avg(combined['events']['Irecv'])
            avg_wait = fmt_avg(combined['events']['Waitall'])
        print(sep.join([
            algorithm, args.size,
            str(combined['first_begin']),
            str(combined['last_end']),
            str(combined['duration']),
            avg_send, avg_recv, avg_wait
        ]))
    else:
        print_report(combined, trace_type, args.size)


if __name__ == '__main__':
    main()