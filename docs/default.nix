{
  perSystem = {pkgs, ...}: {
    packages.docs =
      pkgs.runCommand "docs" {
        buildInputs = with pkgs; [
          mdbook
        ];
      } ''
        mkdir -p $out
        mdbook build --dest-dir $out ${./.}
      '';
  };
}
