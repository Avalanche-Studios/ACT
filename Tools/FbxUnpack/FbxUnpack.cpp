
/*
#
# Copyright(c) 2021 Avalanche Studios.All rights reserved.
# Licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE
#
*/

#include <cstdio>

#include <vector>
#include <string>

#include "fbxsdk.h"
#include "getopt.h"

#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(manager->GetIOSettings()))
#endif

static void Usage()
{
	printf("Usage: FbxUnpack [options] file\n");
	printf("\n");
	printf("Options:\n");

	printf("  -h         Show this help message and exit\n");
	printf("  -o FILE    Output file. Default is stdout\n");
	printf("  -t FILE    Output thumbnail picture. Default is empty, no output\n");
	printf("  -v         Verbose output\n");
}

static void Error(const char *msg)
{
	printf("FbxUnpack: %s\n", msg);
}

bool SaveBMP(const char *file_name, const int32_t channel_count, const int32_t width, const int32_t height, const unsigned char* pixels) 
{
	FILE *file{ nullptr };
	errno_t err = fopen_s(&file, file_name, "wb");
	
	if (err != 0)
	{
		fprintf(stderr, "cannot open file '%s': %s\n",
			file_name, strerror(err));
		return false;
	}

#pragma pack (push, 1)
	struct BMPHeader
	{
		uint16_t BmpIdentifier;
		uint32_t FileSize;
		uint32_t Reserved;
		uint32_t BitmapOffset;
		uint32_t HeaderSize;
		uint32_t Width;
		uint32_t Height;
		uint16_t Planes;
		uint16_t Bpp;
		uint32_t Compression;
		uint32_t SizeOfBitmap;
		uint32_t HorzResolution;
		uint32_t VertResolution;
		uint32_t ColorsUsed;
		uint32_t ColorsImportant;
	};
#pragma pack (pop)
	BMPHeader header =
	{
		('B') | ('M' << 8),
		0,
		0,
		sizeof(BMPHeader),
		40,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
		1,
		32,
		0,
		0,
		0x0B13,
		0x0B13,
		0,
		0,
	};

	size_t written = fwrite(&header, sizeof(header), 1, file);
	if (written != 1)
	{
		fclose(file);
		return false;
	}

	const bool src_has_alpha = (channel_count == 4);
	
	uint8_t* buffer = new uint8_t[4 * width];

	const int32_t dest_r_index = 2;
	const int32_t dest_b_index = 0;
	for (int32_t y = 0; y < height; y++)
	{
		const int32_t flipped_y = y;
		uint8_t* dest = buffer;
		uint8_t* src = ((uint8_t *)pixels) + flipped_y * channel_count * width;
		for (int32_t x = 0; x < width; x++)
		{
			*dest++ = src[dest_r_index];
			*dest++ = src[1];
			*dest++ = src[dest_b_index];

			if (src_has_alpha)
			{
				*dest++ = src[3];
				src += 4;
			}
			else
			{
				src += 3;
			}
		}
		written = fwrite(buffer, 4, width, file);
		if (written != width)
		{
			delete[] buffer;
			fclose(file);
			return false;
		}
	}

	delete[] buffer;
	fclose(file);
	return true;
}

struct SOptionsData
{
	bool compact = false;
	std::string input_file;
	std::string output_file;
	std::string output_thumbnail;
};

bool ReadOptions(SOptionsData& data, int argc, char **argv)
{
	int c;
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help",		0, 0, 'h'},
			{"output-file", 0, 0, 'o'},
			{"thumbnail",   0, 0, 't'},
			{"compact",     0, 0, 'c'},
			{0,               0, 0, 0}
		};

		c = getopt_long(argc, argv, "vcho:t:", long_options, &option_index);
		if (c == -1) break;

		switch (c) {
		case 'h':
			Usage();
			return 0;
		case 'o':
			data.output_file = optarg;
			break;
		case 't':
			data.output_thumbnail = optarg;
			break;
		case 'c':
			data.compact = true;
			break;
		}
	}

	if (optind >= argc)
	{
		Error("No input file specified.");
		return false;
	}
	data.input_file = argv[optind];
	return true;
}

bool SaveThumbnail(FbxThumbnail* thumbnail, const char* filename)
{
	if (!thumbnail)
		return false;
	
	int32_t width = 0;
	int32_t height = 0;

	switch (thumbnail->GetSize())
	{
	case FbxThumbnail::e64x64:
		width = 64;
		height = 64;
		break;
	case FbxThumbnail::e128x128:
		width = 128;
		height = 128;
		break;
	case FbxThumbnail::eCustomSize:
		width = thumbnail->CustomWidth;
		height = thumbnail->CustomHeight;
		break;
	}

	if (width > 0 && height > 0)
	{
		const uint32_t channel_count = (thumbnail->GetDataFormat() == FbxThumbnail::eRGB_24) ? 3 : 4;
		return SaveBMP(filename, channel_count, width, height, thumbnail->GetThumbnailImage());
	}
	return false;
}

