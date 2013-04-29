
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <fstream>
#include <memory.h>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include "SimpleLogger.h"

#include "invdoc.hpp"


#pragma warning(disable : 4018)

namespace cindex
{
	using namespace std;

	using namespace boost;
	using namespace boost::filesystem;
	
	inline void fclose_ss(FILE* f){	if(f)fclose(f);}

	MemoryEntry::MemoryEntry() : WordID(0), DocID(0), Position(0), NextEntry(0) {};
	MemLexiconEntry::MemLexiconEntry(): WordID(0), FirstMemoryEntry(-1), EntryCount(0) {};
	MemLexiconEntry::MemLexiconEntry(const MemLexiconEntry& entry){
		WordID = entry.WordID;
		FirstMemoryEntry = entry.FirstMemoryEntry;
		WordRep = entry.WordRep;
		EntryCount = entry.EntryCount;
		LastMemoryEntry = entry.LastMemoryEntry;
	}
	MemLexiconEntry::MemLexiconEntry(int wid, int fme, const char* rep)
		:WordID(wid), FirstMemoryEntry(fme), WordRep(rep), EntryCount(0), LastMemoryEntry(fme){}

	bool QueryState::NextDocument(){
		if(!position_loaded && docid>=0){
			LoadPositions();// Bypass
		}

		position_loaded = false;
		positions.clear();
		docid = -1;
		if(pos>=buffer)
			return false;
		int *did = (int*)pos;
		docid = *did;
		pos += sizeof(int);
		return true;
	}

	void QueryState::LoadPositions(){
		if(position_loaded) return;
		positions.resize(*pos);
		pos++;
		for(size_t i = 0; i< positions.size() ; i++)
			positions[i] = *(pos++);		
		position_loaded = true;
	}

	const int MemoryBufferForQuery::GetCorpusSplitForSent(int sid){
		for(size_t i = 0 ; i< corpusBounds.size(); i++){
			if(sid >= corpusBounds[i])
				return i;
		}
	}

	const vector<pair<long,int> > * MemoryBufferForQuery::GetChunkForWord(int wid){
		return &(chunkRelink[wid-1]);
	}

	void MemoryBufferForQuery::LoadCorpusBounds(FILE* fl){
		corpusBounds.reserve(255);
		int buffer = 0;
		while(!feof(fl)){
			if(1!=fread(&buffer,sizeof(int),1,fl)){
				break;
			}
			corpusBounds.push_back(buffer);
		}
	}

	void MemoryBufferForQuery::LoadChunkRelink(FILE* fl){
		if(lexicon.size()>0){
			chunkRelink.reserve(lexicon.size());
		}
		int wid;
		long offset;
		int len;
		while(!feof(fl)){
			if(1!=fread(&wid,sizeof(int),1,fl))
				break;
			if(1!=fread(&offset,sizeof(long),1,fl))
				P_FATAL("Error reading chunk re-link file");
			if(1!=fread(&len,sizeof(int),1,fl))
				P_FATAL("Error reading chunk re-link file");

			if(wid > chunkRelink.size())
				chunkRelink.resize(wid);
			if(wid<=0){
				P_FATAL("WID==0");
			}
			chunkRelink[wid-1].push_back(pair<long,int>(offset,len));
			
		}
	}

	void MemoryBufferForQuery::LoadLexicon(istream& ifs){
		string line;
		vector<string> vct;
		vct.reserve(2);
		P_INFO("Loading Lexicon...");
		int nline = 0;
		while(ifs){
			line = "";
			getline(ifs, line);
			split(vct,line,is_any_of(" "),token_compress_on);
			if(vct.size()<2){
				continue;
			}
			size_t lineno = lexical_cast<int>(vct[0]);
			if(lineno < (size_t) nline + 1){
				P_FATAL("Mismatch (duplicate) line number, line (%d) \"%s\"",nline+1, line.c_str());
			}else if(lineno > nline + 1){
				P_ERROR("Mismatch (extra) line number, line (%d) \"%s\"",nline+1, line.c_str());
				while(lexicon.size() < lineno-1){
					lexicon.push_back(string());
				}
			}
			lexicon.push_back(vct[1]);
			dict.insert(DictType::value_type(vct[1],lineno));
			nline = lineno;
		}
		P_INFO("Loaded %d entries in lexicon", nline);
	}

	int MemoryBufferForQuery::QueryWordID(const string& word){
		DictType::const_iterator it = dict.find(word);
		if(it == dict.end()){
			return -1;
		}
		return it->second;
	}

