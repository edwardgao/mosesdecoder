/**
 *  Inverted document I/O and processing
 *
 *  The inverted documents are stored in the directory, with a bunch of files
 *
 *  1. The dictionary file: A text file that store the ID of word and its string form (dict)
 *  2. Sub-directory for inverted documents. (invdoc)
 *     Inside the directory, a bunch of file names from 00-99. The files are indices for word's id ending with the numbers.
 *     The number of concurrently opened files can be modified
 *  3. The corpus index file (corpus) It stores the digitized corpus file. The directory has three or more files, the first two are the index files
 *     and the last ones are the corpus file. The first index file (IDX) is basically an array of integers indicates which sentence goes to which
 *     corpus file. For compatibility, we don't allow the corpus file to go beyond 2GB.When it exceed the limit, we add a new file. The second file
 *     (MAP) is also an array of integers, it means the sentence's offset in the corpus file. Finally the corpus file, an array of integers, that
 *     are word IDs.
 *  4. Chunk re-link file. It stores the chunks written out for each word, it will make indexing much faster.
 *
 *     We limit the sentence length to 255, so we can use UCHAR to store position.
 *
 * The inverted document will store a list of the pairs:
 *
 *  (DocId , Position)
 *
 *  For every entry, the data structure is
 *
 *  TYPE ENTRY_DISK :
 *    UCHAR N_Position
 *    INT DocID
 *    UCHAR[] Positions
 *  END

 *  TYPE ENTRY_MEMORY :
 *    INT DocID
 *    INT Position
 *  END
 *
 *  In memory, during indexing, it is basically a linked list on a large buffer
 *
 *  TYPE MEMORY_ENTRY
 *    INT WordID
 *    ENTRY Entry
 *    POINTER next_entry_memory
 *  END
 *
 *  Each entry in the dictionary (in memory) will hold:
 *
 *  TYPE DICT_ENTRY_MEMORY
 *     INT WordID
 *     CHAR* Word
 *     POINTER FIRST_MEMORY_ENTRY
 *     INT ENTRY_COUNT
 *     INT CHUNKS_WRITTEN
 *  END
 *
 *  When indexing, we want to save on disk IO, so we want to store as much words as possible in memory, so we will keep tracking
 *  the word with maximum number of entries in the buffer, and when the buffer is full, we flush the very word into the File.
 *
 *  In disk storage is as follows:
 *
 *  TYPE ENTRYBLOCK
 *     INT WordID
 *     INT BlockID
 *     INT N_Bytes
 *     ENTRY_DISK[] Entries
 *  END
 *
 *  After flushing out the entries, we set the in-memory entries' WordID to 0, and the position can be reused. The pointer to the last available
 *  inserting point will be set to the first entry that is freed, all operations later on check if the position is available.
 */

#ifndef __INVDOC_H__
#define __INVDOC_H__
#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#ifdef USE_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
typedef std::tr1::unordered_map<std::string,int, boost::hash<std::string> > DictType;
#endif


#ifdef USE_UNORDERED_MAP
#include <unordered_map>
typedef std::unordered_map<std::string,int, boost::hash<std::string> > DictType;
#endif





namespace cindex{

struct MemoryEntry{
	mutable int WordID;
	int DocID;
	int Position;
	int NextEntry; // Index of next entry
	MemoryEntry();
};

// Lexicon entry of the memory
struct MemLexiconEntry{
	int WordID; // ID, 1-based
	std::string WordRep; // Actual word
	int FirstMemoryEntry; // The index of the first memory entry
	int LastMemoryEntry; // The index of the last memory entry;
	mutable int EntryCount; // Number of entries that has been indexed
	MemLexiconEntry();
	MemLexiconEntry(const MemLexiconEntry& /*entry*/);
	MemLexiconEntry(int /*wid*/ , int /*fme*/, const char* /*rep*/);
};



class InvertedDocument;
// Memory dictionary/buffer for indexing
class MemBufferForIndexing{
public:
	friend class InvertedDocument;
	MemBufferForIndexing(int /*buffer_size*/);
	inline int GetBufferSize(){return bufferSize;};
public:
	int IndexNewEntry(const std::string& /*word*/, int /*position*/, int /*docid*/, bool & /*need_flush*/);
	int GetFlushingWordID();
	inline MemoryEntry& GetBufferEntry(int idx){return buffer[idx];};
	inline MemLexiconEntry& GetLexiconEntry(int idx){return lexicon[idx-1];};
	void Flushed(int /*nextAvail*/); // Call after flushed
	void DumpLexicon(std::ostream& ofs) const;
private:

