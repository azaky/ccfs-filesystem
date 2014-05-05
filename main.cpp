#include <iostream>
#include "ccfs.hpp"

using namespace std;

int main(int argc, char** argv){
	
	CCFS filesystem;
	
	//filesystem.create("hello.ccfs");
	filesystem.load("hello.ccfs");
	
	return 0;
}
