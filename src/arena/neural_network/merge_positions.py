import os
import csv
import random

PATH = 'D:/new_positions'


def merge():
    all_positions = []
    for _, _, files in os.walk(PATH):
        for filename in files:
            with open(f'{PATH}/{filename}', 'r') as f:
                contents = f.read()
                lines = contents.splitlines()
                if len(lines) > 2:
                    all_positions.extend(lines)
    with open('../positions.csv', 'w') as f:
        f.write(','.join(('p1_small_boards', 'p2_small_boards', 'p1_big_boards', 'p2_big_boards', 'current_player',
                          'current_board', 'current_player_eval', 'move_played')) + '\n')
        for position in all_positions:
            f.write(position + '\n')


def remove_duplicates():
    seen = set()
    with open('../positions.csv', 'r') as read_file:
        reader = csv.reader(read_file, delimiter=',')
        with open('../unique_positions.csv', 'w') as write_file:
            write_file.write(','.join(next(reader)) + '\n')  # Write header
            for row in reader:
                key = ','.join(row[:-1])
                if key not in seen:
                    seen.add(key)
                    write_file.write(','.join(row) + '\n')


def shuffle():
    with open('../positions.csv', 'r') as f:
        lines = f.readlines()
    header = lines[0]
    lines = lines[1:]
    random.shuffle(lines)
    with open('../positions.csv', 'w') as f:
        f.write(header)
        f.writelines(lines)


def split(train_split):
    with open('../positions.csv', 'r') as f:
        lines = f.readlines()
    header = lines[0]
    lines = lines[1:]
    index = int(len(lines) * train_split)
    with open('../train_positions.csv', 'w') as f:
        f.write(header)
        f.writelines(lines[:index])
    with open('../test_positions.csv', 'w') as f:
        f.write(header)
        f.writelines(lines[index:])


if __name__ == '__main__':
    merge()
    shuffle()
    split(0.99)
