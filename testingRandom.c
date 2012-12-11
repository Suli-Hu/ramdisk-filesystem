#include <stdio.h>

int main( int argc, const char* argv[] )
{
	char *filepath = "/mydir/filename.txt";
	char delim = '/';
	int delimPosition;
	int index = 0;
	char temp = filepath[index];
	while (temp!='\0') {
		if (temp==delim) {
			printf("Found at %d\n", index);
			delimPosition = index;
		}
		printf("char: %c\n", temp);
		index++;
		temp = filepath[index];
	}

	char *pathname = filepath + delimPosition+1;
	printf("pathname: %s\n", pathname);

}