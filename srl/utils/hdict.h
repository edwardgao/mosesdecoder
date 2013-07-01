
#ifndef __HDICT_H__
#define __HDICT_H__

#include <boost/unordered_map.hpp>
#include <vector>
#include <string>
#include <iostream>

namespace srl{

    class dict_exception : public std::exception
    {
        private:
            std::string m_what;
    
        public:
            dict_exception(){};

            dict_exception(const char * what): m_what(what) {}

            inline const char* what() const throw() {return m_what.c_str();};

            virtual ~dict_exception() throw() {}
    };

	class HDICT{
	protected:
		boost::unordered_map<std::string, int> m_word_id;
		bool m_lock_vocab; // Whether allow adding new vocab
		std::vector<const std::string* > m_words;
	public:
		static const int UNK = -1;
		HDICT(bool block): m_lock_vocab(block){};

		bool LockedVocab() const {return m_lock_vocab;}


		int GetWordID(const std::string& word) const{
			boost::unordered_map<std::string, int>::const_iterator it = m_word_id.find(word);
			if(it == m_word_id.end()){
				if(LockedVocab())
					return UNK;
				else{
					throw dict_exception("Trying to add new word while being constant");
				}
			}else{
				return it->second;
			}
		}


		int GetWordID_add(const std::string& word){
			boost::unordered_map<std::string, int>::const_iterator it = m_word_id.find(word);
			if(it == m_word_id.end()){
				if(LockedVocab())
					return UNK;
				else{
					int w;
					std::pair<boost::unordered_map<std::string, int>::const_iterator, bool> 
						ret = m_word_id.insert(std::make_pair(word, w = m_word_id.size()));
					assert(ret.second);
					m_words.resize(w+1);
					m_words[w] = &(ret.first->first);
					return w;
				}
			}else{
				return it->second;
			}
		}

		void Serialize(std::ostream& ostr, const char* header){
			ostr << "@@@@BEGIN:" << header << std::endl;
			for(int i = 0; i< m_words.size(); i++){
				ostr << i << " " << (m_words[i] ? "UN+USED+WORD+VERY+BAD" : m_words[i]->c_str()) << std::endl;
			}
			ostr << "@@@@END:"<<std::endl;
		}
	};
}

#endif

