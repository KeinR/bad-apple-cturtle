# [BAD APPLE!!!](https://docs.google.com/document/d/1yMQGsJwNXTUsabYNo_k56b9-Jwk1EKFaGgM8e2fVt2c/edit?usp=sharing)

https://www.youtube.com/watch?v=FtutLA63Cp8

*Nagareteku toki no naka de demo kedarusa ga hora guruguru mawatte*
*Watashi kara hanareru kokoro mo mienai wa sou shiranai*

*Jibun kara ugoku koto mo naku toki no sukima ni nagasare tsuzukete*
*Shiranai wa mawari no koto nado watashi wa watashi sore dake*

*Yume miteru? Nani mo mitenai? Kataru mo muda na jibun no kotoba*
*Kanashimu nante tsukareru dake yo nani mo kanjizu sugoseba ii no*

*Tomadou kotoba ataerarete mo jibun no kokoro tada uwa no sora*
*Moshi watashi kara ugoku no naraba subete kaeru no nara kuro ni suru*

*Konna jibun ni mirai wa aru no? Konna sekai ni watashi wa iru no?*
*Ima setsunai no? Ima kanashii no? Jibun no koto mo wakaranai mama*

*Ayumu koto sae tsukareru dake yo hito no koto nado shiri mo shinaiwa*
*Konna watashi mo kawareru no nara moshi kawareru no nara shiro ni naru?*

*Nagareteku toki no naka de demo kedarusa ga hora guruguru mawatte*
*Watashi kara hanareru kokoro mo mienaiwa sou shiranai*

*Jibun kara ugoku koto mo naku toki no sukima ni nagasare tsuzukete*
*Shiranai wa mawari no koto nado watashi wa watashi sore dake*

*Yume miteru? Nani mo mitenai? Kataru mo muda na jibun no kotoba*
*Kanashimu nante tsukareru dake yo nani mo kanjizu sugoseba ii no*

*Tomadou kotoba ataerarete mo jibun no kokoro tada uwa no sora*
*Moshi watashi kara ugoku no naraba subete kaeru no nara kuro ni suru*

*Muda na jikan ni mirai wa aru no? Konna tokoro ni watashi wa iru no?*
*Watashi no koto o iitai naraba kotoba ni suru no nara "roku de nashi"*

*Konna tokoro ni watashi wa iru no? Konna jikan ni watashi wa iru no?*
*Konna watashi mo kawareru no nara moshi kawareru no nara shiro ni naru?*

*Ima yume miteru? Nani mo mitenai? Kataru mo muda na jibun no kotoba?*
*Kanashimu nante tsukareru dake yo nani mo kanjizu sugoseba ii no*

*Tomadou kotoba ataerarete mo jibun no kokoro tada uwa no sora*
*Moshi watashi kara ugoku no naraba subete kaeru no nara kuro ni suru*

*Ugoku no naraba ugoku no naraba subete kowasu wa subete kowasu wa*
*Kanashimu naraba kanashimu naraba watashi no kokoro shiroku kawareru?*

*Anata no koto mo watashi no koto mo subete no koto mo mada shiranai no*
*Omoi mabuta wo aketa no naraba subete kowasu no nara kuro ni nare!!!*

# NOTE

**You will need to download the video with youtube-dl and convert the frames with ffmpeg.**
The script `init.cmd` will do that for you, but you need the youtube-dl and ffmpeg
binaries (youtube-dl.exe and ffmpeg.exe) on your PATH.

I would include them here, but I'm not entirely sure of the technical legality of that.

### Author:
- Author: Orion Musselman

## Use C-Turtle to create something that makes you happy!
-  Creativity is encouraged and will be rewarded.
-  A-level work will demonstrate excellent effort!
-  Be sure you use looping and one or more conditinals! *Yah don't worry about that lmao.*

### Starter files have been provided for you in C++

