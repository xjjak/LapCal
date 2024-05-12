"""
This module provides caching for grid search on sklearn pipelines.

The module provides the following functions:

    * `summarize_grid_search`  - prints a summary on scoring results
    * `load_grid_search`       - load grid search cache
    * `update_grid_search`     - update grid search in memory from new
                               classifier
    * `train_best_grid_search` - train model on best parameters saved in
                               the cache
    * `save_grid_search`       - save the cache in memory to disk cache
    * `run_grid_search`        - run grid search and return best scores
                               and classifier

Internal functions:

    * `_prefix_param_grid` - prefix keys in a parameter grid with given prefix
    * `_filter_param_grid` - filter out all cached parameter
                           configurations from a parameter grid
"""

# SCORING = ("accuracy", "precision", "recall", "balanced_accuracy", "f1")
# REFIT = "f1"
# FILE = "result.joblib" # oder so


from joblib import dump, load
import os
from sklearn.model_selection import ParameterGrid, GridSearchCV
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline
from sklearn.metrics import make_scorer, accuracy_score, precision_score, recall_score, balanced_accuracy_score, f1_score
import warnings

from typing import Dict, Any, List, Tuple

# extra types
ParameterGrid = Dict[str,float] # FIXME
Classifier = Any

# Public API


# TODO: consistency


def summarize_grid_search(
        scoring: Tuple[str, ...],
        scores: [float]):
    """
    Print name of scoring method along with the actual score.

    Parameters
    ----------
    scoring : Tuple[str, ...]
        Tuple of scoring method names
    scores : [float]
        List of scores that correspond to the provided scoring methods
    """
    
    print("--- Evaluation ---")
    for index, score in enumerate(scoring):
        print(f"{score}: {scores[index]}")


def _prefix_param_grid(
        prefix: str,
        param_grid: ParameterGrid) -> ParameterGrid:
    """
    Prefix keys in a parameter grid with given prefix.

    Parameters
    ----------
    prefix : str
        prefix to add to the parameter grid keys
    param_grid : ParameterGrid
        parameter grid to prefix

    Returns
    -------
    ParameterGrid
        parameter grid with prefixed keys
    """
    
    if isinstance(param_grid, list):
        new_param_grid = []
        for sub_grid in param_grid:
            new_param_grid.append(
                _prefix_param_grid(prefix, sub_grid)
            )

        return new_param_grid
    elif isinstance(param_grid, dict):
        new_param_grid = dict()
        for key, value in param_grid.items():
            new_param_grid[prefix + key] = value
            
        return new_param_grid
    else:
        return param_grid


def load_grid_search(
        state_file: str | os.PathLike,
        model_key: str,
        scoring: Tuple[str, ...]) -> Dict[str, [float]]:
    """
    Load grid search state associated to model_key from state_file.

    Parameters
    ----------
    state_file : str | os.PathLike
        The file all grid search results are saved in
    model_key : str
        String representing the baseline pipeline
    scoring : Tuple[str, ...]
        Tuple of scoring method names

    Returns
    -------
    Dict[str, [float]]
        Dictionary mapping parameter settings to their scores
    """
    
    if os.path.exists(state_file):
        storage = load(state_file)
        key = scoring
        if model_key in storage and key in storage[model_key]:
            return storage[model_key][key]
            
    return dict()


def _filter_param_grid(
        param_grid: ParameterGrid,
        storage: Dict[str, [float]]) -> ParameterGrid:
    """
    Filter out all cached parameter configurations from a parameter grid.

    Parameters
    ----------
    param_grid : ParameterGrid
        full parameter grid to filter
    storage : Dict[str, [float]]
        cache to filter with

    Returns
    -------
    ParameterGrid
        filtered parameter grid
    """
    
    new_param_grid = []
    for param in ParameterGrid(param_grid):
        if not str(param) in storage:
            new_param = dict()
            for key, value in param.items():
                new_param[key] = [value]
                
            new_param_grid.append(new_param)

    return new_param_grid


