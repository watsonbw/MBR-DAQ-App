{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, utils, ... }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default =
          pkgs.lib.throwIfNot pkgs.stdenv.isDarwin
            "This project is macOS-only and requires Apple clang + Xcode Command Line Tools."
            (
              pkgs.mkShellNoCC {
                buildInputs = with pkgs; [
                  cmake
                  ninja
                  clang-tools
                ];

                shellHook = ''
                  unset DEVELOPER_DIR
                  export CC=/usr/bin/clang
                  export CXX=/usr/bin/clang++
                  export SDKROOT=$(xcrun --show-sdk-path)
                '';
              }
            );
      }
    );
}
