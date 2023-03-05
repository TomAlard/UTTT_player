import csv


def convert(position) -> (bytes, bytes):
    if len(position) != 8 or sum(map(len, position)) != 190:
        return b'', b''
    try:
        float(position[6])
    except ValueError:
        return b'', b''
    current_board = ['0'] * 10
    current_board[int(position[5])] = '1'
    current_board = ''.join(current_board)
    cp_is_p1 = position[4] == '0'
    cp_bb, op_bb = (position[2], position[3]) if cp_is_p1 else (position[3], position[2])
    cp_sbs, op_sbs = (position[0], position[1]) if cp_is_p1 else (position[1], position[0])
    X = cp_bb
    for i in range(9):
        X += '1'*9 if cp_bb[i] == '1' else '0'*9 if op_bb[i] == '1' else cp_sbs[i*9:(i+1)*9]
    X += op_bb
    for i in range(9):
        X += '1'*9 if op_bb[i] == '1' else '0'*9 if cp_bb[i] == '1' else op_sbs[i*9:(i+1)*9]
    X += current_board + '00'
    b = bytes(int(X[i:i+8], 2) for i in range(0, len(X), 8))
    return b, bytes(position[6], encoding='UTF-8')


def convert_positions(filename, new_filename):
    with open(filename, 'r') as read_file:
        reader = csv.reader(read_file, delimiter=',')
        next(reader)
        converted = map(convert, reader)
        with open(f'../{new_filename}_compressed_positions.csv', 'wb') as positions_file:
            with open(f'../{new_filename}_compressed_evaluations.csv', 'wb') as evaluations_file:
                i = 0
                for position, evaluation in converted:
                    if position != b'' and evaluation != b'':
                        positions_file.write(position)
                        evaluations_file.write(evaluation)
                    if i % 1_000_000 == 0:
                        print(f'{i:>7d}/?')
                    i += 1


def main():
    convert_positions('../train_positions.csv', 'train')
    convert_positions('../test_positions.csv', 'test')


if __name__ == '__main__':
    main()
