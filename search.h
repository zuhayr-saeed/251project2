#pragma once

#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/**
 * Cleans a specific token by:
 * - Removing leading and trailing punctuation
 * - Converting to lowercase
 * If the token has no letters, returns the empty string.
 */
string cleanToken(string s) {
    int tokenLength = s.length();
    string cleanToken = s;
    int alphaCount = 0;

    // Count the number of alphabetic characters in the token.
    for (int i = 0; i < cleanToken.size(); i++){
        if (isalpha(s[i])){
            alphaCount += 1;
        }
    }   

    // Return empty string if no alphabetic characters are found.
    if (alphaCount == 0){
        return "";
    } 

    // Remove leading punctuation.
    while (!cleanToken.empty() && ispunct(cleanToken[0])) {
        cleanToken = cleanToken.substr(1);
        tokenLength = cleanToken.length();
    }

    // Remove trailing punctuation.
    while (!cleanToken.empty() && ispunct(cleanToken[tokenLength - 1])) {
        cleanToken = cleanToken.substr(0, tokenLength - 1);
        tokenLength = cleanToken.length();
    }

    // Convert all characters to lowercase.
    for (int i = 0; i < cleanToken.size(); i++){
        cleanToken[i] = tolower(cleanToken[i]);
    }

    return cleanToken;
}

/**
 * Returns a set of the cleaned tokens in the given text.
 */
set<string> gatherTokens(string text) {
    set<string> tokens;
    string currentWord = "";

    // Iterate over each character in the text.
    for (int i = 0; i < text.size(); i++) {
        if (!isspace(text[i])){
            currentWord += text[i];
        }
        else {
            if (!currentWord.empty()){
                string cleanedWord = cleanToken(currentWord);
                tokens.insert(cleanedWord);
                currentWord = "";
            }
        }
    }

    // Add the last word if it's not empty.
    if (!currentWord.empty()){
        currentWord = cleanToken(currentWord);
        tokens.insert(currentWord);
    }

    // Remove empty string from the set if it exists.
    if (tokens.count("") == 1){
        tokens.erase("");
    }  

    return tokens; 
}

/**
 * Builds the inverted index from the given data file. The existing contents
 * of `index` are left in place.
 */
int buildIndex(string filename, map<string, set<string>>& index) {
    ifstream file(filename);

    // Return 0 if file cannot be opened.
    if (!file.is_open()){
        return 0;
    }

    set<string> tokens;
    string line;
    string currentPage;
    int pageCount = 0;

    // Process each line of the file.
    while(getline(file, line)){
        // Check if line is a URL.
        if(line.find("www.") == 0 || line.find("https") == 0){
            currentPage = line; 
            pageCount += 1;
        }
        else {
            string content = line;

            // Tokenize the content.
            tokens = gatherTokens(content);

            // Add tokens to the index.
            for (const string& token : tokens){
                index[token].insert(currentPage);
            }
        }
    }
    return pageCount;

}

/**
 * Runs a search query on the provided index, and returns a set of results.
 *
 * Search terms are processed using set operations (union, intersection, difference).
 */
set<string> findQueryMatches(map<string, set<string>>& index, string query) {

    set<string> result;
    vector<string> queryWords;
    string currentWord;

    // Split the query into words.
    for (int i = 0; i < query.size(); i++) {
        if (query[i] != ' ') {
            currentWord += query[i];
        } else {
            if (!currentWord.empty()) {
                queryWords.push_back(currentWord);
                currentWord.clear(); // Reset word after adding it to queryWords
            }
        }
    }

    if (!currentWord.empty()) {
        queryWords.push_back(currentWord);
    }

    bool isFirstTerm = true;
    bool termNotFound = false;

    // Process each word in the query.
    for (int i = 0; i < queryWords.size(); i++) {
        string queryTerm = queryWords[i];

        // Check if the term starts with a modifier.
        char modifier = queryTerm[0];
        if (modifier == '-' || modifier == '+') {
            queryTerm = queryTerm.substr(1);
        }

        // Clean the query term.
        queryTerm = cleanToken(queryTerm);

        if (index.find(queryTerm) == index.end()) {
            termNotFound = true;
            break;
        }

        // Continue if term not in index.
        if (index.find(queryTerm) == index.end()) {
            continue;
        }

        // Initialize result with the first term if not a negation.
        if (isFirstTerm && modifier != '-') {
            result = index[queryTerm];
            isFirstTerm = false;
        } else if (modifier == '+') {
            // Set intersection for '+'
            set<string> tempResult;
            for (const auto& url : result) {
                if (index[queryTerm].find(url) != index[queryTerm].end()) {
                    tempResult.insert(url);
                }
            }
            result = tempResult;
        } else if (modifier == '-') {
            // Set difference for '-'
            for (auto it = result.begin(); it != result.end();) {
                if (index[queryTerm].find(*it) != index[queryTerm].end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        } else {
            // Union for terms without modifiers after the first term.
            for (const auto& url : index[queryTerm]) {
                result.insert(url);
            }
        }
    }

    // If any term in the query was not found, return an empty set
    if (termNotFound) {
        return set<string>();
    }

    return result;
}

/**
 * Runs the main command loop for the search program
 */
void searchEngine(string filename) {
    map<string, set<string>> index;
    string userInputQuery;

    // Build the index from the file.
    int numIndexedPages = buildIndex(filename, index);
    if (numIndexedPages == 0) {
        cout << "Invalid filename" << endl;
    }
    cout << "Stand by while building index..." << endl;
    cout << "Indexed " << numIndexedPages << " pages containing " << index.size() << " unique terms" << endl;
    
    // Command loop for querying.
    do {
        cout << "\nEnter query sentence (press enter to quit): ";
        getline(cin, userInputQuery);

        // Check for exit condition.
        if (userInputQuery.empty()) {
            cout << "Thank you for searching!" << endl;
            return;
        }

        // Process the query and display results.
        set<string> results = findQueryMatches(index, userInputQuery);
        cout << "Found " << results.size() << " matching pages" << endl;
        for (const string& url : results) {
            cout << url << endl;
        }
    } while (true);

    // Closing message.
    cout << "Thank you for searching!" << endl;
}
