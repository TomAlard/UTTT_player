import os
import random

PATH = '../new_positions'
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
    with open('../positions.csv', 'w') as f:
        f.write(','.join((
            'p1_small_boards', 'p2_small_boards', 'p1_big_boards', 'p2_big_boards', 'current_player',
            'current_board', 'current_player_eval', 'move_played')) + '\n')
        write_positions(PATH, f)


def split(train_split):
    with open('../positions.csv', 'r') as f:
        f.seek(0, 2)
        num_lines = f.tell() // LINE_SIZE
        f.seek(0)
        header = f.readline()
        index = int(num_lines * (1 - train_split))
        with open('../test_positions.csv', 'w') as write_file:
            write_file.write(header)
            for i in range(index):
                write_file.write(f.readline())
        with open('../train_positions.csv', 'w') as write_file:
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