	void MemBufferForIndexing::DumpLexicon(ostream & ofs) const{
		for(size_t i = 0; i< lexicon.size() ; i++){
			ofs << lexicon[i].WordID << " " << lexicon[i].WordRep << endl;
		}
	}

	int MemBufferForIndexing::GetFlushingWordID(){
		return maxCountId;
	}

	MemBufferForIndexing::MemBufferForIndexing(int buffer_size){
		bufferSize = buffer_size;
		buffer.resize(buffer_size);
		lexicon.reserve(65536);
		lastId = 0;
		nextAvailableIndex = 0;
		maxEntryCount = -1;
		maxCountId = -1;
	}


	void MemBufferForIndexing::Flushed(int nextAvail){
		lexicon[maxCountId-1].EntryCount = 0;
		lexicon[maxCountId-1].FirstMemoryEntry = lexicon[maxCountId-1].LastMemoryEntry = -1;
		maxCountId = 0;
		maxEntryCount = 0;
		for(size_t i = 0 ; i < lexicon.size() ; i++){
			if(lexicon[i].EntryCount > bufferSize / 100){
				maxEntryCount = lexicon[i].EntryCount;
				maxCountId = i + 1;
				break;
			}
			if(lexicon[i].EntryCount > maxEntryCount){
				maxEntryCount = lexicon[i].EntryCount;
				maxCountId = i + 1;
			}
		}
		nextAvailableIndex = nextAvail;
	}


	int MemBufferForIndexing::IndexNewEntry(const string& word, int position, int docid, bool &need_flush){
		DictType::const_iterator it = dict.find(word);
		int wordid = -1;
		int prevMemoryEntry = -1;
		if(it != dict.end()){
			wordid = it->second;
			if(lexicon[wordid-1].FirstMemoryEntry < 0){ // previously flushed
				lexicon[wordid-1].FirstMemoryEntry = nextAvailableIndex;
				lexicon[wordid-1].LastMemoryEntry = nextAvailableIndex;
			}else{
				prevMemoryEntry = lexicon[wordid-1].LastMemoryEntry; // store it so we can update the link
				lexicon[wordid-1].LastMemoryEntry = nextAvailableIndex;
			}
		}else{
			lastId += 1;
			wordid = lastId;
			dict.insert(DictType::value_type(word,wordid));
			lexicon.push_back(MemLexiconEntry(wordid,nextAvailableIndex,word.c_str()));

		}
		// store the index
		buffer[nextAvailableIndex].DocID = docid;
		buffer[nextAvailableIndex].NextEntry = -1;
		buffer[nextAvailableIndex].Position = position;
		buffer[nextAvailableIndex].WordID = wordid;
		if(prevMemoryEntry>=0)
			buffer[prevMemoryEntry].NextEntry = nextAvailableIndex;
		lexicon[wordid-1].EntryCount ++;
		if(lexicon[wordid-1].EntryCount > maxEntryCount){
			maxEntryCount = lexicon[wordid-1].EntryCount;
			maxCountId = wordid;
		}

		need_flush = true;
		for(nextAvailableIndex = nextAvailableIndex + 1 ;
			nextAvailableIndex < bufferSize ; nextAvailableIndex ++){ 
				if(buffer[nextAvailableIndex].WordID <= 0){
					need_flush = false;
					break;
				}
		}
		return wordid; // Ask for flushing
	}

	//////////////////////////////////////////////////////////////////////////
	// Main API functions
	InvertedDocument::InvertedDocument(bool index, const char* root, int splits , int buffer_size)
		:totalSplits(splits), rootDir(root), indexMode(index),rootDirPath(root), buffer(buffer_size),
		totalDocuments(0){
			corpusMaster = chunkRelink = corpusSentMaster = NULL;
			if(exists(rootDirPath)){ // Check directory exist
				if(indexMode){
					P_FATAL("The directory for indexing \"%s\" already exists! Cowardly refuse to overwrite it. Quitting",rootDir.c_str());
				}
				boost::filesystem::path rel = rootDirPath / "maxsplit";
				ifstream ifs(rel.string().c_str());
				if(!ifs){
					P_FATAL("The directory does not contain required maxsplit file. Quitting");
				}
				ifs >> totalSplits;
				ifs.close();
				if(totalSplits != splits){
					P_INFO("The directory shows the number of splits is %d instead of specified (or default) %d",totalSplits,splits);
				}
				rel = rootDirPath / "dict";
				ifstream idic(rel.string().c_str());
				C_FATAL(idic);
				qbuffer.LoadLexicon(idic);
				idic.close();
				chunkRelink = OpenFile("crl");
				C_FATAL(chunkRelink);
				qbuffer.LoadChunkRelink(chunkRelink);
				fclose(chunkRelink);
				chunkRelink = NULL;
				corpusMaster = OpenFile("corpus/master");
				C_FATAL(corpusMaster);
				qbuffer.LoadCorpusBounds(corpusMaster);
				fclose(corpusMaster);
			}else{
				if(!indexMode){
					P_FATAL("The directory for indexing \"%s\" does not exists! Quitting",rootDir.c_str());
				}
			}

			invDocs.resize(totalSplits,NULL);

	}

