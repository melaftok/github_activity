#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <iterator>

#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

// Replace 'YOUR_GITHUB_USERNAME' with your GitHub login
const std::string githubUsername = "YOUR_GITHUB_USERNAME";

// Replace 'YOUR_GITHUB_TOKEN' with your GitHub token
const std::string githubToken = "YOUR_GITHUB_TOKEN";

// Function for making HTTP requests to GitHub API
std::string executeCommand(const std::string &command) {
     char buffer[128];
     std::string result = "";
     FILE *pipe = popen(command.c_str(), "r");
     if (!pipe) throw std::runtime_error("popen() failed!");

     try {
         while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
             result += buffer;
         }
     } catch (...) {
         pclose(pipe);
         throw;
     }

     pclose(pipe);
     return result;
}

// Function to get a list of user repositories
std::vector<std::string> getRepositories() {
     std::string command = "curl -s -H \"Authorization: token " + githubToken +
                           "\" https://api.github.com/users/" + githubUsername + "/repos";
     std::string response = executeCommand(command);
     std::vector<std::string> repositories;

     size_t pos = response.find("\"full_name\":");
     while (pos != std::string::npos) {
         size_t start = response.find_first_of("\"", pos + 13) + 1;
         size_t end = response.find_first_of("\"", start);
         repositories.push_back(response.substr(start, end - start));
         pos = response.find("\"full_name\":", end);
     }

     return repositories;
}

// Function to get repository languages
std::map<std::string, int> getRepositoryLanguages(const std::string &repository) {
     std::string command = "curl -s -H \"Authorization: token " + githubToken +
                           "\" https://api.github.com/repos/" + repository + "/languages";
     std::string response = executeCommand(command);
     std::map<std::string, int> languages;

     size_t pos = response.find_first_of("{");
     while (pos != std::string::npos) {
         size_t start = response.find_first_of("\"", pos + 1) + 1;
         size_t end = response.find_first_of("\"", start);
         std::string language = response.substr(start, end - start);

         start = response.find_first_of(":", end) + 1;
         end = response.find_first_of(",", start);
         int bytes = std::stoi(response.substr(start, end - start));

         languages[language] += bytes;
         pos = response.find_first_of("{", end);
     }

     return languages;
}

// Main function
int main() {
     std::vector<std::string> repositories = getRepositories();
     std::map<std::string, int> languages;

     for (const auto &repository : repositories) {
         std::map<std::string, int> repoLanguages = getRepositoryLanguages(repository);
         for (const auto &[language, bytes] : repoLanguages) {
             languages[language] += bytes;
         }
     }

     std::vector<std::string> labels;
     std::vector<int> sizes;

     for (const auto &[language, bytes] : languages) {
         labels.push_back(language);
         sizes.push_back(bytes);
     }

     plt