	int bufferSize;
	int lastId; // Last dictionary id
	int nextAvailableIndex; // Next available position in the buffer
	std::vector<MemoryEntry> buffer;
	std::vector<MemLexiconEntry> lexicon;
	DictType dict;
	int maxEntryCount;
	int maxCountId;

private:
	
};

class MemoryBufferForQuery{
public:

	void LoadLexicon(std::istream& ifs);
	int QueryWordID(const std::string& word);

	inline const std::string& GetWord(int i){return lexicon[i-1];};

	void LoadCorpusBounds(FILE* fl);
	void LoadChunkRelink(FILE* fl);
	const std::vector<std::pair<long,int> > * GetChunkForWord(int wid);
	const int GetCorpusSplitForSent(int sid);

private:

	std::vector<std::string> lexicon;
	DictType dict;
	std::vector<int> corpusBounds; // The boundries of courpus files

	std::vector<std::vector<std::pair<long,int> > > chunkRelink;// The chunk relink table

};

struct ResultEntry{
	int docID;
	int start;
	int end;
};

struct QueryState{
	unsigned char* phead;
	unsigned char* buffer;
	int len;
	unsigned char* pos; // current position
	int docid; // current document id
	std::vector<int> positions ; // current position ids
	bool position_loaded; // If position is loaded
	bool NextDocument();
	void LoadPositions();

	QueryState(){buffer = pos = NULL; position_loaded = false; len = docid = -1;positions.reserve(256);}
	~QueryState(){if(buffer)delete[] phead;}
};

/**
The main class for inverted document
*/
class InvertedDocument{
public:
	InvertedDocument(bool /*index*/, const char* /*root*/, int __splits = 10, int __buffer_size=5000000);
	
public:
	// API functions
	int IndexNewDoc(const char* /*str*/);  // index the document, return document id
	void FlushAll();
	void Close();


public:
	// Query function, query phrases with n left context and n right context, if left context / right context
	// has less words than required, then there must be beginning of the sentence
	std::vector<std::string> QueryContext(const char* /*left_context*/, int /*nleft_words*/, const char* /*righ_context*/, int /*nright_words*/, int length_limit_min = -1, int length_limit_max = -1);
	std::vector<std::string> QueryContext(const std::vector<std::string>& /*left_context*/, int /*nleft_words*/, const std::vector<std::string>& /*righ_context*/, int /*nright_words*/, int length_limit_min = -1 , int length_limit_max = -1);

	bool QueryEvidence(const std::vector<std::string>& /*left_context*/, int /*nleft_words*/, const std::vector<std::string>& /*righ_context*/, int /*nright_words*/);

	inline int GetTotalDocuments()const{return totalDocuments;} ;
protected:
	// Flush
	int FlushBuffer(float min_space = 0.02);
	int FlushBufferAll();
	FILE* GetSplit(int i);
	FILE* GetSplitForWid(int wid);
	FILE* OpenFile(const char* /*relpath*/);
	void WriteMemEntry(const MemLexiconEntry& /*ent*/, FILE* /*fl*/);
	void WriteRelinkInfo(int /*word_id*/,  long /*offset*/, int /*len*/);
	void WriteCorpus(const std::list<int>& /*wids*/);

	QueryState* LoadInvDocForWid(int wid);

	// Query, return the results
	void FetchList(const std::vector<int>& /*left_context*/, int /*nleft_words*/, const std::vector<int>& /*righ_context*/, int /*nright_words*/,
			int length_limit_min , int length_limit_max , std::list<ResultEntry>& results, bool h_l, bool h_r);

	// return true if we should continue (and the result is valid)
	bool FindNextDocument(std::vector<QueryState* >& l_buffer, std::vector<QueryState* >& r_buffer,std::vector<QueryState* >& c_buffer, std::list<ResultEntry>& ret, int l_min, int l_max, bool h_l, bool h_r, bool evidence_only = false);

	
	QueryState* GetMinStateAndBound(std::vector<QueryState*> &l_buffer,std::vector<QueryState*> &r_buffer, int& bound, bool &is_left);

	std::list<std::pair<int,int> > GetContinuousRegions(std::vector<QueryState* >& buffer);

	std::string RecoverSentence(const ResultEntry & e);

	void GetSentenceInfo(int sid, long& offset, unsigned char& len);
private:	
	int totalDocuments;
	MemBufferForIndexing buffer;
	MemoryBufferForQuery qbuffer;
	unsigned char totalSplits; // The total number of split for invdoc
	bool indexMode; // Are we in indexing model or retrieval model?
	std::string rootDir; // The directory root;
	boost::filesystem::path rootDirPath;
	FILE *corpusMaster, *corpusSentMaster;  // Indices of corpus base
	std::vector<FILE*> corpusHandles; // The handles of actual corpus file
	std::vector<FILE*> invDocs; // The handles of inverted documents
	FILE *chunkRelink; // The chunk relink file
};

}
#endif