int main(int argc, char **argv)
{
	using namespace rapidjson;
	using namespace std;

	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	FbxManager* manager = FbxManager::Create();
    if( !manager )
    {
        Error("Unable to create FBX Manager!");
        return EXIT_FAILURE;
    }

	SOptionsData data;
	if (!ReadOptions(data, argc, argv))
		return EXIT_FAILURE;

    //Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);
    
	// TODO: Load plugins from the executable directory (optional)
	//FbxString lPath = FbxGetApplicationDirectory();
	//manager->LoadPluginsDirectory(lPath.Buffer());

	int32_t SDK_major, SDK_minor, SDK_revision;
	int32_t file_major, file_minor, file_revision;

	// Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(SDK_major, SDK_minor, SDK_revision);

	// Create an importer.
    FbxImporter* importer = FbxImporter::Create(manager,"");
    
    // Initialize the importer by providing a filename.
    const bool import_status = importer->Initialize(data.input_file.c_str(), -1, manager->GetIOSettings());
    importer->GetFileVersion(file_major, file_minor, file_revision);
    
    if( !import_status )
    {
        FbxString error = importer->GetStatus().GetErrorString();
        Error("Call to FbxImporter::Initialize() failed.");
        printf("Error returned: %s", error.Buffer());
        
        if (importer->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            printf("FBX file format version for this FBX SDK is %d.%d.%d\n", SDK_major, SDK_minor, SDK_revision);
            printf("FBX file format version for file '%s' is %d.%d.%d\n\n", data.input_file.c_str(), file_major, file_minor, file_revision);
        }
        
        return EXIT_FAILURE;
    }

	// save thumbnail if needed
	bool saved_thumbnail{ false };
	if (!data.output_thumbnail.empty())
		saved_thumbnail = SaveThumbnail(importer->GetSceneInfo()->GetSceneThumbnail(), data.output_thumbnail.c_str());

	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();               // Between StartObject()/EndObject(), 

	writer.Key("SDK_Version");
	writer.StartObject();

	writer.Key("major");            // output a key,
	writer.Uint(SDK_major);             // follow by a value.

	writer.Key("minor");
	writer.Uint(SDK_minor);

	writer.Key("revision");
	writer.Uint(SDK_revision);
	
	writer.EndObject();

    if (importer->IsFBX())
    {
		writer.Key("File");
		writer.StartObject();

		writer.Key("name");            // output a key,
		writer.String(data.input_file.c_str());

		writer.Key("major");            // output a key,
		writer.Uint(file_major);             // follow by a value.

		writer.Key("minor");
		writer.Uint(file_minor);

		writer.Key("revision");
		writer.Uint(file_revision);

		writer.Key("thumbnail");            // output a key,
		writer.Bool(saved_thumbnail);

		writer.EndObject();

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        int32_t anim_stack_count = importer->GetAnimStackCount();
        
		writer.Key("anim_stack_count");
		writer.Uint(anim_stack_count);

		writer.Key("stacks");
		writer.StartArray();

        for(int32_t i = 0; i < anim_stack_count; ++i)
        {
            FbxTakeInfo* take_info = importer->GetTakeInfo(i);
            
			writer.StartObject();               // Between StartObject()/EndObject(), 

			writer.Key("name");
			writer.String(take_info->mName.Buffer());

			writer.Key("desc");
			writer.String(take_info->mDescription.Buffer());

			writer.Key("start_frame");
			writer.Int(static_cast<int32_t>(take_info->mLocalTimeSpan.GetStart().GetFrameCount()));

			writer.Key("end_frame");
			writer.Int(static_cast<int32_t>(take_info->mLocalTimeSpan.GetStop().GetFrameCount()));

			writer.Key("frame_count");
			writer.Uint(static_cast<uint32_t>(take_info->mLocalTimeSpan.GetDuration().GetFrameCount()));
			writer.EndObject();
        }
        
		writer.EndArray();

		// TODO: avoid reading entire scene, put everything into document info !!

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		FbxScene* scene = FbxScene::Create(manager, "My Scene");
		if (!scene)
		{
			Error("Unable to create FBX scene!");
			return EXIT_FAILURE;
		}

        // Set the import states. By default, the import states are always set to
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        false);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         false);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            false);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           false);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            false);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		importer->Import(scene);

		constexpr const char* OPTIONS_NODE_NAME = "compile_options_node";

		for (int32_t i = 0; i < scene->GetNodeCount(); ++i)
		{
			if (strstr(scene->GetNode(i)->GetName(), OPTIONS_NODE_NAME) != nullptr)
			{
				FbxProperty prop = scene->GetNode(i)->FindProperty("compile_options");
				if (prop.IsValid())
				{
					// json data
					FbxString json_data = prop.Get<FbxString>();

					writer.Key("compile_options");            // output a key,
					writer.String(json_data.Buffer());
				}
				prop = scene->GetNode(i)->FindProperty("linked_takes");
				if (prop.IsValid())
				{
					// json data
					FbxString json_data = prop.Get<FbxString>();

					writer.Key("linked_takes");            // output a key,
					writer.String(json_data.Buffer());
				}
				prop = scene->GetNode(i)->FindProperty("user_information");
				if (prop.IsValid())
				{
					// json data
					FbxString json_data = prop.Get<FbxString>();

					writer.Key("user_information");            // output a key,
					writer.String(json_data.Buffer());
				}
			}
		}
    }

	writer.EndObject();

	if (!data.output_file.empty())
	{
		std::FILE* fp = std::fopen(data.output_file.c_str(), "w");

		if (!fp) {
			Error("File opening failed");
			return EXIT_FAILURE;
		}

		FileWriteStream	file_stream(fp, (char*)s.GetString(), s.GetSize());
		file_stream.Flush();

		std::fclose(fp);
	}
	else
	{
		// output to std out
		printf("%s\n", s.GetString());
	}

	// Destroy the importer.
	importer->Destroy();

	// Destroy the manager
	if (manager)
	{
		manager->Destroy();
	}

	return EXIT_SUCCESS;
}