	int InvertedDocument::IndexNewDoc(const char* str){
		A_FATAL(indexMode);
		// First split the text 
		string rep(str);
		trim(rep);
		list<string> words;
		list<int> wids;
		split(words,rep,is_any_of(" \t\n\r"),token_compress_on);
		
		int position = 0;
		bool need_flush;
		for(list<string>::iterator it = words.begin() ; it != words.end(); it++){
			position ++;
			int wid = buffer.IndexNewEntry(*it,position,totalDocuments, need_flush);
			wids.push_back(wid);
			if(need_flush){
				int nextAvail = FlushBuffer();
				A_FATAL(nextAvail>0);
			}
		}
		if(wids.size())
			WriteCorpus(wids);
		// Store document buffer

		return totalDocuments;
	}

	void InvertedDocument::GetSentenceInfo(int sid, long& offset, unsigned char& len){
		if(!corpusSentMaster){
			filesystem::path rel = rootDir / "corpus" / "sentmaster";
			corpusSentMaster = OpenFile(rel.string().c_str());
		}
		long oss = sid ;
		oss *= (sizeof(long)+sizeof(char));
		fseek(corpusSentMaster, oss, SEEK_SET);
		fread(&offset,sizeof(long),1,corpusSentMaster);
		fread(&len,sizeof(char),1,corpusSentMaster);
	}

	void InvertedDocument::WriteCorpus(const std::list<int>& wids){
		totalDocuments ++;
		bool newFile = false;
		if(corpusHandles.size() == 0){
			newFile = true; 
		}else{
			int addsize = wids.size() * sizeof(int);
			long fposition = ftell(corpusHandles.back());
			if(INT_MAX - fposition < addsize){
				newFile = true;
			}
		}
		if(!corpusMaster){
			filesystem::path rel = rootDir / "corpus" / "master";
			corpusMaster = OpenFile(rel.string().c_str());
		}
		if(!corpusSentMaster){
			filesystem::path rel = rootDir / "corpus" / "sentmaster";
			corpusSentMaster = OpenFile(rel.string().c_str());
		}
		
		if(newFile){
			string fname = (boost::format("%03d") % corpusHandles.size() ).str();
			filesystem::path rel = rootDir / "corpus" / fname;
			FILE* nfile =  OpenFile(rel.string().c_str());
			corpusHandles.push_back(nfile);
			fwrite(&totalDocuments,sizeof(int),1,corpusMaster);
		}

		long fpos = ftell(corpusHandles.back());
		fwrite(&fpos, sizeof(long), 1, corpusSentMaster);
		unsigned char c = wids.size() ;
		fwrite(&c, sizeof(char), 1, corpusSentMaster);

		vector<int> wbuf(wids.size());
		copy(wids.begin(),wids.end(), wbuf.begin());
		fwrite(&wbuf[0],sizeof(int),wbuf.size(),corpusHandles.back());

	}

	int InvertedDocument::FlushBuffer(float min_space){
		int space_needed = buffer.GetBufferSize() * min_space;
		int freed = 0;

		if(!chunkRelink){
			filesystem::path rel = rootDir / "crl";
			chunkRelink = OpenFile(rel.string().c_str());
		}
		P_INFO("Flushing memory");

		int minAvail = buffer.GetBufferSize();
		int times = 0;
		while(freed < space_needed){
			times += 1;
			int wid = buffer.GetFlushingWordID();
			if(!wid) break;
			int part_id = wid % totalSplits;
			FILE* fid = GetSplit(part_id);
			long offset = ftell(fid);
			MemLexiconEntry& ent = buffer.GetLexiconEntry(wid);
			WriteMemEntry(ent,fid);
			long offset_e = ftell(fid);
			int len = offset_e - offset;
			WriteRelinkInfo(wid,offset,len);
			freed += ent.EntryCount;
			ent.EntryCount = 0;
			if(ent.FirstMemoryEntry < minAvail)
				minAvail = ent.FirstMemoryEntry;
			ent.FirstMemoryEntry =ent.LastMemoryEntry = -1;		
			buffer.Flushed(minAvail);
		}
		if(times > 10){
			// Buffer too sparse, force full refresh
			P_INFO("Buffer too sparse, force full refresh");
			FlushBufferAll();
		}
		return minAvail;
	}

