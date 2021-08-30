@echo off

mkdir frames
youtube-dl "https://www.youtube.com/watch?v=FtutLA63Cp8" -o video
ffmpeg -i video.mkv frames/%%04d.png -hide_banner
pause

