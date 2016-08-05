/*
	[DRM] Tomb Raider: Legend/Anniversary/Underworld DRM Unpacker
	Copyright (C) Gh0stBlade 2015 - gh0stblade@live[dot]co.uk

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
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "DRM.h"
#include "File.h"
#include "FileExtensions.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <assert.h>

void cDRM::ExtractSections(char* szFilePath)
{
	//Store DRM input path
	this->szFilePath = szFilePath;

	//Initialise ifstream for reading in binary mode
	std::ifstream ifs(this->szFilePath, std::ios::binary);

	//If it's not good to go
	if (!ifs.good())
	{
		std::cout << "Fatal Error: Failed to open file at path: " << this->szFilePath << std::endl;
		return;
	}

	//Read our DRM header into cDRM
	this->m_version = ReadUInt(ifs);
	if (this->m_version != DRM_VERSION)
	{
		std::cout << "Fatal Error: Version mis-match! expected: " << DRM_VERSION << " got: " << this->m_version << std::endl;
		return;
	}

#if TR8
	this->uiNameSize = ReadUInt(ifs);
	this->uiPaddingSize = ReadUInt(ifs);
	this->uiUnk00 = ReadUInt(ifs);
	this->uiUnk01 = ReadUInt(ifs);
#endif

	this->m_numSections = ReadUInt(ifs);
	if (this->m_numSections > DRM_MAX_SECTIONS)
	{
		std::cout << "Fatal Error: Number of Sections: " << this->m_numSections << " exceeded the limit of: " << DRM_MAX_SECTIONS << "!" << std::endl;
		return;
	}

	//If not enough sections
	if (this->m_numSections <= 0)
	{
		std::cout << "Fatal Error: Number of Sections <= 0 !" << std::endl;
		return;
	}

	//Read all the section info into this
	for (int i = 0; i != this->m_numSections; i++)
	{
		this->m_sections.emplace_back();
		Section* section = &this->m_sections[this->m_sections.size() - 1];

		section->uiSize = ReadUInt(ifs);
		section->ucType = ReadUByte(ifs);
		section->ucUnk00 = ReadUByte(ifs);
		section->usUnk01 = ReadUShort(ifs);
		section->uiHeaderSize = ReadUInt(ifs);
		section->uiHash = ReadUInt(ifs);
		section->uiLang = ReadUInt(ifs);
	}

#if TR8
	//Skip past names & padding info
	ifs.seekg(this->m_nameSize + this->m_paddingSize, SEEK_CUR);
#endif

	std::string strOutPath = std::string(this->szFilePath);
	strOutPath = strOutPath.substr(0, strOutPath.find_last_of(".")) + "\\";;

	//Create Directories
	CreateDirectories(strOutPath);

	for (int i = 0; i != this->m_numSections; i++)
	{
		Section* section = &this->m_sections[i];

		//Print extraction message to console
		std::cout << "Extracting Section: " << "[ " << (i + 1) << " / " << this->m_numSections << " ]" << std::endl;

		//Define output file path
		std::stringstream strOutPath2;
		strOutPath2 << strOutPath << std::hex << i << szExtensions[section->ucType];

		//Skip header
		bool bRealSize = false;
		if (bRealSize)
		{
			//Declare variables to store data
			char* szSectionData = new char[section->uiSize];

			//Declare output file stream
			std::ofstream ofs(strOutPath2.str(), std::ios::binary);

			//If not good to go
			if (!ofs.good())
			{
				std::cout << "Fatal Error: Unknown error occured whilst initialising ofstream!" << std::endl;
				return;
			}

			//Skip header info
#if TR7 || TRAE
			ifs.seekg(((section->uiHeaderSize >> 0x8) * 0x8), SEEK_CUR);
#elif TR8
			ifs.seekg((section->uiHeaderSize >> 0x8), SEEK_CUR);
#else
#error "Unsupported Game!"
#endif
			//Read then write the section data
			ifs.read(szSectionData, section->uiSize);
			ofs.write(szSectionData, section->uiSize);
			
			//Flush and close ofstream
			ofs.flush();
			ofs.close();

			//delete allocated section data
			delete[] szSectionData;
		}
		else
		{
			//Declare char* to store section data
#if TR7 || TRAE
			char* szSectionData = new char[section->uiSize + ((section->uiHeaderSize >> 0x8) * 0x8)];
#elif TR8
			char* szSectionData = new char[section->uiSize + (section->uiHeaderSize >> 0x8)];
#endif

			//Declare output file stream
			std::ofstream ofs(strOutPath2.str(), std::ios::binary);

			//If not good to go
			if (!ofs.good())
			{
				std::cout << "Fatal Error: Unknown error occured whilst initialising ofstream!" << std::endl;
				return;
			}

			//Read then write the section data
#if TR7 || TRAE
			ifs.read(szSectionData, section->uiSize + ((section->uiHeaderSize >> 0x8) * 0x8));
			ofs.write(szSectionData, section->uiSize + ((section->uiHeaderSize >> 0x8) * 0x8));
#elif TR8
			ifs.read(szSectionData, section->uiSize + (section->uiHeaderSize >> 0x8));
			ofs.write(szSectionData, section->uiSize + (section->uiHeaderSize >> 0x8));
#else
#error "Unsupported Game!"
#endif
			//Flush and close ofstream
			ofs.flush();
			ofs.close();

			//Delete allocated section data
			delete[] szSectionData;
		}
	}

	//Close ifstream
	ifs.close();

	//Print success
	std::cout << "Successfully Extracted: " << "[ " << (this->m_numSections) << " ] " << " section(s)!" << std::endl;
}

void cDRM::Destroy()
{
	if (this->m_sections.size() > 0)
	{
		this->m_sections.clear();
	}
}