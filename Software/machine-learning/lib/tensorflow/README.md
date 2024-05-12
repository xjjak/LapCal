# TensorFlow Modules

## `metric.py`

This module provides the [proposed custom metric](https://github.com/xjjak/LapCal/issues/28) and some auxiliary metrics and functions.

The difference between some of the functionality provided by this module and similar functionality provided by tensorflow is that the provided functionality is purely functional which makes it easily composable. Most of tensorflows metrics are provided as classes but I couldn't figure out how to compose these into one weighted metric.

### Functions

Listed here are all functions that are part of the public API. For more information see the documentation in the source code.

- `custom_metric(y_true, y_pred)`
- `recall(y_true, y_pred)`
- `precision(y_true, y_pred)`
- `fbeta(y_true, y_pred)`
- `inv_distance(y_true, y_pred)`
