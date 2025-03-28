bash build.sh
chmod +X out/main
export LD_LIBRARY_PATH=/home/dml/ffmpeg/ffmpeg-ascend/ascend/lib:$LD_LIBRARY_PATH
./out/main