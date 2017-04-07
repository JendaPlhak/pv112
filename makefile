# PV112 2017, lesson 4

binaries = cv4_textures cv4_blending
CXX ?= g++
CXXFLAGS := -std=c++11 -lGL -lglut -lGLEW -lIL $(CXXFLAGS)

cv4: $(binaries)

%: %_main.cpp PV112.h
	$(CXX) -o $@ $< PV112.cpp $(CXXFLAGS)

clean:
	env
	rm $(binaries)
