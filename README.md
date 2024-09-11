# 🎵 Light Weight Audio Player 🎵

I need to hear songs while developing. I use YouTube Music, but it's not lightweight. So, I created this lightweight audio player.

## Why Lightweight? 🪶

I have only 8GB RAM, and with Ubuntu installed, while working on projects, I need to run multiple services. So, I need to save RAM as much as possible. Also, by developing this, I can gain a good learning experience.

## How I Started? 🚀

I started by creating a simple audio player using Python. Then, I thought of creating a GUI for it, so I used a custom Tkinter GUI for the UI, Pygame for audio playing, and yt-dlp for downloading audio from YouTube. However, this tech stack used 200 MB of RAM, which was not lightweight for me.

So, I checked with Rust and found Rodio and Iced, but I did not see any difference in RAM usage. To try with C (a great language), I found SDL2 and created this audio player. The audio player runs in 20 MB of RAM, which is good for me.

I don't like to run in CLI because I can't pause or play the song quickly. So, I created a GUI for it. When I checked for a GUI, I found FLTK, which is lightweight and easy to use, taking only 10 MB of RAM, which is good for me.

## Important Note 📝

I am not the best programmer. I have good experience in Python, but I am not interested in UI or desktop applications. I am more interested in web development. I am not good in C; I am learning it, so the code may not be perfect, but it works fine for me.

The logic in the code is mine, but the syntax credit goes to the ChatGPT and Copilot AIs.

## How to Use? 🛠️

I am a Linux user, so I am not sure about Windows or Mac, but you can try it. This code is just version 1. Nothing is complete or perfect; I am still working on it. Not recommended for a better experience. You will not get updates in this code.

## Drawbacks ⚠️

Even though I use C for the audio player, the runtime RAM usage varies from 40-80 MB. I am happy with it, but I will try to reduce it in the future. For that, I might need to switch to another library. Up to now, I have no idea to switch GUI or language.

If you read my story, thanks and sorry for my poor storytelling. Happy Coding! 😊