import os
import glob
import cv2
import subprocess
import numpy as np
from PIL import Image
from datetime import timedelta

def ms_to_srt(ms):
    """
    Convertit un temps en millisecondes en format SRT "HH:MM:SS,mmm"
    """
    td = timedelta(milliseconds=ms)
    total_seconds = int(td.total_seconds())
    hours = total_seconds // 3600
    minutes = (total_seconds % 3600) // 60
    seconds = total_seconds % 60
    milliseconds = ms % 1000
    return f"{hours:02}:{minutes:02}:{seconds:02},{milliseconds:03}"

def generate_srt_from_text(custom_file, srt_output, fixed_duration=1000):
    """
    Lit le fichier texte custom_file ayant des lignes du type :
       1744201397453 : 70 (char: F)
    et génère un fichier SRT (srt_output) où chaque sous-titre sera affiché
    pendant fixed_duration millisecondes (par défaut 1000 ms = 1 seconde).
    Les temps sont relatifs au premier timestamp présent dans le fichier.
    """
    with open(custom_file, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]
    if not lines:
        print("Le fichier de sous-titres est vide !")
        return False

    base_timestamp = None
    srt_lines = []
    index = 1
    for line in lines:
        try:
            # Attendu : "1744201397453 : 70 (char: F)"
            parts = line.split(":", 1)
            timestamp_str = parts[0].strip()
            timestamp = int(timestamp_str)
            if base_timestamp is None:
                base_timestamp = timestamp
            # Dans la deuxième partie, on attend un format comme "70 (char: F)"
            remainder = parts[1]
            # On extrait la lettre après "char:"
            if "char:" in remainder:
                char_part = remainder.split("char:")[1]
                key_char = char_part.strip().strip(")").strip()
            else:
                key_char = ""
            # Temps relatif en ms depuis le début du fichier
            rel_start = timestamp - base_timestamp
            rel_end = rel_start + fixed_duration
            start_str = ms_to_srt(rel_start)
            end_str = ms_to_srt(rel_end)
            srt_lines.append(f"{index}\n{start_str} --> {end_str}\n{key_char}\n\n")
            index += 1
        except Exception as e:
            print(f"Erreur en traitant la ligne '{line}' : {e}")
    with open(srt_output, "w", encoding="utf-8") as f:
        f.writelines(srt_lines)
    print(f"Fichier SRT généré avec succès : {srt_output}")
    return True

def add_subtitles_with_ffmpeg(video_path, srt_file):
    """
    Utilise ffmpeg pour ajouter les sous-titres du fichier srt_file à la vidéo video_path.
    Le résultat est enregistré dans output_with_subtitles.mp4.
    """
    output_video_with_subtitles = "output_with_subtitles.mp4"
    command = [
        "ffmpeg", "-i", video_path,
        "-vf", f"subtitles={srt_file}",
        "-c:v", "libx264", "-c:a", "aac", "-strict", "experimental",
        "-y", output_video_with_subtitles
    ]
    try:
        subprocess.run(command, check=True)
        print("Sous-titres ajoutés à la vidéo avec succès !")
    except subprocess.CalledProcessError as e:
        print(f"Erreur lors de l'ajout des sous-titres: {e}")
    # Supprimer le fichier SRT temporaire
    if os.path.exists(srt_file):
        os.remove(srt_file)

def create_video_with_subtitles(IP: str, subtitles_file: str):
    """
    Crée une vidéo à partir des images du dossier IP,
    convertit le fichier de sous-titres custom (subtitles_file)
    en un fichier SRT, puis ajoute les sous-titres à la vidéo.
    """
    srt_file = "subtitle.srt"
    if not generate_srt_from_text(subtitles_file, srt_file, fixed_duration=1000):
        return

    # Récupération des images (les noms doivent être des nombres indiquant le timestamp)
    images = sorted(
        glob.glob(os.path.join(IP, "*.jpg")),
        key=lambda x: float(os.path.basename(x).split(".")[0])
    )
    if not images:
        print("Aucune image trouvée dans le dossier.")
        return

    # On prend la taille de la première image pour définir les dimensions de la vidéo
    frame = Image.open(images[0])
    width, height = frame.size
    output_video_path = "output.mp4"
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    fps = 8.0
    out = cv2.VideoWriter(output_video_path, fourcc, fps, (width, height))
    if not out.isOpened():
        print("Erreur lors de l'ouverture du fichier vidéo en écriture.")
        return

    for img_path in images:
        frame = Image.open(img_path).convert("RGB")
        frame_bgr = cv2.cvtColor(np.array(frame), cv2.COLOR_RGB2BGR)
        out.write(frame_bgr)
    out.release()
    print("Vidéo créée sans sous-titres.")
    add_subtitles_with_ffmpeg(output_video_path, srt_file)

def create_video_with_subtitles_since(IP: str, subtitles_file: str, since: float):
    """
    Crée une vidéo à partir des images dont le nom (timestamp) est supérieur
    ou égal à since, convertit les sous-titres custom en SRT et ajoute les
    sous-titres à la vidéo.
    """
    srt_file = "subtitle.srt"
    if not generate_srt_from_text(subtitles_file, srt_file, fixed_duration=1000):
        return

    images = sorted(
        glob.glob(os.path.join(IP, "*.jpg")),
        key=lambda x: float(os.path.basename(x).split(".")[0])
    )
    # Filtrer les images avec un timestamp >= since
    images = [img for img in images if float(os.path.basename(img).split(".")[0]) >= since]
    if not images:
        print("Aucune image à traiter pour 'since'.")
        return

    frame = Image.open(images[0])
    width, height = frame.size
    output_video_path = "output.mp4"
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    fps = 8.0
    out = cv2.VideoWriter(output_video_path, fourcc, fps, (width, height))
    if not out.isOpened():
        print("Erreur lors de l'ouverture du fichier vidéo en écriture.")
        return

    for img_path in images:
        frame = Image.open(img_path).convert("RGB")
        frame_bgr = cv2.cvtColor(np.array(frame), cv2.COLOR_RGB2BGR)
        out.write(frame_bgr)
    out.release()
    print("Vidéo créée depuis un certain temps sans sous-titres.")
    add_subtitles_with_ffmpeg(output_video_path, srt_file)

def delete_text_and_image_since(IP: str, since: float):
    """
    Supprime les fichiers image dans le dossier IP dont le nom (timestamp) est supérieur ou égal à since.
    Et supprime les lignes du fichier de sous-titres (si existant) dont le timestamp est >= since.
    """
    images = glob.glob(os.path.join(IP, "*.jpg"))
    for img_path in images:
        try:
            timestamp = float(os.path.basename(img_path).split(".")[0])
            if timestamp >= since:
                os.remove(img_path)
                print(f"Image supprimée : {img_path}")
        except Exception as e:
            print(f"Erreur lors de la suppression de {img_path}: {e}")

    subtitle_path = f"{IP}_subtitles.txt"
    if os.path.exists(subtitle_path):
        with open(subtitle_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
        with open(subtitle_path, "w", encoding="utf-8") as f:
            for line in lines:
                try:
                    ts = line.strip().split(":", 1)[0]
                    if float(ts) < since:
                        f.write(line)
                except Exception as e:
                    print(f"Erreur en traitant une ligne du fichier de sous-titres: {e}")
        print("Lignes de sous-titres récentes supprimées.")
    else:
        print("Aucun fichier de sous-titres à nettoyer.")
    print("Suppression terminée.")

