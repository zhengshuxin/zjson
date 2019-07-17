all:
	@(mkdir build; cd build; cmake ..; make)
	@(cd example; make)
	@(cd tools; make)
clean cl:
	@(rm -rf build)
	@(rm -f libzjson.so libzjson.a)
	@(cd example; make clean)
	@(cd tools; make clean)

rebuild rb: cl all
