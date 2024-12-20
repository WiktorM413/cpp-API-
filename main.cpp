#include <iostream>
#include <curl/curl.h>
#include <rapidjson/document.h>    // For parsing JSON
#include <rapidjson/prettywriter.h> // For pretty-printing JSON
#include <rapidjson/stringbuffer.h> // For converting JSON to string
#include <fstream>
using namespace std;
using namespace rapidjson;

/*
    WriteCallBack function explained:
    void* contents -> its the data the buffer recieves
    size_t size -> its the size of one data element
    size_t nmemb -> its the number of data elements
    void* userp -> a user pointer so if we want to store the contents somewhere we can store it through reference

    Each line explained:
        WriteFileCallBack():
           line 1: We cast the userp pointer (which is a void*) to an ofstream*, and use it to access the output file stream
           line 2: we calculate the total size because we need to return it
           line 3: we write to the content recieved to our file
           line 4: we return the total size (it is a must)

        WriteStringCallBack():
           line 1: we calculate the total size
           line 2: we append the argument in which we will write the data
           line 3: we return the total size (once again it is a must)
*/


size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ofstream* outFile = static_cast<ofstream*>(userp);
    size_t totalSize = size * nmemb;
    outFile->write(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t WriteStringCallback(void* contents, size_t size, size_t nmemb, string* out)
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}


string GetJSONFromApiLink(const char* APILink)
{
    CURL* curl;
    CURLcode res;
    string response;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, APILink);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            cerr << "Error while accessing API" << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response;
}

char* GetDataFromJSON(const string JSON, const char* name)
{
    Document document;
    document.Parse(JSON.c_str());
    if (document.HasParseError())
    {
        cerr << "Error while Parsing JSON" << endl;
        return 0;
    }
    return (char*)(document[name].GetString());
}

void DownloadImageToFolder(const char* path, const char* url)
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    ofstream File(path, ios::binary);
    if (curl)
    {
        if (!File.is_open())
        {
            cerr << "Could not open file\n";
            return;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &File);


        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            cerr << "Curl error: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    File.close();
}
string GetIMGExtensionFromURL(const char* URL)
{
    string s = URL;
    int index = -1;
    for (int i = s.length() - 1; i >= 0; --i)
    {
        if (s[i] == '.')
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        return "";
    }
    string extension = "";
    extension.append(s, index, s.length() - 1);
    return extension;
}
int main()
{
    const char* url = "https://api.waifu.pics/sfw/waifu";
    string folderPath;
    int amount;
    cout << "Type exact path to folder where you wish to store images:\n";
    cin >> folderPath;
    cout << "Now choose amount of pictures to generate(images may repeat)\n";
    cin >> amount;

    for (int i = 0; i < amount; i++)
    {
        string JSON = GetJSONFromApiLink(url);
        char* ImgURL = GetDataFromJSON(JSON, "url");
        string filename = "img" + to_string(i + 1);
        string extension = GetIMGExtensionFromURL(ImgURL);
        cout << extension << endl;
        string filepath = folderPath + filename + extension;
        cout << JSON << "\n\n\n";

        DownloadImageToFolder(filepath.c_str(), ImgURL);
    }

    return 0;
}
