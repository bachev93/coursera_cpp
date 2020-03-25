#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <set>

using namespace std;

class InvertedIndex {
public:
    void Add(const string& document);
    vector<pair<size_t, size_t>> Lookup(const string& word) const;

    const string& GetDocument(size_t id) const {
        return docs[id];
    }

    void removeDuplicates();

private:
    //[слово]вектор{docID; количество повторений слова в одном документе}
    map<string, vector<pair<size_t, size_t>>> orig_index;

    //[слово][docID] = индекс вектора пар для конкретного слова и документа
    map<string, map<size_t, size_t>> indexes;

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
