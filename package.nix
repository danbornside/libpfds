{ stdenv
, cunit
, libgcc
, libffi
, doxygen
}: stdenv.mkDerivation {
  src = ./.;
  name = "libpfds";
  buildInputs = [ cunit libgcc libffi doxygen ];
}
