GPP = g++ -m32 -std=c++11 -Ilib -Isrc -fno-stack-protector -nodefaultlibs
GCC = gcc -m32 -Ilib -Isrc -fno-stack-protector -nodefaultlibs
LINK = $(GCC) -Wl,-z,defs -lcurl
OUTFILE = "./DeepL.so"

COMPILE_FLAGS = -c -O3 -fPIC -w -DLINUX -fno-operator-names

DeepL = -D DeepL $(COMPILE_FLAGS)

all: DeepL

clean:
	-rm -f *~ *.o *.so
  
static: GPP = g++ -m32 -std=c++11 -Ilib -Isrc -fno-stack-protector -static-libgcc -static-libstdc++
static: GCC = gcc -m32 -Ilib -Isrc -fno-stack-protector -static-libgcc -static-libstdc++
static: LINK = $(GPP) -static -Wl,-z,defs -lcurl
static: all

DeepL: clean
	$(GPP) $(DeepL) ./lib/sdk/*.cpp
	$(GPP) $(DeepL) ./lib/json/*.cpp
	$(GPP) $(DeepL) ./src/*.cpp
	$(LINK) -shared -o $(OUTFILE) *.o
