import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
import numpy as np
from time import time
from neural_network.network import NeuralNetwork


class PositionDataset(Dataset):
    LINE_SIZE = 30
    SPLIT_INDEX = 24

    def __init__(self, filename: str):
        with open(filename, 'rb') as f:
            f.seek(0, 2)
            self.amount_of_lines = f.tell() // self.LINE_SIZE
            f.seek(0)
            self.data = f.read()

    def __len__(self):
        return self.amount_of_lines

    def __getitem__(self, index):
        pos = index * self.LINE_SIZE
        position = self.data[pos:pos+self.SPLIT_INDEX]
        evaluation = self.data[pos+self.SPLIT_INDEX:pos+self.LINE_SIZE]
        X = np.frombuffer(position, dtype=np.uint8)
        X = np.unpackbits(X)[:190]
        X = torch.from_numpy(X).to(torch.float32)
        y = torch.tensor([float(evaluation) - 0.5], dtype=torch.float32)
        return X, y


def train_loop(dataloader, model, loss_fn, optimizer, scheduler):
    size = len(dataloader.dataset)
    train_loss = 0
    for i, (X, y) in enumerate(dataloader):
        X, y = X.cuda(), y.cuda()
        pred = model(X)
        loss = loss_fn(pred, y)
        train_loss += loss.item()
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        if i % 10_000 == 0:
            loss, current = loss.item(), i * len(X)
            print(f'loss: {loss:>7f} [{current:>7d}/{size:>7d}]')
    num_batches = len(dataloader)
    train_loss /= num_batches
    scheduler.step(train_loss)
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
    learning_rate = 0.001
    batch_size = 1024
    epochs = 1000

    training_data = PositionDataset('../train_positions2.csv')
    testing_data = PositionDataset('../test_positions2.csv')
    train_dataloader = DataLoader(training_data, batch_size=batch_size, num_workers=4, persistent_workers=True,
                                  pin_memory=True)
    test_dataloader = DataLoader(testing_data, batch_size=batch_size)
    model = torch.load('model_latest.pth')
    loss_fn = nn.MSELoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate, momentum=0.9, nesterov=True)
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(optimizer, patience=3)
    for i in range(epochs):
        print(f'Epoch {i+1}\n-------------------------------')
        start = time()
        train_loop(train_dataloader, model, loss_fn, optimizer, scheduler)
        test_loop(test_dataloader, model, loss_fn)
        print(f'Epoch completed in {time() - start:<7f}\n')
        torch.save(model, 'model_latest.pth')


if __name__ == '__main__':
    main()
