/*
Copyright (C) 2001 StrmnNrmn

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Base/Daedalus.h"

#include "Config/ConfigOptions.h"
#include "Core/CPU.h"
#include "Core/ROM.h"
#include "Core/RomSettings.h"
#include "Debug/DBGConsole.h"
#include "Debug/DebugLog.h"
#include "Interface/Preferences.h"
#include "Interface/RomDB.h"
#include "System/IO.h"
#include "System/Paths.h"
#include "System/SystemInit.h"
#include "Test/BatchTest.h"
#include "Utility/Profiler.h"

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

int __cdecl main(int argc, char **argv)
{
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		IO::Filename exe_path;
		GetModuleFileName(hModule, exe_path, ARRAYSIZE(exe_path));
		gDaedalusExePath = exe_path;
		IO::Path::RemoveFileSpec(&gDaedalusExePath);
	}
	else
	{
		fprintf(stderr, "Couldn't determine executable path\n");
		return 1;
	}

	// ReadConfiguration();

	int result = 0;

	if (!System_Init())
	{
		fprintf(stderr, "System_Init failed\n");
		return 1;
	}

	if (argc > 1)
	{
		bool 			batch_test = false;
		const char *	filename   = NULL;

		for (int i = 1; i < argc; ++i)
		{
			const char * arg = argv[i];
			if (*arg == '-')
			{
				++arg;
				if( strcmp( arg, "-batch" ) == 0 )
				{
					batch_test = true;
					break;
				}
			}
			else
			{
				filename = arg;
			}
		}

		if (batch_test)
		{
			#ifdef DAEDALUS_BATCH_TEST_ENABLED
				BatchTestMain(argc, argv);
			#else
				fprintf(stderr, "BatchTest mode is not present in this build.\n");
			#endif
		}
		else if (filename)
		{
			// Need absolute path when loading from Visual Studio
			// This is ok when loading from console too, since arg0 will be empty, it'll just load file name (arg1)
			std::string rom_path = filename;// IO::Path::Join(gDaedalusExePath, argv[1]);
			fprintf(stderr, "Loading %s\n", rom_path.c_str());
			System_Open(rom_path);
			CPU_Run();
			System_Close();
		}
	}
	else
	{
		// result = RunMain();
	}

	// Write current config out to the registry
	// WriteConfiguration();
	System_Finalize();
	return result;
}