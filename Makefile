CC = g++ -std=gnu++11  -ggdb -O3
PROG = VisPlano
SRCS =	main.cpp\
	inputdata.cpp\
	TinyPngOut.cpp\
	configdata.cpp\
	Shape.cpp\
	SVG2D.cpp\
	Coord.cpp\
	Transform.cpp\
	ImgTransform.cpp\
	NiceScale.cpp\
	boundingBox.cpp\
	LoadShapeFile.cpp\
	lut.cpp\
	sigfigs.cpp

LIBS = -fopenmp
INC= -I/usr/lib64

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS) $(INC)

clean:
	rm -f $(PROG)
