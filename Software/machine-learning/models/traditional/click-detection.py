import os
import sys
sys.path.append(os.environ["LAPCAL_LIBS"])

from pathlib import Path

DATADIR = Path(os.environ["LAPCAL_DATA_DIR"])
CACHE = DATADIR / "cache"
os.makedirs(CACHE, exist_ok=True)
CACHE_FILE = CACHE / "traditional-grid-search-results.joblib"
SCORING = ("accuracy", "precision", "recall", "balanced_accuracy", "f1")
REFIT = "f1"

from lib.sklearn.grid_search import *
from sklearn.linear_model import LogisticRegression

_param_grid = [
    {
        "solver": ["lbfgs"],
        "penalty": ["l2"],
        "C": 10.0 ** np.arange(-6, 3),
        "class_weight": [{1: weight} for weight in np.arange(1,20)] + ["balanced"]
    },
    {
        "solver": ["lbfgs"], 
        "penalty": [None], 
        "class_weight": [{1: weight} for weight in np.arange(1,20)] + ["balanced"]
    },
    {
        "solver": ["saga"],
        "penalty": ["l1", "l2", "elasticnet"],
        "C": 10.0 ** np.arange(-6, 3),
        "class_weight": [{1: weight} for weight in np.arange(1,20)] + ["balanced"]
    },
    {
        "solver": ["saga"],
        "penalty": [None],
        "class_weight": [{1: weight} for weight in np.arange(1,20)] + ["balanced"]
    },
]
    

_scores, clf_logistic_regression = run_grid_search(
    state_file         = CACHE_FILE,
    model              = LogisticRegression(),
    initial_param_grid = _param_grid,
    scoring            = SCORING,
    refit              = REFIT,
)

if __name__ == '__main__':
    summarize_grid_search(
        scoring = SCORING,
        scores  = _scores,
    )
