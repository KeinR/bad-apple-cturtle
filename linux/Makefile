
SOURCES=main.cpp
OUTPUT=badapple
URL="https://www.youtube.com/watch?v=FtutLA63Cp8"
AUDIO=audio.wav
VIDEO=video.mkv
FRAMES=frames
RESOURCES=$(AUDIO) $(VIDEO) $(FRAMES)

all: res bin

run: all

bin: $(OUTPUT)

res: $(RESOURCES)

# NOTE: Normally, warnings would be a good idea;
# unfrortunately, the C-Turtle library has so many warnings that it is just impractical.
# I'll just rely on the linting for now...
$(OUTPUT): $(SOURCES)
	@echo "--------- compiling source ---------"
	g++ -O2 -Iinclude $(SOURCES) -o $(OUTPUT) $(shell pkg-config x11 --libs) -pthread

$(AUDIO): $(VIDEO)
	@echo "--------- extracting audio ---------"
	ffmpeg -i video.mkv -acodec pcm_s16le -ac 2 audio.wav

$(VIDEO):
	@echo "--------- downloading video ---------"
	youtube-dl "$(URL)" -o video


$(FRAMES): $(VIDEO)
	mkdir frames
	@echo "--------- extracting frames ---------"
	ffmpeg -i video.mkv "frames/%04d.png" -hide_banner

clean:
	rm -rf $(FRAMES) $(VIDEO) $(AUDIO) $(OUTPUT)

