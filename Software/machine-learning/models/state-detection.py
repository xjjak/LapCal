# support imports from the ../modules/ directory
import os
import sys
from pathlib import Path
sys.path.append(os.environ["LAPCAL_LIBS"])

DATADIR = Path(os.environ["LAPCAL_DATA_DIR"])
# Dataset of type 
DATASET = DATADIR / "somefile.joblib" # FIXME

import torch
from torch import nn
from torch.utils.data import DataLoader, Dataset
from torchvision import datasets
from torchvision.transforms import ToTensor



import numpy as np
import joblib


# TODO: to tensor???
# TODO: to module
class LapcalDataset(Dataset):
    def __init__(self, dataset_file, mode="train", transform=None, target_transform=None):
        if mode == "train":
            self.x, self.y, _, _, _, _ = joblib.load(dataset_file)
        elif mode == "dev":
            _, _, self.x, self.y, _, _ = joblib.load(dataset_file)
        elif mode == "test":
            _, _, _, _, self.x, self.y = joblib.load(dataset_file)
        else:
            raise Exception("Unknown mode.")
            
        self.transform = transform
        self.target_transform = target_transform

    def __len__(self):
        return len(self.x)

    def __getitem__(self, idx):
        features = self.x[idx]
        label = self.y[idx]
        if self.transform:
            features = self.transform(features)
        if self.target_transform:
            label = self.target_transform(label)
        return features, label


# Declaring datasets.
training_data = LapcalDataset(DATASET, mode="train")
dev_data      = LapcalDataset(DATASET, mode="dev")
test_data     = LapcalDataset(DATASET, mode="test")


batch_size = 64

# Create data loaders.
train_dataloader = DataLoader(training_data, batch_size=batch_size)
dev_dataloader   = DataLoader(dev_data,      batch_size=batch_size)
test_dataloader  = DataLoader(test_data,     batch_size=batch_size)


# Get cpu, gpu or mps device for training.
device = (
    "cuda"
    if torch.cuda.is_available()
    else "mps"
    if torch.backends.mps.is_available()
    else "cpu"
)
print(f"Using {device} device")


# Define model.
# TODO: flatten? -> readings as list
class NeuralNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.flatten = nn.Flatten()
        self.stack = nn.Sequential(
            nn.Linear(3*6*9 + 17, 512),
            nn.ReLU(),
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 17)
            nn.Sigmoid()
        )

    def forward(self, x):
        x = self.flatten(x)
        logits = self.stack(x)
        return logits

model = NeuralNetwork().to(device)
print(model)


# Optimization.
loss_fn = nn.BCELoss()
optimizer = torch.optim.SGD(model.parameters(), lr=1e-3) # RMSProp
# - https://pytorch.org/docs/stable/generated/torch.optim.SGD.html#torch.optim.SGD
# - https://pytorch.org/docs/stable/generated/torch.optim.RMSprop.html#torch.optim.RMSprop


# TODO: integrate custom metric

# TODO: put elsewhere?
def train(dataloader, model, loss_fn, optimizer):
    size = len(dataloader.dataset)
    model.train()
    for batch, (X, y) in enumerate(dataloader):
        X, y = X.to(device), y.to(device)

        # Compute prediction error
        pred = model(X)
        loss = loss_fn(pred, y)

        # Backpropagation
        loss.backward()
        optimizer.step()
        optimizer.zero_grad()

        if batch % 100 == 0:
            loss, current = loss.item(), (batch + 1) * len(X)
            print(f"loss: {loss:>7f}  [{current:>5d}/{size:>5d}]")

            
def test(dataloader, model, loss_fn):
    size = len(dataloader.dataset)
    num_batches = len(dataloader)
    model.eval()
    test_loss, correct = 0, 0
    with torch.no_grad():
        for X, y in dataloader:
            X, y = X.to(device), y.to(device)
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
            correct += (pred.argmax(1) == y).type(torch.float).sum().item()
    test_loss /= num_batches
    correct /= size
    print(f"Test Error: \n Accuracy: {(100*correct):>0.1f}%, Avg loss: {test_loss:>8f} \n")


epochs = 5
for t in range(epochs):
    print(f"Epoch {t+1}\n-------------------------------")
    train(train_dataloader, model, loss_fn, optimizer)
    test(dev_dataloader, model, loss_fn)
print("Done!")



# TODO: matplotlib visualization capabilities

