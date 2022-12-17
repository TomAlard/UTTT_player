import csv


def convert(position) -> bytes:
    if len(position) != 8 or sum(map(len, position)) != 190:
        return b''
    try:
        float(position[6])
    except ValueError:
        return b''
    current_board = ['0'] * 10
    current_board[int(position[5])] = '1'
    current_board = ''.join(current_board)
    if position[4] == '0':
        X = position[2] + position[0] + position[3] + position[1] + current_board + '00'
    else:
        X = position[3] + position[1] + position[2] + position[0] + current_board + '00'
    b = bytes(int(X[i:i+8], 2) for i in range(0, len(X), 8))
    return b + bytes(position[6], encoding='UTF-8')


def convert_positions(filename):
    with open(filename, 'r') as read_file:
        reader = csv.reader(read_file, delimiter=',')
        next(reader)
        converted = map(convert, reader)
        with open(filename[:-4] + '2.csv', 'wb') as write_file:
            i = 0
            for position in converted:
                if position != b'':
                    write_file.write(position)
                if i % 100_000 == 0:
                    print(f'{i:>7d}/?')
                i += 1


def main():
    convert_positions('../train_positions.csv')
    convert_positions('../test_positions.csv')


if __name__ == '__main__':
    main()
