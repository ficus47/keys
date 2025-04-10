import discord
from discord.ext import commands
from image_maker import *  # Assure-toi d'importer correctement tes fonctions

subtitles_file = ".txt"  # Remplace par le chemin vers ton fichier de sous-titres
dir = "discord/"

intents = discord.Intents.default()
intents.messages = True  # Permet de recevoir les messages
intents.message_content = True  # Active les intents pour recevoir les messages

bot = commands.Bot(command_prefix="!", intents=intents)

ID_CANAL = 1358140808347389972  # Remplace par l'ID de ton canal

@bot.event
async def on_ready():
    print(f"Connecté en tant que {bot.user}")
    # Tu peux aussi afficher un message dans un canal spécifique si tu veux
    channel = bot.get_channel(ID_CANAL)
    await channel.send("Je suis prêt !")  # Exemple de message dans un canal
    channel = bot.get_channel(ID_CANAL)  # Remplace 'général' par le nom de ton salon
    if channel:
        await channel.send(f"The commands are:\n"
                           f"!generate <IP> to generate a video of the IP\n"
                           f"!generate_since <IP> <second> to generate a video of the IP since the second\n"
                           f"!delete <IP> <second> to delete the images of the IP since the second\n"
                           f"<second> is the number of seconds since 1 January 1970 (midnight UTC)")

@bot.event
async def on_member_join(member):
    # Accède au salon général (ou un autre salon spécifique)
    channel = bot.get_channel(ID_CANAL)  # Remplace 'général' par le nom de ton salon
    if channel:
        await channel.send(f"Welcome to the server, {member.name} ! 🎉\n"
                           f"The commands are:\n"
                           f"!generate <IP> to generate a video of the IP\n"
                           f"!generate_since <IP> <second> to generate a video of the IP since the second\n"
                           f"!delete <IP> <second> to delete the images of the IP since the second\n"
                           f"<second> is the number of seconds since 1 January 1970 (midnight UTC)")

@bot.command()
async def generate(ctx, ip):
    channel = bot.get_channel(ID_CANAL)
    try:
        await channel.send(f"Generating video for IP: {ip}")
        # Appelle ta fonction de génération de vidéo
        create_video_with_subtitles(dir+ip, os.path.abspath(dir+ip+subtitles_file))  # Remplace par ta fonction qui génère la vidéo
        await channel.send(f"Video generated for IP: {ip}", file=discord.File("output_with_subtitles.mp4"))  # Envoie la vidéo générée
    except Exception as e:
        await channel.send(f"An error occurred: {str(e)}")

@bot.command()
async def generate_since(ctx, ip, second):
    channel = bot.get_channel(ID_CANAL)
    try:
        await channel.send(f"Generating video for IP: {ip} since {second}")
        # Appelle ta fonction de génération de vidéo depuis une seconde donnée
        create_video_with_subtitles_since(dir+ip, os.path.abspath(dir+ip+subtitles_file), int(second)*1000)
        await channel.send(f"Video generated for IP: {ip} since {second}", file=discord.File("output_with_subtitles.mp4"))
    except Exception as e:
        await channel.send(f"An error occurred: {str(e)}")

@bot.command()
async def delete(ctx, ip, second):
    channel = bot.get_channel(ID_CANAL)
    try:
        await channel.send(f"Deleting images for IP: {ip} since {second}")
        # Appelle ta fonction pour supprimer les images depuis une seconde donnée
        delete_text_and_image_since(dir+ip, os.path.abspath(dir+ip+subtitles_file), int(second)*1000)
        await channel.send(f"Images deleted for IP: {ip} since {second}")
    except Exception as e:
        await channel.send(f"An error occurred: {str(e)}")

bot.run("MTM1ODEzNjM1MjgxMzg3OTUzNw.GxtJS_.8IAQ3q9J_SwkGHg-u_-b2NKIwgUq8lEagOFijo")  # Remplace par ton token Discord



# MTM1ODEzNjM1MjgxMzg3OTUzNw.GxtJS_.8IAQ3q9J_SwkGHg-u_-b2NKIwgUq8lEagOFijo