{nixpkgs ? import <nixpkgs> {}} :
with nixpkgs;
let libpfds = pkgs.callPackage (import ./package.nix) {};
in pkgs.mkShell {
  inputsFrom = [ libpfds ];
  packages = [ pkgs.gdb pkgs.dmalloc pkgs.ddd pkgs.pkg-config pkgs.libffi.dev valgrind ];
}

