let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-23.11";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in

pkgs.mkShellNoCC {
  packages = with pkgs; [
    python311
    python311Packages.scikit-learn
    python311Packages.tensorflowWithCuda
    python311Packages.joblib
  ];
}
