#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#define SPHERELIB_STRICT
//#include <crtdbg.h>

#include "../UserInterface/Locale_inc.h"
#ifdef DIRECTX9
#include <MicrosoftDirectX/d3d9.h>
#include <MicrosoftDirectX/d3dx9.h>
#else
#include <d3d8.h>
#include <d3dx8.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "../eterBase/StdAfx.h"
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
