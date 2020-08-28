
#include "fileProcess.h"


// FileProcess::FileProcess(void)
// {
//     cout << "Class Constructor" << endl;
// }


// FileProcess::~FileProcess(void)
// {
//     cout << "Class Destroy" << endl;
// }

// ----------------------------------------------------------------------------------------
// We will need to create some functions for loading data.  This program will
// expect to be given a directory structured as follows:
//    top_level_directory/
//        person1/
//            image1.jpg
//            image2.jpg
//            image3.jpg
//        person2/
//            image4.jpg
//            image5.jpg
//            image6.jpg
//        person3/
//            image7.jpg
//            image8.jpg
//            image9.jpg
//
// The specific folder and image names don't matter, nor does the number of folders or
// images.  What does matter is that there is a top level folder, which contains
// subfolders, and each subfolder contains images of a single person.

// This function spiders the top level directory and obtains a list of all the
// image files.

// auto objs = loadDirList(faces);
// cout << "objs.size(): "<< objs.size() << endl;
//
// std::vector<std::vector<string>> FileProcess::listDir (const string& dir)
// {
//     std::vector<std::vector<string>> objects;
//     for (auto subdir : directory(dir).get_dirs())
//     {
//         cout << "subdir: " << subdir << endl;

//         std::vector<string> imgs;
//         for (auto img : subdir.get_files())
//         {
//             imgs.push_back(img);
//             cout << "\timg: " << img << endl;
//         }

//         if (imgs.size() != 0)
//         {
//             objects.push_back(imgs);
//         }
//     }
//     return objects;
// }
// //-----------------------------------------------------------------------------

int listInDir(const string& loc, std::vector<directory>& dirs, std::vector<file>& files)
{
    //int result;
    try
    {
        directory test(loc);

        //cout << "directory: " << test.name() << endl;
        //cout << "full path: " << test.full_name() << endl;
        //cout << "is root:   " << ((test.is_root())?"yes":"no") << endl;

        // get all directories and files in test
        dirs = test.get_dirs();
        files = test.get_files();

        // sort the files and directories
        sort(files.begin(), files.end());
        sort(dirs.begin(), dirs.end());
        return ((test.is_root())?1:0);

        //cout << "\n\n\n";

        //cout << setw(13) << "Byte" << "    " << "Name\n\n";
        // print all the subdirectories
        //for (unsigned long i = 0; i < dirs.size(); ++i)
        //    cout << "        <DIR>    " << dirs[i].name() << "\n";

        // print all the subfiles
        //for (unsigned long i = 0; i < files.size(); ++i)
        //    cout << setw(13) << files[i].size() << "    " << files[i].name() << "\n";


        //cout << "\n\nnumber of dirs:  " << dirs.size() << endl;
        //cout << "number of files: " << files.size() << endl;
    }
    catch (file::file_not_found& e)
    {
        cout << "file not found or accessible: " << e.info << endl;
    }
    catch (directory::dir_not_found& e)
    {
        cout << "dir not found or accessible: " << e.info << endl;
    }
    catch (directory::listing_error& e)
    {
        cout << "listing error: " << e.info << endl;
    }

    //return result;
}

static void read_file(const std::string& filename, std::vector<string>& labels, std::vector<string>& parameter)
{
    char separator = ':';

    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        cout << "No valid input file was given, please check the given filename." << endl;
        //return 1;
    }

    std::string line, path, classlabel;
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) 
        {
            labels.push_back(path);
            parameter.push_back(classlabel);
        }
    }
}