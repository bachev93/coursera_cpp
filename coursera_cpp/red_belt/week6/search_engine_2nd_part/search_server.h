#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <future>
#include <mutex>
#include <deque>

#include "synchronized.h"

using namespace std;

class InvertedIndex {
public:
    void Add(string&& document);
    const vector<pair<size_t, size_t>>& Lookup(const string& word) const;

    const string& GetDocument(size_t id) const {
        return docs[id];
    }

    size_t GetNumDocs() const {
        return docs.size();
    }

private:
    //[слово]вектор{docID; количество повторений слова в одном документе}
    map<string, vector<pair<size_t, size_t>>> map_index;

    //[слово][docID] = индекс вектора пар для конкретного слова и документа
//    map<string, map<size_t, size_t>> indexes;

    vector<string> docs;

    vector<pair<size_t, size_t>> empty_lookup_;
};

class SearchServer {
public:
    SearchServer() = default;
    explicit SearchServer(istream& document_input);
    void UpdateDocumentBase(istream& document_input);
    void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
    InvertedIndex index;
    mutex mut_;
//    Synchronized<InvertedIndex> index;
    vector<future<void>> futures_;
    bool firstDocUpdate = true;
};
