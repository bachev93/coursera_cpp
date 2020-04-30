#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <future>
#include <mutex>
#include <deque>

using namespace std;


struct Entry {
    size_t docID_, hitcount_;
};

class InvertedIndex {
public:
    InvertedIndex() = default;
    void Add(string&& document);
    const vector<Entry>& Lookup(const string_view& word) const;
    size_t getDocsSize() const;

    size_t GetNumDocs() const {
        return docs.size();
    }

private:
    map<string_view, vector<Entry>> index_;
    deque<string> docs;
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
    vector<future<void>> futures_;
    bool firstDocUpdate = true;
};
