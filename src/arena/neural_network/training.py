import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
import numpy as np
from time import time
from neural_network.network import NeuralNetwork, WeightClipper


class PositionDataset(Dataset):
    POSITIONS_LINE_LENGTH = 24
    EVALUATIONS_LINE_LENGTH = 6

    def __init__(self, is_train: bool):
        base_filename = f"../{'train' if is_train else 'test'}_compressed"
        positions_filename = f'{base_filename}_positions.csv'
        evaluations_filename = f'{base_filename}_evaluations.csv'
        with open(positions_filename, 'rb') as f:
            f.seek(0, 2)
            self.amount_of_lines = f.tell() // self.POSITIONS_LINE_LENGTH
            f.seek(0)
            self.positions = np.fromfile(f, dtype=np.uint8).reshape((self.amount_of_lines, self.POSITIONS_LINE_LENGTH))
            self.positions = torch.from_numpy(self.positions)
        with open(evaluations_filename, 'rb') as f:
            evals = []
            while e := f.read(self.EVALUATIONS_LINE_LENGTH):
                evals.append(float(e) - 0.5)
            self.evaluations = np.array(evals, dtype=np.float32)
            self.evaluations = torch.from_numpy(self.evaluations)

    def __len__(self):
        return self.amount_of_lines

    def __getitem__(self, index):
        X = np.unpackbits(self.positions[index])[:190].astype(np.float32)
        y = self.evaluations[index].reshape((1,))
        return X, y


def train_loop(dataloader, model, loss_fn, optimizer):
    size = len(dataloader.dataset)
    train_loss = 0
    clipper = WeightClipper()
    for i, (X, y) in enumerate(dataloader):
        X, y = X.cuda(), y.cuda()
        pred = model(X)
        loss = loss_fn(pred, y)
        train_loss += loss.item()
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        if i % clipper.frequency == 0:
            model.apply(clipper)
        if i % 10_000 == 0:
            loss, current = loss.item(), i * len(X)
            print(f'loss: {loss:>7f} [{current:>7d}/{size:>7d}]')
    num_batches = len(dataloader)
    train_loss /= num_batches
    print(f'Train average loss: {train_loss:>8f}')


def test_loop(dataloader, model, loss_fn):
    test_loss = 0
    with torch.no_grad():
        for X, y in dataloader:
            X, y = X.cuda(), y.cuda()
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
    num_batches = len(dataloader)
    test_loss /= num_batches
    print(f'Test average loss: {test_loss:>8f}')


def main():
    learning_rate = 0.1
    batch_size = 1024
    epochs = 1000

    training_data = PositionDataset(True)
    testing_data = PositionDataset(False)
    train_dataloader = DataLoader(training_data, batch_size=batch_size, num_workers=8, persistent_workers=True,
                                  pin_memory=True)
    test_dataloader = DataLoader(testing_data, batch_size=batch_size)
    model = NeuralNetwork().cuda()
    loss_fn = nn.MSELoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate, momentum=0.9, nesterov=True)
    for i in range(epochs):
        print(f'Epoch {i+1}\n-------------------------------')
        start = time()
        train_loop(train_dataloader, model, loss_fn, optimizer)
        test_loop(test_dataloader, model, loss_fn)
        print(f'Epoch completed in {time() - start:<7f}\n')
        torch.save(model, 'model_latest.pth')


if __name__ == '__main__':
    main()