1. **REASON FOR ART CHOICE**
*Why did this particular art appeal to you and make you happy?*

Touhou is one of the best games series in existance and Bad Apple is one of the best PVs in existance
and it's a trend to port Bad Apple to random places and it's cool.

2. **TITLE** 

Bad Apple, since that's the name of the PV.

3. **C-Turtle METHODS** 
*How much difficulty did you have figuring out how to use the C-Turtle library? Did your knowledge of the Python Turtle library help?*

The documentation isn't the best, but I managed.
It's been so long since I've used Python Turtle that my past experience didn't really help.

4. **C-Turtle USABILITY**
*Was there anything that you tried that did not work? Explain.*

- Double buffering: Meh, some workaround by drawing one frame on top of the other, then deleting the old one.
- Clearing the screen: Fail. Have to reset the entire turtle.
- Hiding turtle always: Fail. Restting will cause it to show for a little while until I can hide it. Until I can find some othe way of clearing the turtle's content...
- Multithreading drawing: I am certain enough that it will just not work that I'm not even going to try it... Rule of thumb: if they don't mention thread safety, there is none. Besides, can't even do double buffering properly.
- Getting sound position with PlaySound... Eugh. It's a bit *too* simple, honestly.

5. **PROCESS SUMMARY**
6. **DESIGN CHALLENGES**
(this answers both)
*Briefly summarize your design and implementation process, 
including how much your initial design plan evolved, 
the final results you achieved, and the amount of time you spent 
as a programmer or programmers in accomplishing these results. 
This should be one or at most two paragraphs.*
*Describe the primary conceptual challenges that you encountered 
in trying to complete this lab and what might have made the 
lab easier for you.*

So I guess this will be the "white paper" I guess for what I did.
It's going to be a challenge recalling it all, but I can at least elaborate on the
code itself.

So, I want to draw Bad Apple with CTurtle. First things first is the video decode.
I am in no way going to spend my time learning how to use VLC - no, my idea is to
preprocess the frames with python or something into a binary format that can be
quicky read by the C++ code. This way, there is no need for even image decode.
Unfortunately, Windows is horrible for developing anything outside of Visual Studio,
and even then it's a pain, and my precious Arch Linux install (my preferred dev environment)
was having issues, and I couldn't get examples to work on my dual-booted Ubuntu install.
I figured that I could just use stb_image for image decode, and perhaps extract the frames
and grayscale images one off. I was reminded of ffmpeg, and took a one liner from
Stack Overflow that would take the frames from the video. Not grayscale, but that's fine.
Now, for the actual implementation.

Originally, I wanted to have the drawn shapes be filled. After racking my brain, I could not
fall upon a solution that I deemed fast enough - the most clever I got was using raycasting to
find out if a polygon could be created in an area, using a flood algorithm (?) to find all the
"holes", or "color pockets" in a scene, and then drawing those each as individual shapes.
I figured it would be best to just focus in drawing the outlines for now, and as I would later
learn, CTurtle is so slow it was a good thing that I abandoned that idea.

Originally I was going to take inspiration from what I remember from A* and modify that to
sort of pathfind around the holes. I didn't like the idea though, because of the cost of building
and iterating the linked list map. (are those called "graphs?") I spent a while thinking about this
one, and finally rested upon iterating the entire image, finding points where the color changes,
then following a trail of changing color to get get a "path" that encompases a hole. This is important
for efficiency, since it's faster to draw one line than many, and if the points come right after
one another in order, then I can remove a few in-between without significant loss of image quality.
However, this had issues. I can't remember them all really, but I can say that it Just Didn't Work.

I did a lot of debugging at this time - not using visual studio's debugger, as it's awkward to use,
but through prints and forensics; main.cpp:152 I write binary dumps of stb_image's output to make sure
that the library is functioning as I expect.

