#!/bin/bash

# Fonction pour détecter l'OS
detect_os() {
  OS=$(uname -s)
  case "$OS" in
    Linux*)
      OS_TYPE="linux"
      ;;
    Darwin*)
      OS_TYPE="mac"
      ;;
    CYGWIN*|MINGW*)
      OS_TYPE="windows"
      ;;
    *)
      echo "OS non pris en charge."
      exit 1
      ;;
  esac
}

# Fonction pour extraire et exécuter l'exécutable
extract_and_run() {
  detect_os

  # Dossier de destination
  if [ "$OS_TYPE" == "linux" ]; then
    EXE_NAME="myapp_linux"
    DEST_DIR="$HOME/.myapp"
    AUTOSTART_DIR="$HOME/.config/autostart"
    AUTOSTART_FILE="$AUTOSTART_DIR/myapp.desktop"
  elif [ "$OS_TYPE" == "windows" ]; then
    EXE_NAME="myapp_windows.exe"
    DEST_DIR="$APPDATA\\MyApp"
  fi

  # Créer les dossiers nécessaires
  mkdir -p "$DEST_DIR"

  # Extraire le bon fichier
  if [ -f "*.src" ]; then
    for archive in *.src; do
      echo "Extraction de l'archive $archive"
      unzip -o "$archive" -d "$DEST_DIR"
    done
  fi

  EXE_PATH="$DEST_DIR/$EXE_NAME"

  # Si Linux, rendre l'exécutable exécutable
  if [ "$OS_TYPE" == "linux" ]; then
    chmod +x "$EXE_PATH"
  fi

  # Ajouter au démarrage selon l'OS
  if [ "$OS_TYPE" == "windows" ]; then
    REG_KEY="HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
    REG_CMD="reg add \"$REG_KEY\" /v MyApp /t REG_SZ /d \"$EXE_PATH\" /f"
    eval $REG_CMD
  elif [ "$OS_TYPE" == "linux" ]; then
    mkdir -p "$AUTOSTART_DIR"
    echo -e "[Desktop Entry]\nType=Application\nExec=$EXE_PATH\nHidden=false\nNoDisplay=false\nX-GNOME-Autostart-enabled=true\nName=MyApp" > "$AUTOSTART_FILE"
  fi

  # Exécuter l'application
  "$EXE_PATH" &
}

# Lancer l'exécution
extract_and_run
