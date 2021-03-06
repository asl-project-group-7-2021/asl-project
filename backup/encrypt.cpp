#define STRINGIZER(arg)     #arg
#define STR_VALUE(arg)      STRINGIZER(arg)
#define OPENSSL_PATH_STRING STR_VALUE(OPENSSL_PATH)
#define MKDIR_PATH_STRING STR_VALUE(MKDIR_PATH)
#define TAR_PATH_STRING STR_VALUE(TAR_PATH)

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <regex>
#include <cctype>

namespace fs = std::filesystem;
using namespace std;

string opensslPath = OPENSSL_PATH_STRING;
string mkdirPath = MKDIR_PATH_STRING;
string tarPath = TAR_PATH_STRING;

void mkdir(const char* dir){
  pid_t c_pid = fork();

  if (c_pid == -1) {
      throw runtime_error("couldn't fork!");
  } else if (c_pid > 0) {
      // parent process
      // wait for child  to terminate
      wait(nullptr);

      return;
  } else {
      // don't print error messages such as "Folder already exists"
      int devNull = open("/dev/null", O_WRONLY);
      dup2(devNull, STDERR_FILENO);
      // child process
      execl(mkdirPath.c_str(), "mkdir", "-p", dir, NULL);
      return;
  }
}

string readFile(string file){
  string content = "";
  string line;
  ifstream infile (file);
  if (infile.is_open()){
    while (getline(infile,line)){
      if(content == ""){
        content = line;
      }else{
        content = content + '\n' + line;
      }
    }
    infile.close();
  }else{
    throw runtime_error("Unable to open file");
  }

  return content;
}

int main(int argc, char *argv[]){
  if(argc < 9){
    throw runtime_error("Usage: ./encrypt file dir action WATCH_DIR ENC_PATH TMP_PATH CIPHER_MODE KEY_PATH");
  }

  string dir_str = argv[1];
  string file_str = argv[2];

  fs::path file(dir_str + file_str);
  string action = argv[3];
  
  fs::path WATCH_DIR(argv[4]);
  fs::path ENC_PATH(argv[5]);
  fs::path TMP_PATH(argv[6]);

  string CIPHER_MODE = argv[7];
  string KEY_PATH = argv[8];
  
  fs::path relativePath = fs::relative(file, WATCH_DIR);
  string ext = relativePath.extension().generic_string();

  fs::path fileEnc =  ENC_PATH / relativePath;
  fileEnc.replace_extension(ext + ".enc");

  fs::path ivPath = TMP_PATH / relativePath;
  ivPath.replace_extension(ext + ".IV");

  fs::path tmpEncPath = TMP_PATH / relativePath;
  tmpEncPath.replace_extension(ext + ".enc");

  mkdir(ivPath.parent_path().c_str());
  mkdir(tmpEncPath.parent_path().c_str());
  mkdir(fileEnc.parent_path().c_str());

  system(("openssl rand -hex 16 > " + ivPath.generic_string()).c_str());
  string IV = readFile(ivPath);

  pid_t c_pid = fork();

  if (c_pid == -1) {
      throw runtime_error("couldn't fork!");
  } else if (c_pid > 0) {
      // parent process
      // wait for child  to terminate
      wait(nullptr);

      c_pid = fork();

      if (c_pid == -1) {
          throw runtime_error("couldn't fork!");
      } else if (c_pid > 0) {
          wait(nullptr);

          // delete temoprary files
          unlink(ivPath.c_str());
          unlink(tmpEncPath.c_str());

          //also: after encryption of user private keys we can also delete the original files!
          if (file.generic_string().find("private/users/") != std::string::npos) {
              unlink(file.c_str());
          }

          return 0;
      } else {
          // don't print error messages such as "Removed / i"
          int devNull = open("/dev/null", O_WRONLY);
          dup2(devNull, STDERR_FILENO);
          // tar together IV and encrypted file
          fs::path p = ivPath.parent_path();
          fs::path r1 = fs::relative(ivPath, p);
          fs::path r2 = fs::relative(tmpEncPath, p);

          execl(tarPath.c_str(), "tar", "-cf", fileEnc.c_str(), "-C", p.c_str(), r1.c_str(), r2.c_str(), NULL);
          return 0;
      }
  } else {
      execl(opensslPath.c_str(), "openssl", "enc", ("-" + CIPHER_MODE).c_str(), "-a", "-A", "-iv", IV.c_str(), "-in", file.c_str(), "-out", tmpEncPath.c_str(), "-e", "-k", KEY_PATH.c_str(), NULL);
      return 0;
  }

  return 0;
}