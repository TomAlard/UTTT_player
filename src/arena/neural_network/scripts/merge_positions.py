import os
import csv
import random

PATH = '../positions_jacekmax_100ms'
LINE_SIZE = 197


def write_positions(path, write_file):
    for _, _, files in os.walk(path):
        for filename in files:
            print(filename)
            with open(f'{path}/{filename}', 'r') as f:
                lines = f.readlines()
                random.shuffle(lines)
                write_file.writelines(lines)
                write_file.write('\n')


def merge():
    with open('../../positions.csv', 'w') as f:
        f.write(','.join((
            'p1_small_boards', 'p2_small_boards', 'p1_big_boards', 'p2_big_boards', 'current_player',
            'current_board', 'current_player_eval', 'move_played')) + '\n')
        write_positions(PATH, f)


def remove_duplicates():
    seen = set()
    with open('../../positions.csv', 'r') as read_file:
        reader = csv.reader(read_file, delimiter=',')
        with open('../../unique_positions.csv', 'w') as write_file:
            write_file.write(','.join(next(reader)) + '\n')  # Write header
            for row in reader:
                key = ','.join(row[:-1])
                if key not in seen:
                    seen.add(key)
                    write_file.write(','.join(row) + '\n')


def shuffle():
    with open('../../positions.csv', 'r') as f:
        lines = f.readlines()
    header = lines[0]
    lines = lines[1:]
    indices = list(range(len(lines)))
    random.shuffle(indices)
    with open('../../positions.csv', 'w') as f:
        f.write(header)
        f.writelines(lines)


def split(train_split):
    with open('../../positions.csv', 'r') as f:
        f.seek(0, 2)
        num_lines = f.tell() // LINE_SIZE
        f.seek(0)
        header = f.readline()
        index = int(num_lines * (1 - train_split))
        with open('../../test_positions.csv', 'w') as write_file:
            write_file.write(header)
            for i in range(index):
                write_file.write(f.readline())
        with open('../../train_positions.csv', 'w') as write_file:
            write_file.write(header)
            i = 0
            while e := f.readline():
                write_file.write(e)
                i += 1
                if i % 1_000_000 == 0:
                    print(f'{i}/{int(num_lines * train_split)}')


if __name__ == '__main__':
    print('merge')
    merge()
    print('split')
    split(0.99)
