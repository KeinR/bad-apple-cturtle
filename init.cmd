@echo off

REM Neato https://serverfault.com/questions/95686/change-current-directory-to-the-batch-file-directory
REM I still hate Batch though
REM Actually this is exactly why I hate it
cd /D "%~dp0"

mkdir frames
youtube-dl "https://www.youtube.com/watch?v=FtutLA63Cp8" -o video
ffmpeg -i video.mkv frames/%%04d.png -hide_banner
ffmpeg -i video.mkv -acodec pcm_s16le -ac 2 audio.wav
pause

