# Changes

- 2023-10-14 21:43: Thinking whether a I should have a single python environment for all models or one for every model respectively...
  
  *I think a "global" environment would make more sense in terms of consistency, so that is what I'll be going with for now.*
- 2023-10-15 13:56: For Support Vector Machine the linear kernel can now be considered since a pipeline with standard scaler was recently introduced.
- 2023-10-18 17:17: Switch from looping over class weights ourselves to using the GridSearch class from sklearn. Together with the use of the f1-score this makes model evaluation more objective and more abstractable.
  
  The f1-score was chosen because of its characteristic to balance recall and precision for the positive set. In our case this comes pretty close to what we want:
  - decent precision; we would rather have some clicks not recognized than random hallucinated clicks that could cause all kinds of things while in use.
  - good recall; of course we would still want all actual clicks to be recognized best as possible.
  
  Finally grid search results are saved since it is computationally expensive.
- 2023-10-18 17:27: Add SGDClassifier as another model.
- 2023-10-19 16:16: Added memoization to grid search. This changes the saving mechanism for grid search to be partial making testing and partial execution easier if not possible.
- 2023-10-19 18:39: Rewrote the "Click Detection" notebook to use grid search exclusively.
- 2023-10-20 12:00: Add a "Analysis" section to the jupyter notebook for a closer analysis of the current best model and to provide some intuition for why the model might work.
- 2023-10-20 14:01: Added permutation importance to get an intuition which features contribute the most to the working model.
- 2023-10-21 20:00: Added partial dependence plots for the most important features. Also added partial dependence plots for all features coming directly from the clicking finger.
  
  The PDPs give some intuition into how the model works. Especially the PDPs for all features from the clicking finger clearly shows that most features have near no impact but some higher impact features can be intuitively understood by thinking the actual movement that is happening.
- 2023-10-22 22:40: Build a test model for demonstration purposes. This model uses just the four features that showed a clear impact in the PDPs. Even though the number of features was reduced from 108 to 4 the model still has surprisingly decent performance.
- 2023-10-22 23:14: Added UMAP projection for increased insight and intuition for data separability.
