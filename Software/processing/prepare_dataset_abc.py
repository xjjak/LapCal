from abc import ABC, abstractmethod


class DataPreparation(ABC):
    """Classes that process collected data into a dataset useful for a
    specific purpose."""

    @abstractmethod
    def get_name(self):
        ...

    def __call__(self, *args, **kwargs):
        return self.prepare_dataset(*args, **kwargs)

    @abstractmethod
    def prepare_dataset(self):
        ...
