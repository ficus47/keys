import discord
from discord.ext import commands
import image  # Assure-toi d'importer correctement tes fonctions

subtitles_file = "subtitles.txt"  # Remplace par le chemin vers ton fichier de sous-titres

intents = discord.Intents.default()
intents.messages = True  # Active les intents pour recevoir les messages

bot = commands.Bot(command_prefix="!", intents=intents)

ID_CANAL = 1358140808347389972  # Remplace par l'ID de ton canal

@bot.event
async def on_ready():
    print(f"Connecté en tant que {bot.user}")
    # Tu peux aussi afficher un message dans un canal spécifique si tu veux
    channel = bot.get_channel(ID_CANAL)
    await channel.send("Je suis prêt !")  # Exemple de message dans un canal

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
    await ctx.send(f"Generating video for IP: {ip}")
    # Appelle ta fonction de génération de vidéo
    create_video_with_subtitles(ip, subtitles_file)  # Remplace par ta fonction qui génère la vidéo
    await ctx.send(f"Video generated for IP: {ip}", file=discord.File("output.avi"))  # Envoie la vidéo générée

@bot.command()
async def generate_since(ctx, ip, second):
    await ctx.send(f"Generating video for IP: {ip} since {second}")
    # Appelle ta fonction de génération de vidéo depuis une seconde donnée
    create_video_with_subtitles_since(ip, subtitles_file, int(second))
    await ctx.send(f"Video generated for IP: {ip} since {second}", file=discord.File("output.avi"))

@bot.command()
async def delete(ctx, ip, second):
    await ctx.send(f"Deleting images for IP: {ip} since {second}")
    # Appelle ta fonction pour supprimer les images depuis une seconde donnée
    delete_text_and_image_since(ip, int(second))
    await ctx.send(f"Images deleted for IP: {ip} since {second}")

bot.run("MTM1ODEzNjM1MjgxMzg3OTUzNw.GFPVw_.fMAEg6rgsgD6DlLMcF1HQiyEugA63mhEmMPaYw")  # Remplace par ton token Discord





#MTM1ODEzNjM1MjgxMzg3OTUzNw.GFPVw_.fMAEg6rgsgD6DlLMcF1HQiyEugA63mhEmMPaYw