After nearly breaking my keyboard I came upon an idea: Loop linearly through the image. Ignore white
nodes (pixels) (could also be white, I just chose black arbitrarially). For each node, search around it
and find if there is a node that diffets from it. If there is, this is the beginning of a path around a hole,
and I loop continually - search the nodes around the current one, the "locus," and find another one
that has an adjacent node that is different. The loop ends when there are no nearby nodes that have
and adjacent (reminder, black) node that differs from it. Every locus gets added to a bool map so that
nodes don't get placed into multible paths. I make sure to check that the locus is not equal to the
first locus, so that we don't loop multible times in the case of closed holes.

This encountered an issue with paths being very, very short. I eventually found out the cause:
loci were choosing diagonal adjacent nodes that diverted away from the path, and trapped the
path. Something like this:

0 = black
1 = white

1110000\
1100000\
1100000\
1100000\
1000000

Now with the steps the path takes (0 = not touched by path):

0001000\
0024000\
0030000\
0000000\
0000000

Notice how the path is completely valid, and yet, it is stopped at 4?
This issue is fixed by requiring that each new locus have adjacent white
nodes HORIZONTALLY or VERTICALLY, NOT DIAGONALLY. This is apparent in the static
arrays `matrixRedPill` and `matrixBluePill`. Their names have contextual meaning,
but it's not that deep so don't try too hard.

The next issue to tackle, now that the paths are complete, is to prune them so that drawing
is faster. Honestly, the speed of the previous operations isn't really important given
how slow CTurtle drawing is... What's important is that I reduce the nodes in the paths to
only what is necessary to have a complete picture.

At first I just skipped a few frames, but that didn't really work out so well.
I did some thinking and finally settled upon taking the area of the deviation for
when to select a node.

There is the last node added, the locus, and the one after it. Those two form
a line that will represent the "intended direction" (IDIR) of the next section of the path
that will later be drawn by the turtle. We loop through the next nodes in the path,
and for each, we form a triangle with the node, the locus, and the point where
the IDIR line and a perpendicular line to IDIR that passes through the node intersect
(called INTR). The locus, node, and INTR form a triangle that we take the area of,
which is just a simple math problem. If the area is too high, if we are losing too much
"precision," we add the node and make it the next locus, and so on.

I encountered an issue where the lines appeared disjointed, like part of the path was missing.
I thought the issue was more systemic and was reluctant to debug it, but upon serious inspection,
it was simply that I forgot to add the first locus node.

I also had an issue of extremely slow speed. I thought at first that it was because
CTurtle was slow, but it turns out, it was me that was slow ;). I had to set
TurtleScreen::tracer with all zeros. Once I did that, it sped up dramatically, and I fixed
by double buffering woes of the last buffer being visible.

It looks really good at this point, but still a little slow.

Then I got a amazing idea.

Multithreading!

The bottleneack was now no longer the drawing, but the pre-processing of each frame.
I benchmarked the frame processing, and saw that each frame took ~0.1 seconds to process.
That's 10 frames per second.

The freshmen computers (one of which I am so privilaged to have) have four cores.
8 logical threads, but is that really cores??? What are even logical threads???

Anyways, that means I could probobly do well with like four threads, I guess five plus
the main thread.

Each worker thread will be started with a state structure. The state will contain some
flags and the frame buffer (two words, not one). Each frame is a single index, so threads
can work on multible frames at the same time. When looking for a frame to work on, a thread
will start from the index of the rendering frame and iterate over each other frame and
check for one not being worked on by another thread, then work on that frame. The main
thread will traverse the array linearly, only rendering a frame if it has been marked
as completed.

This resulted in a speed up so dramatic that the frames are now rendered at 30 frames
per second - just like the original video! Success!

Perhaps a better way of distributing the frame work would be having each thread
be assigned a number, 0, 1, 2 3, and then increment that number by 4 each time.
That way, it will never hit another frame that is worked on by another thread,
and there is no need to iterate, and the frames will be completed in a progressive
manner. 