	int InvertedDocument::FlushBufferAll(){
		if(!chunkRelink){ 
			filesystem::path rel = rootDir / "crl";
			chunkRelink = OpenFile(rel.string().c_str());
		}
		for(size_t wid = 1; wid <= buffer.lexicon.size(); wid++){
            if(wid % 10000 == 1)
                cerr << "Dumped [" << wid+1 << "/" << buffer.lexicon.size() << "] lines    \r" ;
			MemLexiconEntry& ent = buffer.GetLexiconEntry(wid);
			if(ent.EntryCount == 0)
				continue;
			int part_id = wid % totalSplits;
			FILE* fid = GetSplit(part_id);
			long offset = ftell(fid);
			WriteMemEntry(ent,fid);
			long offset_e = ftell(fid);
			int len = offset_e - offset;
			WriteRelinkInfo(wid,offset,len);
			ent.EntryCount = 0;
			ent.FirstMemoryEntry =ent.LastMemoryEntry = -1;		
			buffer.lexicon[wid-1].EntryCount = 0;
			buffer.lexicon[wid-1].FirstMemoryEntry = buffer.lexicon[wid-1].LastMemoryEntry = -1;
		}
		return -1;
	}


	void InvertedDocument::WriteMemEntry(const MemLexiconEntry& ent, FILE* fl){
		unsigned char poses[255];
		unsigned char npos = 0;
		int docid = -1;
		int idx = ent.FirstMemoryEntry;
		while(idx >=0 ){
			MemoryEntry& me = buffer.GetBufferEntry(idx);
			idx = me.NextEntry;
			if(docid != me.DocID){
				if(docid >=0){
					fwrite(&docid, sizeof(int),1,fl);
					fwrite(&npos,sizeof(char),1,fl);
					fwrite(poses,sizeof(char),npos,fl);
				}
				docid = me.DocID;
				npos = 1;
				poses[0] = me.Position;
			}else{
				if(npos == 255){
					continue; // Ignore
				}
				npos++;
				poses[npos-1] = me.Position;
			}
			me.WordID = -1;
		}
		if(docid >=0){
			fwrite(&docid, sizeof(int),1,fl);
			fwrite(&npos,sizeof(char),1,fl);
			fwrite(poses,sizeof(char),npos,fl);
		}

		return;
	}

	void InvertedDocument::WriteRelinkInfo(int word_id,long offset, int len){
		fwrite(&word_id,sizeof(int),1,chunkRelink); 
		fwrite(&offset,sizeof(long),1,chunkRelink);
		fwrite(&len,sizeof(int),1,chunkRelink);
	}

	FILE* InvertedDocument::OpenFile(const char* relpath){
		filesystem::path rpath(relpath);
		
		filesystem::path npath = rpath.is_absolute() ? rpath : ( rootDir / relpath);
		filesystem::path parent = npath.parent_path();
		filesystem::create_directories(parent);
		string a = npath.string();
		if(indexMode)
			return fopen(a.c_str(),"wb");
		else
			return fopen(a.c_str(),"rb");
	}

	vector<string> InvertedDocument::QueryContext(const char* left_context, int nleft_words, const char* righ_context, int nright_words, int length_limit_min, int length_limit_max){
		vector<string> left_c, right_c;
		split(left_c, left_context, is_any_of(" \t\n\r"),token_compress_on);
		split(right_c, righ_context, is_any_of(" \t\n\r"),token_compress_on);
		return QueryContext(left_c, nleft_words, right_c, nright_words, length_limit_min, length_limit_max);
	}

