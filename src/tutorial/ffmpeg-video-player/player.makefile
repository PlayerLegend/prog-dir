test/tutorial-player: LDLIBS += `pkg-config --libs libavcodec libavformat libswscale libavutil`
test/tutorial-player: src/tutorial/ffmpeg-video-player/player.o
test/tutorial-player: src/log/log.o

src/tutorial/ffmpeg-vide-player/player.o: CFLAGS += `pkg-config --cflags libavcodec libavformat libswscale libavutil`

TESTS_C += test/tutorial-player
