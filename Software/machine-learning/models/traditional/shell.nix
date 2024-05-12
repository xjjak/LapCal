let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-23.11";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in
pkgs.mkShellNoCC {
  packages = with pkgs; [
    python311
  ] ++ (with python311Packages; [
    numpy
      joblib
      scikit-learn
      matplotlib
      umap-learn
  ]);
  
  LAPCAL_LIBS = "../../";
  LAPCAL_DATA_DIR = "../../data/";
}
