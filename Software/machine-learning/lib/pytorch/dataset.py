from torch.utils.data import DataLoader, Dataset

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