	vector<string> InvertedDocument::QueryContext(const vector<string> & left_context, int nleft_words,
			const vector<string> & right_context, int nright_words, int length_limit_min, int length_limit_max){
		A_FATAL(!indexMode);
		bool hard_restrict_left =(left_context.size()<(size_t)nleft_words); // Whether we have the hard restriction on left side
		bool hard_restrict_right=(right_context.size()<(size_t)nright_words);

		vector<int> lc(left_context.size()),rc(right_context.size());

		if(lc.size())
			transform(left_context.begin(), left_context.end(), &(lc[0]), boost::bind(&MemoryBufferForQuery::QueryWordID,boost::ref(qbuffer),_1));
		if(rc.size())
			transform(right_context.begin(), right_context.end(), &(rc[0]), boost::bind(&MemoryBufferForQuery::QueryWordID,boost::ref(qbuffer),_1));

		for(size_t i = 0; i<lc.size(); i++) if(lc[i] < 0) return vector<string>();
		for(size_t i = 0; i<rc.size(); i++) if(rc[i] < 0) return vector<string>();

		list<ResultEntry> results;
		FetchList(lc, nleft_words, rc, nright_words, length_limit_min, length_limit_max, results,hard_restrict_left,hard_restrict_right);
		// How to output

		vector<string> ret(results.size());
		transform(results.begin(),results.end(),ret.begin(),boost::bind(&InvertedDocument::RecoverSentence, boost::ref(*this),_1));

		return ret;
	}

	bool InvertedDocument::QueryEvidence(const vector<string>& left_context, int nleft_words, const vector<string>& right_context, int nright_words){
		A_FATAL(!indexMode);
		bool hard_restrict_left =(left_context.size()<(size_t)nleft_words); // Whether we have the hard restriction on left side
		bool hard_restrict_right=(right_context.size()<(size_t)nright_words);

		vector<int> lc(left_context.size()),rc(right_context.size());

		if(lc.size())
			transform(left_context.begin(), left_context.end(), &(lc[0]), boost::bind(&MemoryBufferForQuery::QueryWordID,boost::ref(qbuffer),_1));
		if(rc.size())
			transform(right_context.begin(), right_context.end(), &(rc[0]), boost::bind(&MemoryBufferForQuery::QueryWordID,boost::ref(qbuffer),_1));

		for(size_t i = 0; i<lc.size(); i++) if(lc[i] < 0) return false;
		for(size_t i = 0; i<rc.size(); i++) if(rc[i] < 0) return false;
		vector<QueryState* > l_buffer(left_context.size()), r_buffer(right_context.size()), c_buffer(left_context.size()+right_context.size());
		transform(lc.begin(),lc.end(),l_buffer.begin(),boost::bind(&InvertedDocument::LoadInvDocForWid,boost::ref(*this),_1));
		transform(rc.begin(),rc.end(),r_buffer.begin(),boost::bind(&InvertedDocument::LoadInvDocForWid,boost::ref(*this),_1));
		copy(l_buffer.begin(),l_buffer.end(), &(c_buffer[0]));
		copy(r_buffer.begin(),r_buffer.end(), &(c_buffer[left_context.size()]));
		
		list<ResultEntry> results;

		while(FindNextDocument(l_buffer,r_buffer,c_buffer,results, -1,-1,hard_restrict_left,hard_restrict_right,true)){ // Now nothing, do we need to check sth else? add here
			if(results.size())
				break;
		}
		for(vector<QueryState* >::iterator it = l_buffer.begin(); it != l_buffer.end(); it++) 
			if(*it) delete *it;
		for(vector<QueryState* >::iterator it = r_buffer.begin(); it != r_buffer.end(); it++) 
			if(*it) delete *it;
		return results.size();

	}

	list<pair<int,int> > InvertedDocument::GetContinuousRegions(vector<QueryState* >& buffer){
		list<pair<int,int> > ret;
		vector<pair<int,int> > points(buffer.size());
		for(size_t i = 0; i< buffer.size(); i++){
			buffer[i]->LoadPositions();
			points[i].first = 0;
			points[i].second = buffer[i]->positions[0];
		}

		if(buffer.size()==1){
			for(size_t i = 0 ; i< buffer[0]->positions.size() ;i++){
				ret.push_back(pair<int,int>(buffer[0]->positions[i],buffer[0]->positions[i]));
			}
			return ret;
		}
		int curr = 0;
		
		while(true){
			if(points[curr].second == points[curr+1].second - 1){
				curr += 1;
				if(curr == buffer.size()-1){
					ret.push_back(pair<int,int>(points[0].second,points[curr].second));
					curr = 0;
					for(size_t i = 0; i< buffer.size(); i++){
						points[i].first++;
						if(points[i].first >= buffer[i]->positions.size())
							return ret;
						points[i].second = buffer[i]->positions[points[i].first];
					}
				}
			}else if(points[curr].second < points[curr+1].second - 1){
				points[curr].first ++ ;
				if(points[curr].first >= buffer[curr]->positions.size())
					return ret;
				points[curr].second = buffer[curr]->positions[points[curr].first] ;
				curr = 0;
			}else{
				points[curr+1].first ++ ;
				if(points[curr+1].first >= buffer[curr+1]->positions.size())
					return ret;
				points[curr+1].second = buffer[curr+1]->positions[points[curr+1].first] ;
				curr = 0;
			}
		}
		return ret;
	}

