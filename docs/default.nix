{
  perSystem = {pkgs, ...}: let
    typst' = pkgs.typst.withPackages (ps: [
      ps.abbr_0_3_0
    ]);
  in {
    packages.docs =
      pkgs.runCommand "docs.pdf" {
        buildInputs = [
          typst'
        ];
      } ''
        cd ${./.}
        typst compile vulcanite.typ $out
      '';
  };
}
