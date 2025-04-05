import glob
import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import imageio

def create_video_with_subtitles(IP:str, subtitles_file:str):
    # Charger les sous-titres
    subtitles = {}
    with open(IP + ".txt", "r", encoding="utf-8") as f:
        for line in f:
            parts = line.strip().split(":", 1)  # Sépare le timestamp et le texte
            if len(parts) == 2:
                timestamp, text = parts
                subtitles[timestamp.strip()] = text.strip()

    # Charger les images BMP
    images = sorted(glob.glob(f"{IP}/*.bmp"), key=lambda x: int(os.path.basename(x).split(".")[0]))

    # Lire une image pour obtenir la taille
    frame = Image.open(images[0])
    height, width = frame.size

    # Initialiser l'écriture vidéo avec imageio
    video_writer = imageio.get_writer("output.mp4", fps=30)

    # Définir la police (remplace par le chemin vers une police TTF si besoin)
    font = ImageFont.load_default()

    # Générer la vidéo
    for img_path in images:
        timestamp = os.path.basename(img_path).split(".")[0]  # Récupère le timestamp
        frame = Image.open(img_path)  # Ouvre l'image avec PIL
        draw = ImageDraw.Draw(frame)

        # Ajouter le texte si disponible
        if timestamp in subtitles:
            text = subtitles[timestamp]
            text_position = (10, height - 40)  # Position en bas à gauche
            draw.text(text_position, text, fill="white", font=font)

        # Convertir en format RGB et ajouter à la vidéo
        frame_rgb = np.array(frame)
        video_writer.append_data(frame_rgb)

    video_writer.close()
    print("Vidéo créée avec succès ! 🎬")


def create_video_with_subtitles_since(IP: str, subtitles_file: str, since: int):
    # Charger les sous-titres
    subtitles = {}
    with open(IP + ".txt", "r", encoding="utf-8") as f:
        for line in f:
            parts = line.strip().split(":", 1)  # Sépare le timestamp et le texte
            if len(parts) == 2:
                timestamp, text = parts
                if int(timestamp.strip()) >= since:
                    subtitles[timestamp.strip()] = text.strip()

    # Charger les images BMP
    images = sorted(glob.glob(f"{IP}/*.bmp"), key=lambda x: int(os.path.basename(x).split(".")[0]) if int(os.path.basename(x).split(".")[0]) >= since else 0)

    # Vérifier qu'il y a des images à traiter
    if not images:
        print("Aucune image à traiter !")
        return

    # Lire une image pour obtenir la taille
    frame = Image.open(images[0])
    height, width = frame.size

    # Initialiser l'écriture vidéo avec imageio
    video_writer = imageio.get_writer("output_since.mp4", fps=30)

    # Définir la police (remplace par le chemin vers une police TTF si besoin)
    font = ImageFont.load_default()

    # Générer la vidéo
    for img_path in images:
        timestamp = os.path.basename(img_path).split(".")[0]  # Récupère le timestamp
        frame = Image.open(img_path)  # Ouvre l'image avec PIL
        draw = ImageDraw.Draw(frame)

        # Ajouter le texte si disponible
        if timestamp in subtitles:
            text = subtitles[timestamp]
            text_position = (10, height - 40)  # Position en bas à gauche
            draw.text(text_position, text, fill="white", font=font)

        # Convertir en format RGB et ajouter à la vidéo
        frame_rgb = np.array(frame)
        video_writer.append_data(frame_rgb)

    video_writer.close()
    print("Vidéo créée avec succès ! 🎬")


def delete_text_and_image_since(IP: str, since: int):
    # Supprimer les images BMP
    images = sorted(glob.glob(f"{IP}/*.bmp"), key=lambda x: int(os.path.basename(x).split(".")[0]))
    for img_path in images:
        timestamp = int(os.path.basename(img_path).split(".")[0])  # Récupère le timestamp
        if timestamp >= since:
            os.remove(img_path)
            print(f"Image supprimée : {img_path}")

    # Supprimer le fichier de sous-titres
    if os.path.exists("sous_titres.txt"):
        os.remove("sous_titres.txt")
        print("Fichier de sous-titres supprimé.")
    else:
        print("Aucun fichier de sous-titres à supprimer.")
    print("Suppression terminée.")
