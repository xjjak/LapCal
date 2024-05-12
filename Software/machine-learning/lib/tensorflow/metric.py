"""
This module provides some metrics as functions including a custom metric designed to be used as the main metric used in the machine learning part of this project.

The module provides the following functions:

    * recall - calculates the recall metric
    * precision - calculates the precision metric
    * fbeta - calculates the fbeta metric
    * inv_distance - calculates a metric that increases with increase
                     in difference to prediction
    * custom_metric - calculates a metric

"""

import keras.backend as K
from keras.metrics import binary_accuracy

from functools import partial

def recall(y_true, y_pred):
    """
    Calculate the recall metric (https://en.wikipedia.org/wiki/Precision_and_recall).

    Parameters
    ----------
    """
    true_positives = K.sum(K.round(K.clip(y_true * y_pred, 0, 1)))
    possible_positives = K.sum(K.round(K.clip(y_true, 0, 1)))
    recall_keras = true_positives / (possible_positives + K.epsilon())
    return recall_keras


def precision(y_true, y_pred):
    true_positives = K.sum(K.round(K.clip(y_true * y_pred, 0, 1)))
    predicted_positives = K.sum(K.round(K.clip(y_pred, 0, 1)))
    precision_keras = true_positives / (predicted_positives + K.epsilon())
    return precision_keras


eps = 1e-10

def fbeta(y_true, y_pred, beta=1):
    r = K.clip(recall(y_true, y_pred), eps, 1)
    p = K.clip(precision(y_true, y_pred), eps, 1)
    return (1 + beta**2) * (p * r) / (beta**2 * p + r)


def inv_distance(y_true, y_pred):
    return K.clip(1 - 2*K.mean(K.abs(y_true - y_pred)), 0, 1)


def custom_metric(y_true, y_pred):
    metrics = [precision, partial(fbeta, beta=2), binary_accuracy, inv_distance]
    coeff   = [0.1      , 0.6                   , 0.2            , 0.1         ]
    return sum([c * metric(y_true, y_pred) for c, metric in zip(coeff, metrics)])
