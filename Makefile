all:
	@(mkdir build; cd build; cmake ..; make)
	@(cd example; make)
clean cl:
	@(rm -rf build)
	@(rm -f libzjson.so libzjson.a)
	@(cd example; make clean)

rebuild rb: cl all
