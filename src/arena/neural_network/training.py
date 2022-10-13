import csv
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
import matplotlib.pyplot as plt
from neural_network.network import NeuralNetwork


class PositionDataset(Dataset):
    TRAINING_DATA_PERCENTAGE = 0.8

    def __init__(self, filename: str, is_training_dataset: bool, transform, target_transform):
        with open(filename, 'r') as f:
            self.positions = list(csv.DictReader(f, delimiter=','))
        self.is_training_dataset = is_training_dataset
        self.transform = transform
        self.target_transform = target_transform

    def __len__(self):
        return int(len(self.positions) * (self.TRAINING_DATA_PERCENTAGE if self.is_training_dataset else
                                          1 - self.TRAINING_DATA_PERCENTAGE))

    def __getitem__(self, index):
        if not self.is_training_dataset:
            index += int(len(self.positions) * self.TRAINING_DATA_PERCENTAGE)
        positions = self.positions[index]
        return self.transform(positions), self.target_transform(positions)


def transform_inputs(position: dict[str, str]) -> torch.Tensor:
    current_board = ['0'] * 10
    current_board[int(position['current_board'])] = '1'
    current_board = ''.join(current_board)
    if position['current_player'] == '0':
        X = list(map(int, position['p1_big_boards'] + position['p1_small_boards'] + position['p2_big_boards']
                     + position['p2_small_boards'] + current_board))
    else:
        X = list(map(int, position['p2_big_boards'] + position['p2_small_boards'] + position['p1_big_boards']
                     + position['p1_small_boards'] + current_board))
    return torch.tensor(X, dtype=torch.float32).to(device)


def eval_target_transform(position: dict[str, str]) -> torch.Tensor:
    return torch.tensor([float(position['current_player_eval'])], dtype=torch.float32).to(device)


def train_loop(dataloader, model, loss_fn, optimizer, append):
    size = len(dataloader.dataset)
    train_loss = 0
    for i, (X, y) in enumerate(dataloader):
        pred = model(X)
        loss = loss_fn(pred, y)
        train_loss += loss.item()
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        if i % 100 == 0:
            loss, current = loss.item(), i * len(X)
            print(f'loss: {loss:>7f} [{current:>5d}/{size:>5d}]')
    num_batches = len(dataloader)
    train_loss /= num_batches
    append(train_loss)


def test_loop(dataloader, model, loss_fn, append):
    test_loss = 0
    with torch.no_grad():
        for X, y in dataloader:
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
    num_batches = len(dataloader)
    test_loss /= num_batches
    append(test_loss)
    print(f'Test average loss: {test_loss:>8f}\n')


def main():
    learning_rate = 1e-3
    batch_size = 64
    epochs = 100

    training_data = PositionDataset('../positions.csv', True, transform_inputs, eval_target_transform)
    testing_data = PositionDataset('../positions.csv', False, transform_inputs, eval_target_transform)
    train_dataloader = DataLoader(training_data, batch_size=batch_size, shuffle=True)
    test_dataloader = DataLoader(testing_data, batch_size=batch_size, shuffle=True)
    model = NeuralNetwork().to(device)
    loss_fn = nn.MSELoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate)
    train_losses, test_losses = [], []
    for i in range(epochs):
        print(f'Epoch {i+1}\n-------------------------------')
        train_loop(train_dataloader, model, loss_fn, optimizer, train_losses.append)
        test_loop(test_dataloader, model, loss_fn, test_losses.append)
    torch.save(model, 'model.pth')
    plt.plot(train_losses, label='Training Loss')
    plt.plot(test_losses, label='Testing Loss')
    plt.legend()
    plt.show()
    print('Done!')


if __name__ == '__main__':
    # device = 'cuda' if torch.cuda.is_available() else 'cpu'
    device = 'cpu'
    main()
