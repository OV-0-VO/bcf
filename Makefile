all:beacon-flood

beacon-flood: beacon-flood.cpp
		g++ -o beacon-flood beacon-flood.cpp -ltins -pthread