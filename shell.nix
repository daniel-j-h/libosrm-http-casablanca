# Nix development environment for reproducible builds.
# Enter development environment via `nix-shell`.
#
# Resources:
# - https://nixos.org/nix/
# - https://nixos.org/nix/manual/#chap-quick-start
# - https://nixos.org/nixpkgs/manual/

with import <nixpkgs> {}; {
  devEnv = stdenv.mkDerivation {
    name = "osrm-integration-casablanca";
    buildInputs = [ cmake ninja boost openssl tbb stxxl luabind lua expat bzip2 zlib ];
  };
}
