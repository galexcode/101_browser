#include "FontServer/FontServer.h"
#include <cstdio>

int main(int argc, char** argv)
{
	printf("%i\n", readTTF("../../testfiles/fonts/timesi.ttf"));

	return 0;
}