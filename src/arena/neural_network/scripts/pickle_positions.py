import csv


def set_one(value):
    if value == 0:
        return b'0' * 5
    result = [b'0'] * 5
    result[value - 1] = b'1'
    return b''.join(result)


def convert(position) -> (bytes, bytes):
    if len(position) != 8 or sum(map(len, position)) != 190:
        return b'', b''
    try:
        float(position[6])
    except ValueError:
        return b'', b''
    cp_is_p1 = position[4] == '0'
    result = b''
    bb_iter = zip(position[2], position[3]) if cp_is_p1 else zip(position[3], position[2])
    for i, (cp_bb, op_bb) in enumerate(bb_iter):
        cp_set, op_set = cp_bb == '1', op_bb == '1'
        if cp_set or op_set:
            value = 5 if cp_set and op_set else 3 if cp_set else 4
            result += set_one(value) * 9
            continue
        sb_iter = (zip(position[0][9*i:9*(i+1)], position[1][9*i:9*(i+1)]) if cp_is_p1
                   else zip(position[1][9*i:9*(i+1)], position[0][9*i:9*(i+1)]))
        for cp_sb, op_sb in sb_iter:
            value = 1 if cp_sb == '1' else 2 if op_sb == '1' else 0
            result += set_one(value)
    current_board = [b'0'] * 10
    current_board[int(position[5])] = b'1'
    result += b''.join(current_board) + b'0'
    b = bytes(int(result[i:i+8], 2) for i in range(0, len(result), 8))
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
