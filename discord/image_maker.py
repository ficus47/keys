import glob
import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import imageio

import os
import glob
from PIL import Image, ImageDraw, ImageFont
import imageio
import numpy as np

def create_video_with_subtitles(IP: str, subtitles_file: str):
    # Charger les sous-titres
    subtitles = {}
    with open(IP + ".txt", "r", encoding="utf-8") as f:
        for line in f:
            parts = line.strip().split(":", 1)  # Sépare le timestamp et le texte
            if len(parts) == 2:
                timestamp, text = parts
                subtitles[timestamp.strip()] = text.strip()

    # Charger les images BMP, triées par timestamp (float, pas int)
    images = sorted(
        glob.glob(f"{IP}/*.bmp"),
        key=lambda x: float(os.path.basename(x).split(".")[0])
    )

    if not images:
        print("Aucune image trouvée.")
        return

    # Lire une image pour obtenir la taille
    frame = Image.open(images[0])
    width, height = frame.size

    # Initialiser l'écriture vidéo
    video_writer = imageio.get_writer("output.mp4", fps=30)

    font = ImageFont.load_default()

    for img_path in images:
        timestamp = os.path.basename(img_path).split(".")[0]
        frame = Image.open(img_path)
        draw = ImageDraw.Draw(frame)

        if timestamp in subtitles:
            text = subtitles[timestamp]
            draw.text((10, height - 40), text, fill="white", font=font)

        frame_rgb = np.array(frame)
        video_writer.append_data(frame_rgb)

    video_writer.close()
    print("Vidéo créée avec succès ! 🎬")


def create_video_with_subtitles_since(IP: str, subtitles_file: str, since: float):
    subtitles = {}
    with open(IP + ".txt", "r", encoding="utf-8") as f:
        for line in f:
            parts = line.strip().split(":", 1)
            if len(parts) == 2:
                timestamp, text = parts
                if float(timestamp.strip()) >= since:
                    subtitles[timestamp.strip()] = text.strip()

    images = sorted(
        glob.glob(f"{IP}/*.bmp"),
        key=lambda x: float(os.path.basename(x).split(".")[0])
    )

    # Filtrer seulement celles après `since`
    images = [img for img in images if float(os.path.basename(img).split(".")[0]) >= since]

    if not images:
        print("Aucune image à traiter !")
        return

    frame = Image.open(images[0])
    width, height = frame.size

    video_writer = imageio.get_writer("output_since.mp4", fps=30)
    font = ImageFont.load_default()

    for img_path in images:
        timestamp = os.path.basename(img_path).split(".")[0]
        frame = Image.open(img_path)
        draw = ImageDraw.Draw(frame)

        if timestamp in subtitles:
            text = subtitles[timestamp]
            draw.text((10, height - 40), text, fill="white", font=font)

        frame_rgb = np.array(frame)
        video_writer.append_data(frame_rgb)

    video_writer.close()
    print("Vidéo créée avec succès ! 🎬")


def delete_text_and_image_since(IP: str, since: float):
    images = glob.glob(f"{IP}/*.bmp")
    for img_path in images:
        timestamp = float(os.path.basename(img_path).split(".")[0])
        if timestamp >= since:
            os.remove(img_path)
            print(f"Image supprimée : {img_path}")

    if os.path.exists(IP + ".txt"):
        with open(IP + ".txt", "r", encoding="utf-8") as f:
            lines = f.readlines()
        with open(IP + ".txt", "w", encoding="utf-8") as f:
            for line in lines:
                ts = line.strip().split(":", 1)[0]
                if float(ts) < since:
                    f.write(line)
        print("Lignes de sous-titres récentes supprimées.")
    else:
        print("Aucun fichier de sous-titres à nettoyer.")

    print("Suppression terminée.")
