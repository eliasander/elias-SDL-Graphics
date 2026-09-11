{
  description = "C development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          # Build tools
          gcc
          git
          gnumake
          pkg-config
          cmake
          ninja

          # Audio
          alsa-lib
          libpulseaudio
          libjack2
          sndio

          # Text/input
          libthai
          fribidi
          libusb1

          # X11
          libX11
          libXext
          libXrandr
          libXcursor
          libXfixes
          libXi
          libXScrnSaver
          libXtst
          libxkbcommon
          libxinerama
          libxcb

          # Graphics
          libdrm
          libgbm
          libGL
          libglvnd
          mesa

          # EGL / GLES
          libglvnd
          mesa

          # Desktop / system integration
          dbus
          ibus
          systemd

          # Wayland
          wayland
          wayland-protocols
          wayland-scanner
        ];

        shellHook = ''
          export CC=clang
          export CXX=clang++

          export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [
            pkgs.wayland
            pkgs.libxkbcommon
            pkgs.libX11
            pkgs.libXcursor
            pkgs.libXext
            pkgs.libXfixes
            pkgs.libXi
            pkgs.libXinerama
            pkgs.libXrandr
            pkgs.libxcb
            pkgs.libGL
            pkgs.libglvnd
          ]}:$LD_LIBRARY_PATH"
        '';
      };
    };
}