It's pretty complete, if I do say so myself, but it's missing something
important...
SOUND!

What good is a turtle adaptation of a music PV if it doesn't
have any... Music?

In Windex, this is simple enough; there is a PlaySound function.
Under Linux, you might want to consider VLC or something of the nature.
There was one major issue however....

The sound and animation just didn't want to sync.
I tried many things, to no avail. I thought I could get the
play status from PlaySound, but that does not seem possible.

I resolved to put the PlaySound on SYNC and on a thread that
I control to reduce impact from thread startup. Then,
as the animation is playing, I correct the FPS to make sure that
it doesn't get too far/behind. I don't really know why
it's out of sync to begin with, but I can theorize that
it's due to the call to time() and whatnot that throw it off
ever so slightly, or floating point errors (more likely)
since the frame duration is 33.3333 ms... something for 30 FPS.

It will sometimes not sync very well however - the sound will
be behind the animation.
THe only reason I have for this right now is memory cache - 
and I've tested it to some extent.
The audio file is in read from disk the first time it's played,
but after that it gets shunted off to a memory cache by the OS that
makes reads way, way faster.
In any case, after a few attempts, it is synched pretty well.

And uh, yah, there you go. I wanted to get filling done, but oh well.
Also Linux portability - I feel slightly dirty licensing a Windows
only application under the GPL.

7. **ERRORS**
*List in bulleted form of all known errors 
and deficiencies with a very brief explanation.*

This would be a very different question if you were asking for ALL errors
that I experienced. But for now...
- Drawing is too slow (but there's not much I can do about that short of editing C-Turtle directly)
- No filling for the silloets (like in PV)
- Not portable to Linux in its current state, as it uses Windex libs
- Audio sync would be improved dramatically if VLC was used, as there is more control over streaming

8. **LEARNING AND REACTION**
*A paragraph or so of your own comments 
on what you learned and your reactions to this lab.*

I learned a lot, I think. My understanding of "graph" algorithm design
was improved, as I built the turtle paths. I learned more than I
felt I should about Windex APIs. This is one of the few times I've used
multithreading, and especially for something so practical like speeding
up a computation (usually, I reconsider using more processing power).
I gained a greater understanding of syncronizing frames, as I've learned that
frames will have some ammount of error that has to be corrected.
To be completely honest, I haven't had this much fun in a while.

9. **INTEGRITY STATEMENT**
*Please briefly describe ALL help you received and 
all help you gave to others in completing this lab.
Also list all other references below.*

Stack Exchange for many things. I don't have all the links, but the ones that I can find:
- https://stackoverflow.com/questions/6828751/batch-character-escaping#16018942
- https://softwareengineering.stackexchange.com/questions/105912/can-you-change-code-distributed-under-the-mit-license-and-re-distribute-it-unde
- https://stackoverflow.com/questions/25173462/what-is-the-sizeof-stdarraychar-n
- https://stackoverflow.com/questions/51316233/how-can-i-see-git-diff-on-the-visual-studio-code-side-by-side-file
- https://savvyadmin.com/extract-audio-from-video-files-to-wav-using-ffmpeg/
- For how to break video into frames with ffmpeg

C-Turtle docs (which weren't really that helpful, thanks, and the website is down (?) so I had to download the repo and view locally)

## References:

- stb_image under public domain; Sean T. Barrett
- CTurtle under MIT; Jesse W. Walker
- CImg under CeCILL; David Tschumperle

All library licences listed above are compatable with the GPL.

- Bad Apple (indirectly) (original PV); originally from NicoNico, but also found on YouTube
- youtube-dl for scraping the video from YouTube
- ffmpeg for extraction of video frames
- Windows for PlaySound function (even though it sucks!)
- All those "Bad Apple on X" videos/projects
    - See this great playlist: https://www.youtube.com/playlist?list=PLajlU5EKJVdonUGTEc7B-0YqElDlz9Sf9