def update_grid_search(
        storage: Dict[str, [float]],
        clf: Classifier,
        scoring: Tuple[str, ...],
        refit: str):
    """
    Update cache in memory according to given classifier.

    Parameters
    ----------
    storage : Dict[str, [float]]
        cache in memory
    clf : Classifier
        classifier to evaluate scores
    scoring : Tuple[str, ...]
        tuple of scores to save
    refit : str
        scoring method to rank results by
    """
    
    for index, param in enumerate(clf.cv_results_["params"]):
        param = str(param)
        if not param in storage:
            new_scores = []
            for score in scoring:
                score_key = "mean_test_"+score
                if score_key in clf.cv_results_:
                    new_scores.append(clf.cv_results_[score_key][index])
                else:
                    new_scores.apppend(np.nan)

            storage[param] = new_scores

    if refit in scoring:
        refit_index = scoring.index(refit)
        if "best" in storage:
            current_best = str(storage["best"])
            current_score = storage[current_best][refit_index]
            
            new_best = str(clf.best_params_)
            new_score = storage[new_best][refit_index]
    
            if new_score > current_score:
                storage["best"] = clf.best_params_
        else:
            storage["best"] = clf.best_params_
    else:
        print("Something went wrong: refit score not in scoring.")
        

def train_best_grid_search(
        model: Classifier,
        storage: Dict[str, [float]]) -> [float]:
    """
    Train model on best parameters saved in the cache.

    Parameters
    ----------
    model : Classifier
        the model object (pipeline) to train
    storage : Dict[str, [float]]
        the parameter cache
    """
    try:
        param = storage["best"]
        scores = storage[str(param)]
        model.set_params(**param)
        model.fit(X_train, y_train)
        return scores
    except Exception as e:
        print("Could not determine best model:", e)
        return []


def save_grid_search(
        state_file: str | os.PathLike,
        model_key: str,
        scoring: Tuple[str, ...],
        storage: Dict[str, [float]]):
    """
    Save the cache in memory to disk cache.

    Parameters
    ----------
    state_file : str | os.PathLike
        file to save changes to
    model_key : str
        used pipeline
    scoring : Tuple[str, ...]
        tuple of scoring methods
    storage : Dict[str, [float]]
        in memory cache
    """
    
    if os.path.exists(state_file):
        database = load(state_file)
    else:
        database = dict()

    if not model_key in database:
        database[model_key] = dict()

    database[model_key][scoring] = storage

    dump(database, state_file)


def run_grid_search(
        state_file: str | os.PathLike,
        model: Classifier,
        initial_param_grid: ParameterGrid,
        scoring: Tuple[str, ...],
        refit: str) -> Tuple[Tuple[float], Classifier]:
    """
    Run grid search and return best scores and classifier.
    
    Parameters
    ----------
    """
    
    pipe = Pipeline(steps=[
        ("scaler", StandardScaler()), 
        ("model", model)
    ])

    model_key = str(pipe)
    
    storage = load_grid_search(state_file, model_key, scoring)

    param_grid = _filter_param_grid(
        _prefix_param_grid("model__", initial_param_grid),
        storage
    )

    print(f"Processing {len(param_grid)} of {len(ParameterGrid(initial_param_grid))} fits.")
    
    clf = GridSearchCV(
        estimator = pipe,
        param_grid = param_grid,
        scoring = scoring,
        n_jobs = -1,
        refit = refit,
        verbose = 1
    )

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        
        if len(param_grid) > 0:
            clf.fit(X_train, y_train)
            
            update_grid_search(storage, clf, scoring, refit)
    
    best_clf = Pipeline(steps=[
        ("scaler", StandardScaler()), 
        ("model", model)
    ])
    scores = train_best_grid_search(best_clf, storage)

    print(storage["best"])
    save_grid_search(state_file, model_key, scoring, storage)
    
    return scores, best_clf

