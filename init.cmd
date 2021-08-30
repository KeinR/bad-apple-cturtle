@echo off

mkdir frames
youtube-dl "https://www.youtube.com/watch?v=FtutLA63Cp8" -o video.mp4
ffmpeg -i video.mp4 frames/%%04d.png -hide_banner
pause

