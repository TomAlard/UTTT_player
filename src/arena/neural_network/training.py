import csv
import random
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
import matplotlib.pyplot as plt
from neural_network.network import NeuralNetwork


class PositionDataset(Dataset):
    def __init__(self, filename: str, transform, target_transform):
        with open(filename, 'r') as f:
            self.positions = list(csv.DictReader(f, delimiter=','))
        self.transform = transform
        self.target_transform = target_transform

    def __len__(self):
        return len(self.positions)

    def __getitem__(self, index):
        position = self.positions[index]
        return self.transform(position), self.target_transform(position)


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
    player_eval = float(position['current_player_eval'])
    return torch.tensor([player_eval - 0.5], dtype=torch.float32).to(device)


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
            print(f'loss: {loss:>7f} [{current:>6d}/{size:>6d}]')
    num_batches = len(dataloader)
    train_loss /= num_batches
    print(f'Train average loss: {train_loss:>8f}\n')
    append(train_loss)


def test_loop(dataloader, model, loss_fn, scheduler, append):
    test_loss = 0
    with torch.no_grad():
        for X, y in dataloader:
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
    num_batches = len(dataloader)
    test_loss /= num_batches
    append(test_loss)
    scheduler.step(test_loss)
    print(f'Test average loss: {test_loss:>8f}\n')
    return test_loss


def test_actual_loss():
    testing_data = PositionDataset('../test_positions.csv', transform_inputs, eval_target_transform)
    model = torch.load('model2.pth')
    model.eval()
    num_samples = 1000
    error = 0
    values = []
    for _ in range(num_samples):
        inputs, label = testing_data[random.randrange(len(testing_data))]
        inputs = inputs.unsqueeze(0)
        values.append(model(inputs).item())
        error += abs(label.item() - model(inputs).item())
    print('Average error:', error / num_samples)


def main():
    learning_rate = 1e-1
    batch_size = 64
    epochs = 1000

    training_data = PositionDataset('../train_positions.csv', transform_inputs, eval_target_transform)
    testing_data = PositionDataset('../test_positions.csv', transform_inputs, eval_target_transform)
    train_dataloader = DataLoader(training_data, batch_size=batch_size, shuffle=True)
    test_dataloader = DataLoader(testing_data, batch_size=batch_size, shuffle=True)
    model = NeuralNetwork().to(device)
    loss_fn = nn.MSELoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate, momentum=0.9)
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(optimizer, factor=0.75)
    train_losses, test_losses = [], []
    best_loss = 1
    for i in range(epochs):
        print(f'Epoch {i+1}\n-------------------------------')
        train_loop(train_dataloader, model, loss_fn, optimizer, train_losses.append)
        loss = test_loop(test_dataloader, model, loss_fn, scheduler, test_losses.append)
        if loss < best_loss:
            best_loss = loss
            torch.save(model, 'model2.pth')
        if (i % 25 == 0 and i != 0) or i+1 == epochs:
            plt.plot(train_losses, label='Training Loss')
            plt.plot(test_losses, label='Testing Loss')
            plt.xlabel('Epochs')
            plt.ylabel('Loss')
            plt.legend()
            plt.show()
    print('Done!')


if __name__ == '__main__':
    # device = 'cuda' if torch.cuda.is_available() else 'cpu'
    device = 'cpu'
    test_actual_loss()
    # main()
