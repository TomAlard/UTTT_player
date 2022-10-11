import os


def main():
    all_positions = []
    for _, _, files in os.walk("./games"):
        for filename in files:
            with open(f'./games/{filename}', 'r') as f:
                contents = f.read()
                winner = contents[-3]
                lines = contents.splitlines()
                positions = [f'{line},{winner}' for line in lines if len(line) > 10]
                all_positions.extend(positions)
    with open('positions.csv', 'w') as f:
        for position in all_positions:
            f.write(position + '\n')


if __name__ == '__main__':
    main()
