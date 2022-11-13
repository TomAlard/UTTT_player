import csv


def convert(position):
    add = 0 if position['current_player'] == '0' else 9
    current_board = int(position['current_board'])
    result = b''
    for i, (p1_bb, p2_bb) in enumerate(zip(position['p1_big_boards'], position['p2_big_boards'])):
        p1_set, p2_set = p1_bb == '1', p2_bb == '1'
        if p1_set or p2_set:
            value = 8 if p1_set and p2_set else 6 if p1_set else 7
            result += bytes(chr(ord('a') + value+add) * 9, encoding='UTF-8')
            continue
        for p1_sb, p2_sb in zip(position['p1_small_boards'][9*i:9*(i+1)], position['p2_small_boards'][9*i:9*(i+1)]):
            value = 1 if p1_sb == '1' else 2 if p2_sb == '1' else 0
            if i == current_board or current_board == 9:
                value += 3
            result += bytes(chr(ord('a') + value+add), encoding='UTF-8')
    result += bytes(position['current_player_eval'], encoding='UTF-8')
    return result


def convert_positions(filename):
    with open(filename, 'r') as read_file:
        reader = csv.DictReader(read_file, delimiter=',')
        converted = map(convert, reader)
        with open(filename[:-4] + '2.csv', 'wb') as write_file:
            i = 0
            for position in converted:
                write_file.write(position + b'\n')
                if i % 100_000 == 0:
                    print(f'{i:>7d}/5366060')
                i += 1


def main():
    convert_positions('../train_positions.csv')
    convert_positions('../test_positions.csv')


if __name__ == '__main__':
    with open('../test_positions2.csv', 'rb') as f:
        f.seek(0, 2)
        print(f.tell())
    # main()
