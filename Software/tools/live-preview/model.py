from pipe import Pipe

import joblib

import tensorflow as tf
from tensorflow import keras


class Model(Pipe):
    def __init__(self, *args, **kwargs):
        self.ready = False
        self.result = None
        self.load(*args, **kwargs)
        
    def load(self, *args, **kwargs):
        self.model = None

    def predict(self, data):
        return []

    def feed(self, data):
        self.result = self.predict(data)
        self.ready = True
        return True

    def retrieve(self):
        self.ready = False
        return self.result
        


# joblib
class SklearnModel(Model):
    def load(self, path):
        self.model = joblib.load(path)

    def predict(self, features):
        return self.model.predict([features])


class TensorFlowModel(Model):
    def load(self, path, threshold=0.5):
        self.model = keras.models.load_model(path)
        self.threshold = threshold

    def predict(self, features):
        # print(self.model.predict([features]))
        return self.model.predict([features])[0] > self.threshold