	void InvertedDocument::FetchList(const vector<int>& left_context, int nleft_words, 
		const vector<int>& right_context, int nright_words,
		int length_limit_min , int length_limit_max , list<ResultEntry>& results, bool h_l, bool h_r){
		// Fetch doc list left
		// Fetch doc list_right
    	// Merge
		// Return
		vector<QueryState* > l_buffer(left_context.size()), r_buffer(right_context.size()), c_buffer(left_context.size()+right_context.size());
		transform(left_context.begin(),left_context.end(),l_buffer.begin(),boost::bind(&InvertedDocument::LoadInvDocForWid,boost::ref(*this),_1));
		transform(right_context.begin(),right_context.end(),r_buffer.begin(),boost::bind(&InvertedDocument::LoadInvDocForWid,boost::ref(*this),_1));
		if(l_buffer.size())
			copy(l_buffer.begin(),l_buffer.end(), &(c_buffer[0]));
		if(r_buffer.size())
			copy(r_buffer.begin(),r_buffer.end(), &(c_buffer[left_context.size()]));
		

		while(FindNextDocument(l_buffer,r_buffer,c_buffer,results, length_limit_min,length_limit_max,h_l,h_r)); // Now nothing, do we need to check sth else? add here

		for(vector<QueryState* >::iterator it = l_buffer.begin(); it != l_buffer.end(); it++) 
			if(*it) delete *it;
		for(vector<QueryState* >::iterator it = r_buffer.begin(); it != r_buffer.end(); it++) 
			if(*it) delete *it;
	} 

