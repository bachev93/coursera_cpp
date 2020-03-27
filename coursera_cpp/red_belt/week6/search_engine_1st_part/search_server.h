#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <string>

using namespace std;


class InvertedIndex {
public:
    InvertedIndex() = default;
    void Add(string&& document);
    const vector<pair<size_t, size_t>> Lookup(const string& word) const;

    const string& GetDocument(size_t id) const {
        return docs[id];
    }

private:
    //[слово]вектор{docID; количество повторений слова в одном документе}
    map<string, vector<pair<size_t, size_t>>> map_index;

    vector<string> docs;
};

class SearchServer {
public:
    SearchServer() = default;
    explicit SearchServer(istream& document_input);
    void UpdateDocumentBase(istream& document_input);
    void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
    InvertedIndex index;
};
