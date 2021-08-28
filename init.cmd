@echo off

mkdir frames
ffmpeg -i video.mp4 frames/%%04d.png -hide_banner
pause

