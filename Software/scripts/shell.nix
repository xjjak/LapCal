let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-23.11";
  pkgs = import nixpkgs { config = {}; overlays = []; };
  ble-serial = pkgs.python311Packages.buildPythonPackage rec {
    pname = "ble-serial";
    version = "v2.7.1";
    
    src = pkgs.fetchFromGitHub {
      owner = "Jakeler";
      repo = pname;
      rev = "v2.7.1";
      sha256 = "1b3rrhn5inn4anihm33n2p6rw2wj87w5cjv1awn9dhw7vp3ba9k9";
    };

    propagatedBuildInputs = with pkgs.python311Packages; [
      bleak
      coloredlogs
      pyserial
    ];
  };
in
pkgs.mkShellNoCC {
  packages = with pkgs; [
    ble-serial
    
    python311
    python311Packages.flask

    hid-listen

    libnotify

    platformio

    bash
  ];
}
