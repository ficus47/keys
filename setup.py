# setup.py

import os
import sys
import shutil
import subprocess

def main():
    # Vérifier les arguments
    if len(sys.argv) != 3:
        print("Usage: python setup.py <exe1> <exe2>")
        sys.exit(1)
    exe1, exe2 = sys.argv[1], sys.argv[2]

    # Répertoire de build dans le dossier courant
    build_dir = os.path.join(os.getcwd(), "fake_sign_package")
    # Nettoyage du dossier existant
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    os.makedirs(build_dir, exist_ok=True)

    # Copier les deux executables dans build_dir
    for exe in (exe1, exe2):
        if not os.path.isfile(exe):
            print(f"Erreur : fichier introuvable : {exe}")
            sys.exit(1)
        shutil.copy2(exe, os.path.join(build_dir, os.path.basename(exe)))

    # Se placer dans build_dir
    os.chdir(build_dir)

    # Variables
    CERT_NAME = "JustAFakeCert"
    CERT_PASS = "1234"
    ZIP_PASSWORD = "serenity2025"
    OUTPUT_ZIP = "launcher-signed.zip"

    # 1. Générer un certificat auto-signé
    subprocess.run([
        "openssl", "req", "-x509", "-newkey", "rsa:2048",
        "-keyout", "fake.key", "-out", "fake.crt",
        "-days", "365", "-nodes",
        "-subj", f"/CN={CERT_NAME}"
    ], check=True)

    # 2. Créer le PFX
    subprocess.run([
        "openssl", "pkcs12", "-export",
        "-out", "fake.pfx",
        "-inkey", "fake.key",
        "-in", "fake.crt",
        "-password", f"pass:{CERT_PASS}"
    ], check=True)

    # 3. Signer les deux exécutables
    signed_exes = []
    for exe in (os.path.basename(exe1), os.path.basename(exe2)):
        signed_name = os.path.splitext(exe)[0] + "-signed.exe"
        subprocess.run([
            "osslsigncode", "sign",
            "-pkcs12", "fake.pfx",
            "-pass", CERT_PASS,
            "-n", CERT_NAME,
            "-i", "http://example.com",
            "-in", exe,
            "-out", signed_name
        ], check=True)
        signed_exes.append(signed_name)

    # 4. Créer un ZIP protégé par mot de passe
    subprocess.run(
        ["zip", "-P", ZIP_PASSWORD, OUTPUT_ZIP] + signed_exes,
        check=True
    )

    print(f"✅ Build terminé : {OUTPUT_ZIP} (mot de passe : {ZIP_PASSWORD})")

if __name__ == "__main__":
    main()

