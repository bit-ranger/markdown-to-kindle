#include <fstream>
#include <filesystem>
#include <iostream>

using namespace std;


void streamCopy(std::istream& in, std::ostream& out, unsigned long count)
{
	const unsigned long bufferSize = 4096;
	char buffer[bufferSize];
	while (count > bufferSize)
	{
		in.read(buffer, bufferSize);
		out.write(buffer, bufferSize);
		count -= bufferSize;
	}

	in.read(buffer, count);
	out.write(buffer, count);
	out.flush();
}


void insertBom(ofstream& outFile)
{
	unsigned char bom[] = { 0xEF,0xBB,0xBF };
	outFile.write(reinterpret_cast<char*>(bom), sizeof(bom));
}

void append(ofstream& outFile, ifstream& inFile)
{
	inFile.seekg(0, ios::end);
	const auto endg = inFile.tellg();

	inFile.seekg(0, ios::beg);
	const auto begg = inFile.tellg();

	streamCopy(inFile, outFile, endg - begg);
}


int main(int argc, char* argv[])
{
	const string targetDir = string(argv[1]);
	const string targetName = string(argv[2]);
	string srcPathMerged = targetDir + "\\" + targetName + ".md";
	ofstream srcFileMerged(srcPathMerged, ios::ate | ios::binary);

	for (const auto& entry : filesystem::directory_iterator(targetDir))
	{
		if (entry.is_directory())
		{
			continue;
		}
		string srcPath = entry.path().string();
		if (srcPath.rfind(".md") != (srcPath.length() - string(".md").length()))
		{
			continue;
		}
		if (srcPath == srcPathMerged)
		{
			continue;
		}

		ifstream srcFile(srcPath, ios::binary);
		append(srcFileMerged, srcFile);
		srcFileMerged << endl;
		srcFile.close();
	}
	srcFileMerged.close();

	string genPathMerged = targetDir + argv[2] + ".html.tmp";
	system(("bin\\pandoc.exe -s -f gfm -t html --toc  -o " + genPathMerged + " " + srcPathMerged).c_str());

	ifstream genFileMerged(genPathMerged, ios::ate | ios::binary);
	string genPathMergedBom = targetDir + "\\" + targetName + ".html";
	ofstream genFileMergedBom(genPathMergedBom, ios::ate | ios::binary);
	insertBom(genFileMergedBom);
	append(genFileMergedBom, genFileMerged);


	genFileMerged.close();
	genFileMergedBom.close();

	system(("bin\\kindlegen.exe " + genPathMergedBom).c_str());

	filesystem::remove(srcPathMerged);
	filesystem::remove(genPathMerged);
	filesystem::remove(genPathMergedBom);

	return 0;
}

