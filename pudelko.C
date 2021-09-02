#include <iostream>
#include <sstream>
#include <vector>
std::vector<std::string> split (const string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;
    while (getline (ss, item, delim)) {
        result.push_back (item);
    }
    return result;
}
void listFiles(const TSystemDirectory &dir, std::vector<TString> &vecPaths, TString extension) {
	TList *files = dir.GetListOfFiles();
	if (files) {
		TSystemFile *file;
		TString fname;
		TIter next(files);
		while ((file=(TSystemFile*)next())) {
			fname = file->GetName();
            TString fullpath = TString(dir.GetName() ) + TString("/") + fname;
            if(fname.CompareTo("..") == 0 || fname.CompareTo(".") == 0) continue;
			if (!file->IsDirectory() && fname.EndsWith(extension) ) {
				vecPaths.push_back(fullpath);
			} else {
				TSystemDirectory dir0(fullpath, fullpath);
                //recurse here
				listFiles(dir0, vecPaths, extension);
			}
		}

	}
}
/**
 * if you execute this after your analysis which produces pdf files, this function with update the index.html.
 *
 */
void pudelko() {
    UInt_t url_path_index = 18;
    UInt_t filename_index = 26;
	std::vector<TString> vecPaths;
	TString pathName = "/scratch/gccb/data";
	TSystemDirectory dir(pathName, pathName);
	listFiles(dir, vecPaths, ".pdf");

    //monitor current acquisition state
//    TString body("<p>Current wave_0.dat filesize: <span id='monitor'></span></p>");
    TString body;
    /**
     *  temporarily used until a database is available
     */
    std::vector<TString> vecLogs;
    listFiles(dir, vecLogs, ".txt");
    for(UInt_t i=0; i < vecLogs.size(); ++i) {
        if(vecLogs[i].Contains("log")  ) {
            body.Append("<p><a href='" + vecLogs[i](url_path_index, vecLogs[i].Length() ) + "'>" + vecLogs[i](filename_index, vecLogs[i].Length() ) + "</a></p>");
        }
    }
    /**
     *  /temporarily used until a database is available
     */

    TString h1, prev_h1;
    for(UInt_t i=0; i < vecPaths.size(); ++i) {
        std::vector<std::string> vecToken = split(vecPaths[i].Data(), '/');
        h1 = "<h1>" + vecToken[4] + "</h1>";
        if(prev_h1.CompareTo(h1) != 0)
            body.Append(h1);

        body.Append("<p><a href='" + vecPaths[i](url_path_index, vecPaths[i].Length() ) + "'>" + vecPaths[i](filename_index, vecPaths[i].Length() ) + "</a></p>");
        prev_h1 = h1;
    }

    std::ofstream index;
    //still an early prototype
    index.open("/scratch/gccb/data/index.html");
    //javascript for updating the monitor span element, monitoring is started with /scratch/gccb/mark/watch/watch.sh
    TString javascript("<script>function readTextFile(file){var rawFile = new XMLHttpRequest();rawFile.overrideMimeType('text/plain'); rawFile.open('GET', file, false);rawFile.onreadystatechange = function (){if(rawFile.readyState === 4){if(rawFile.status === 200 || rawFile.status == 0){var allText = rawFile.responseText; document.getElementById('monitor').innerHTML = allText;}}} \n rawFile.send(null);} readTextFile('daqstatus');</script>");
    TString header("<!DOCTYPE html><meta charset='UTF-8'><html><head></head><body>");
//    TString footer(javascript + "</body></html>");
    TString footer("</body></html>");
    TString page = header;
    page.Append(body);
    page.Append(footer);
    index << page.Data();
    index.close();
}
