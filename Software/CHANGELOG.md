# Changes

- 2023-10-15 09:26: Create a general state-model that is supposed to predict which buttons would all be pressed.

  This hopefully simplifies the prediction to one model. Some concerns though are:
  - Very quick clicks may not be recognized when press and release are so quick that the model never predicts the pressed state.
  - The training data must be artificially modified in a way that assumes a working model which may lead to better scores than actual performance.
