import csv


def convert(position) -> bytes:
    current_board = ['0'] * 10
    current_board[int(position[5])] = '1'
    current_board = ''.join(current_board)
    if position[4] == '0':
        X = list(position[2] + position[0] + position[3] + position[1] + current_board)
    else:
        X = list(position[3] + position[1] + position[2] + position[0] + current_board)
    b = b''.join(b'\x00' if x == '0' else b'\x01' for x in ''.join(X))
    return b + bytes(position[6], encoding='UTF-8')


def convert_positions(filename):
    with open(filename, 'r') as read_file:
        reader = csv.reader(read_file, delimiter=',')
        next(reader)
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
    main()