	bool InvertedDocument::FindNextDocument(vector<QueryState* >& l_buffer,vector<QueryState* >& r_buffer, vector<QueryState* >& c_buffer, list<ResultEntry>& ret, int l_min, int l_max, bool h_l, bool h_r, bool evidence_only){
		int bound;
		bool isLeft;
		ResultEntry resultEntry;
		bool continueWorking = true;
		list<pair<int,int> >::iterator newLast;
		if(l_max<0){
			l_max = INT_MAX;
		}
		while(continueWorking){
			QueryState* qst = GetMinStateAndBound(l_buffer,r_buffer,bound,isLeft);
/*
			for(int i=0; i<c_buffer.size();i++){
				cout << "(" << i << "," << c_buffer[i]->docid << ")" << ",";
			}
*/
			if(!qst) return false;
			if(bound == INT_MAX){ // Only one word, find every doc
				qst->LoadPositions();
				resultEntry.docID = qst->docid;
				if(isLeft){
					long offset;
					unsigned char len;
					GetSentenceInfo(qst->docid,offset,len);
					for(size_t i =0; i<qst->positions.size();i++){
						if(h_l && qst->positions[i] !=1)
							break;
						int lth = len - qst->positions[i] - 1;
						if(lth >= l_min && lth <=l_max){
							resultEntry.start = qst->positions[i];
							resultEntry.end = len; // REMEMBER LAST WORD INDEX IS EXCLUSIVE
							ret.push_back(resultEntry);
							if(evidence_only) return false;
						}
					}
				}else{
					long offset;
					unsigned char len;
					if(h_r)
						GetSentenceInfo(qst->docid,offset,len);
					for(size_t i =0; i<qst->positions.size();i++){
						if(h_r && qst->positions[i]!=len)
							continue;
						int lth = qst->positions[i] - 1;
						if(lth >= (int) l_min && lth <= (int) l_max){
							resultEntry.end = qst->positions[i]-1;
							resultEntry.start = 0; // REMEMBER LAST WORD INDEX IS EXCLUSIVE
							ret.push_back(resultEntry);
							if(evidence_only) return false;
						}
					}
				}
				if(!qst->NextDocument()){
					return false;
				}
				return true;
			}else{ // More than one, 
				do{
					if(qst->docid == bound){ // Possible match
						// see if all ids are the same
						bool id_same = true;
						for(size_t i = 0; i < c_buffer.size(); i++){
							if(c_buffer[i]->docid != qst->docid){
								id_same = false;
								break;
							}
						}
						if(id_same){
							long offset;
							unsigned char len;
							if(l_buffer.size()==0){
								list<pair<int,int> > r_lst = GetContinuousRegions(r_buffer);
								if(h_r && r_lst.size()){
									GetSentenceInfo(qst->docid,offset,len);
									newLast = remove_if(r_lst.begin(),r_lst.end(),boost::bind(&pair<int,int>::second,_1) != len);
									r_lst.erase(newLast,r_lst.end());
								}
								resultEntry.docID = qst->docid;
								for(list<pair<int,int> >::iterator it = r_lst.begin(); it!= r_lst.end(); it++){
									if(it->first -1 < l_max && it->first-1 > l_min){
										resultEntry.start = 0;
										resultEntry.end = it->first-1;
										ret.push_back(resultEntry);
										if(evidence_only) return false;
									}
								}
							}else{
								if(r_buffer.size()==0){
									list<pair<int,int> > l_lst = GetContinuousRegions(l_buffer);
									resultEntry.docID = qst->docid;
									
									GetSentenceInfo(qst->docid,offset,len);
									if(h_l && l_lst.size()){
										newLast = remove_if(l_lst.begin(),l_lst.end(),boost::bind(&pair<int,int>::first,_1) != 1);
										l_lst.erase(newLast,l_lst.end());
									}
									for(list<pair<int,int> >::iterator it = l_lst.begin(); it!= l_lst.end(); it++){
										if(len - it->second -1 < l_max && len - it->second-1 > l_min){
											resultEntry.start = it->second ;
											resultEntry.end = len;
											ret.push_back(resultEntry);
											if(evidence_only) return false;
										}
									}
								}else{
									resultEntry.docID = qst->docid;
									list<pair<int,int> > r_lst = GetContinuousRegions(r_buffer);
									list<pair<int,int> > l_lst = GetContinuousRegions(l_buffer);

									if(h_r && r_lst.size()){	
										GetSentenceInfo(qst->docid,offset,len);
										newLast = remove_if(r_lst.begin(),r_lst.end(),boost::bind(&pair<int,int>::second,_1) != len);
										r_lst.erase(newLast,r_lst.end());
									}
									if(h_l){
										newLast = remove_if(l_lst.begin(),l_lst.end(),boost::bind(&pair<int,int>::first,_1) != 1);
										l_lst.erase(newLast,l_lst.end());
									}
									for(list<pair<int,int> >::iterator it = l_lst.begin(); it!= l_lst.end(); it++){
										for(list<pair<int,int> >::iterator it1 = r_lst.begin(); it1!= r_lst.end(); it1++){
											int l = it1->first - it->second - 1;
											if( l < l_max && l > l_min && (l > 0 && !evidence_only)){
												resultEntry.start = it->second ;
												resultEntry.end = it1->first-1;
												ret.push_back(resultEntry);
												if(evidence_only) return false;
											}
										}
									}
								}
							}
							for(size_t i = 0; i < c_buffer.size(); i++){
								if(!c_buffer[i]->NextDocument())
									return false;
							}
							// Make sure when we break, a new entry will be picked
							return true; // If we break here, we will not return and it will go back to GetMinStateAndBound, which will find at least one new entry
						}else{
							for(size_t i = 0; i < c_buffer.size(); i++){ // The fact is, we know docid == bound, there won't be a match, so for each state that docid == bound, proceed to next
								if(c_buffer[i]->docid == bound){
									if(!c_buffer[i]->NextDocument())
										return false;
								}
							}
							return true;
						}
					}else if(qst->docid < bound){
						
					}else{
						return true;
					}
				}while(qst->NextDocument());
				return false;
			}
		}
	}

