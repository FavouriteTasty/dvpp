model_name="dvpp"

cmake src -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE

make

if [ $? == 0 ];then
	echo "make for app ${model_name} Successfully"
	exit 0
else
	echo "make for app ${model_name} failed"
	exit 1
fi
