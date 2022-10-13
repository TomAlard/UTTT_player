import os


def main():
    all_positions = []
    for _, _, files in os.walk("../games"):
        for filename in files:
            with open(f'../games/{filename}', 'r') as f:
                contents = f.read()
                lines = contents.splitlines()
                if len(lines) > 2 and len(lines[-2]) == 1:
                    winner = contents[-3]
                    positions = [f'{line},{winner}' for line in lines if len(line) > 10]
                    all_positions.extend(positions[:-1])
    with open('../positions.csv', 'w') as f:
        f.write(','.join(('p1_small_boards', 'p2_small_boards', 'p1_big_boards', 'p2_big_boards', 'current_player',
                          'current_board', 'current_player_eval', 'move_played', 'winner')) + '\n')
        for position in all_positions:
            f.write(position + '\n')


if __name__ == '__main__':
    main()
