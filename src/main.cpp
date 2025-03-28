#include "acl/acl.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include <math.h>

using namespace std;
int32_t deviceId = 0;


void InitResource()
{
	aclError ret = aclInit(nullptr);
	ret = aclrtSetDevice(deviceId);
}

void DestroyResource()
{
	aclError ret = aclrtResetDevice(deviceId);
	aclFinalize();
}

int main() {	
	InitResource();
	DestroyResource();
}
