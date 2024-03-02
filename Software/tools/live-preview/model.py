import joblib
import tensorflow as tf
from tensorflow import keras

class Model:
    def __init__(self):
        self.model = None

    def predict(self, features):
        return []


# joblib
class SklearnModel:
    def __init__(self, path):
        self.model = joblib.load(path)

    def predict(self, features):
        return self.model.predict([features])


class TensorFlowModel:
    def __init__(self, path):
        self.model = keras.models.load_model(path)

    def predict(self, features):
        # print(self.model.predict([features]))
        return self.model.predict([features])[0] > 0.01