	QueryState* InvertedDocument::GetMinStateAndBound(vector<QueryState*> &l_buffer, vector<QueryState*> &r_buffer, int& bound, bool& isLeft){
		QueryState* minid1 = NULL;
		int min1 = INT_MAX, min2 = INT_MAX;
		for(size_t i = 0 ; i < l_buffer.size() ; i++){
			if(l_buffer[i]->docid < 0){
				if(!l_buffer[i]->NextDocument()){
					return NULL;
				}
			}
			if(min1 >= l_buffer[i]->docid){
				min2 = min1;
				min1 = l_buffer[i]->docid;
				minid1 = l_buffer[i];
				isLeft = true;
			}else if(min2 > l_buffer[i]->docid){
				min2 =  l_buffer[i]->docid;
			}
		}
		for(size_t i = 0 ; i < r_buffer.size() ; i++){
			if(r_buffer[i]->docid < 0){
				if(!r_buffer[i]->NextDocument()){
					return NULL;
				}
			}
			if(min1 >= r_buffer[i]->docid){
				min2 = min1;
				min1 = r_buffer[i]->docid;
				minid1 = r_buffer[i];
				isLeft = false;
			}else if(min2 > r_buffer[i]->docid){
				min2 =  r_buffer[i]->docid;
			}
		}
		bound = min2;
		return minid1;
	}


	QueryState* InvertedDocument::LoadInvDocForWid(int wid){
		QueryState* qs = new QueryState();
		const vector<pair<long,int> >& chunks = *qbuffer.GetChunkForWord(wid);
		FILE* fh = GetSplitForWid(wid);
		int fsize = 0;
		for(size_t i = 0; i< chunks.size() ; i++)
			fsize += chunks[i].second;
		if(!fsize)
			return NULL;
		unsigned char* buffer = new unsigned char[fsize];
		unsigned char* p = buffer;
		for(size_t i = 0; i< chunks.size() ; i++){
			fseek(fh,chunks[i].first,SEEK_SET);
			int l = fread(p,1,chunks[i].second,fh);
			if(chunks[i].second != l)
				P_FATAL("Read error for wid %d (chunk %d, offset %ld, len %d, read %d)", wid, i,chunks[i].first,chunks[i].second, l);
			p+= chunks[i].second;
		}
		qs->buffer = p;
		qs->pos = qs->phead = buffer;
		return qs;
	}
	 

	string InvertedDocument::RecoverSentence(const ResultEntry & e){
		string ret;
		int sid = e.docID;
		long offset;
		unsigned char len;
		GetSentenceInfo(sid, offset, len);
		size_t split = (size_t) qbuffer.GetCorpusSplitForSent(sid);
		FILE* nfile = NULL;
		if(corpusHandles.size() < split+1){
			corpusHandles.resize(split+1,0);
		}else{
			nfile = corpusHandles[split];
		}
		if(!nfile){
			string fname = (boost::format("%03d") % split ).str();
			filesystem::path rel = rootDir / "corpus" / fname;
			corpusHandles[split] = nfile = OpenFile(rel.string().c_str());
		}

		fseek(nfile,offset,SEEK_SET);
		vector<int> wds(len);
		fread(&wds[0], sizeof(int), len, nfile);
		for(int i = e.start ; i<e.end; i++){
			ret += qbuffer.GetWord(wds[i]);
			ret += " ";
		}
		trim(ret);
		return ret;
	}

	FILE* InvertedDocument::GetSplitForWid(int wid){
		return GetSplit(wid % totalSplits);
	}


	FILE* InvertedDocument::GetSplit(int i){
		A_FATAL(i>=0 && i < totalSplits);
		if(invDocs[i] == NULL){
			string relpath = ( boost::format("inv/%03d") % i ).str();
			invDocs[i] = OpenFile(relpath.c_str());
		}
		return invDocs[i];
	}
	void InvertedDocument::FlushAll(){
//		int flush_id;
        P_INFO("Start flushing all indices\n");
		FlushBufferAll();

		boost::filesystem::path rel = rootDirPath / "dict";


		ofstream ofs(rel.string().c_str());
        
        P_INFO("Dumping Lexicon\n");
		buffer.DumpLexicon(ofs);

		ofs.close();

		boost::filesystem::path rel1 = rootDirPath / "maxsplit";

		ofstream ofs1(rel1.string().c_str());

		ofs1 << this->totalSplits << endl;

		ofs1.close();

	}	

	void InvertedDocument::Close(){
		FlushAll();

		if(corpusMaster)
			fclose(corpusMaster);
		if(corpusSentMaster)
			fclose(corpusSentMaster);
		if(chunkRelink){
			// Before closing, update the last entry
			
			fclose(chunkRelink);
		}
		for_each(corpusHandles.begin(),corpusHandles.end(),fclose_ss);
		for_each(invDocs.begin(),invDocs.end(),fclose_ss);
		corpusHandles.resize(0);
		invDocs.resize(0);
	}

}
