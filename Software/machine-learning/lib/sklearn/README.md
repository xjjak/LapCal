# Scikit-learn Modules

## `grid_search.py`

This module provides methods that enhances scikit-learns grid search with very primitive caching.

### Functions

Listed here are all functions that are part of the public API. For more information see the documentation in the source code.

- `summarize_grid_search(scoring, scores)`
- `load_grid_search(state_file, model_key, scoring)`
- `update_grid_search(storage, clf, scoring, refit)`
- `train_best_grid_search(model, storage)`
- `save_grid_search(state_file, model_key, scoring, storage)`
- `run_grid_search(state_file, model, initial_param_grid, scoring, refit)`
