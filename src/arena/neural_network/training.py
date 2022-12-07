import time
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
from network import NeuralNetwork


class PositionDataset(Dataset):
    LINE_SIZE = 88

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
        position = self.data[pos:pos+81]
        evaluation = self.data[pos+81:pos+88]
        return transform_inputs(position), eval_target_transform(evaluation)


def transform_inputs(position: bytes) -> torch.Tensor:
    result = torch.zeros(1458, dtype=torch.float32)
    indices = torch.arange(0, 1441, 18)
    indices += torch.frombuffer(position, dtype=torch.uint8) - 97
    result[indices] += 1
    return result


def eval_target_transform(evaluation: bytes) -> torch.Tensor:
    player_eval = float(evaluation)
    return torch.tensor([player_eval - 0.5], dtype=torch.float32)


def train_loop(dataloader, model, loss_fn, optimizer):
    size = len(dataloader.dataset)
    train_loss = 0
    start = time.time()
    for i, (X, y) in enumerate(dataloader):
        X, y = X.cuda(), y.cuda()
        pred = model(X)
        loss = loss_fn(pred, y)
        train_loss += loss.item()
        optimizer.zero_grad(set_to_none=True)
        loss.backward()
        optimizer.step()
        if i % 1000 == 0:
            loss, current = loss.item(), i * len(X)
            print(f'loss: {loss:>7f} [{current:>7d}/{size:>7d}], time: {time.time() - start:<7f}')
    num_batches = len(dataloader)
    train_loss /= num_batches
    print(f'Train average loss: {train_loss:>8f}\n')


def test_loop(dataloader, model, loss_fn, scheduler):
    test_loss = 0
    with torch.no_grad():
        for X, y in dataloader:
            X, y = X.cuda(), y.cuda()
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
    num_batches = len(dataloader)
    test_loss /= num_batches
    scheduler.step(test_loss)
    print(f'Test average loss: {test_loss:>8f}\n')
    return test_loss


def test_actual_loss():
    testing_data = PositionDataset('../test_positions2.csv')
    model = torch.load('model_latest_NNUE_1024_batch.pth').cpu()
    model.eval()
    error = 0
    values = []
    for inputs, label in testing_data:
        inputs = inputs.unsqueeze(0)
        values.append(model(inputs).item())
        error += abs(label.item() - model(inputs).item())
        print(label.item() + 0.5, model(inputs).item() + 0.5)
    print('Average error:', error / len(testing_data))


def main():
    learning_rate = 0.1
    batch_size = 1024
    epochs = 1000

    training_data = PositionDataset('../train_positions2.csv')
    testing_data = PositionDataset('../test_positions2.csv')
    train_dataloader = DataLoader(training_data, batch_size=batch_size, num_workers=8, persistent_workers=True,
                                  pin_memory=True, shuffle=True)
    test_dataloader = DataLoader(testing_data, batch_size=batch_size)
    model = NeuralNetwork().cuda()
    loss_fn = nn.L1Loss()
    optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate, momentum=0.9, nesterov=True)
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(optimizer)
    best_loss = 1
    for i in range(epochs):
        print(f'Epoch {i+1}\n-------------------------------')
        train_loop(train_dataloader, model, loss_fn, optimizer)
        loss = test_loop(test_dataloader, model, loss_fn, scheduler)
        if loss < best_loss:
            best_loss = loss
            # torch.save(model, 'model_best_NNUE.pth')
        torch.save(model, 'model_latest_1024_batch.pth')


if __name__ == '__main__':
    main()
