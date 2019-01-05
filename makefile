#this i makefile

all: fm

fm:
	g++ -o FM_v1.0 Fiduccia_Mattheyses_v1.0.cpp
	g++ -o FM_v1.1 Fiduccia_Mattheyses_v1.1.cpp
	g++ -o FM_v1.2 Fiduccia_Mattheyses_v1.2.cpp
	g++ -o FM_v1.3 Fiduccia_Mattheyses_v1.3.cpp
	g++ -o FM_v1.4 Fiduccia_Mattheyses_v1.4.cpp

clean:
	rm -rf FM_v* fm
	rm -rf Sain_Kushwaha* fm
