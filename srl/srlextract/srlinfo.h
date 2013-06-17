/*
* srcheader.h
*
*  Created on: Nov 5, 2010
*      Author: qing
*/

#ifndef SRLHEADER_H_
#define SRLHEADER_H_

#include <string>
#include <vector>
#include <set>
#include <boost/thread.hpp>
#include "porter_stemmer.h"
#include <boost/unordered_map.hpp>

namespace srl{
	
	// A class holding functions for classifing tags
	class TagClassifier{
	public:
		virtual bool IsPredicate (const std::string& str) const;
		virtual std::string MapToStandardName (const std::string& str)const ;

		static TagClassifier Instance;
	};

	class SRLInfoReader;

	class SRLInformation{
		friend class SRLInfoReader;
	public:
		// Public accessors
		inline size_t GetNumberOfWords()const {return words_.size();};
		inline const std::string& GetWordAtIndex(size_t index)const{return words_[index];};
		inline size_t GetNumberOfLabels() const{return label_.size();};
		inline const std::string& GetLabelAtIndex(size_t index)const{return label_[index];};
		inline const std::pair<int,int>& GetRegionAtIndex(size_t index) const {return regions_[index];};
		inline int GetSentenceNumber() const{return sent_number_;};
		inline bool error() const{return error_;};
		
		/**!
		Map the indices in the sentence to the sentence, return false if cannot map
		the index_map will have the same length of the origin sentence, and each cell contains the index of corresponding word of the original word
		on the SRL structure. If specified, reversed_map will be the same size as the SRL structure (GetNumberOfWords), and each cell contains the index 
		of the original sentence's word
		*/
		bool MapIndices(const std::vector<std::string>& origin_sentence, std::vector<int>& index_map, std::vector<int>* reversed_map);

		int GetRegionCoveringWord(int word_index);
		std::set<int> GetRegionCoveringWords(int start, int end);
		int GetPredicateIndex(const TagClassifier& cls = TagClassifier::Instance);

		// Constructors
		SRLInformation();
		~SRLInformation();


	protected:
		// Actual initializer
		void Init();

	private:
		std::vector<std::string> words_;
		std::vector<std::string> label_;
		// The regions should be INCLUSIVE
		std::vector<std::pair<int,int> > regions_;
		std::vector<int> word_to_region_map_;
		int predicate_index;
		bool error_;
		int sent_number_;
	};

	enum PredicatePlacement
	{
		PP_INVALID_PLACEMENT = -1,
		PP_PROVIDED     = 0, // Predicate provided with the **TERMINAL**
		PP_LEFT_EXPECT  = 1, // Predicate is expected to the left of the phrase
		PP_RIGHT_EXPECT = 2, // Predicate is expected to the right of the phrase
		PP_NT_PROVIDED  = 3, // Provided by one of the non-terminals. For any number, substract NT_PROVIDED to get the index of the NT
	};

	// Functions for Predicate Placement
	inline bool IsNTProvided(PredicatePlacement p) {return p>=3;};
	inline int  ProvidedNTIndex(PredicatePlacement p) {return p-3;};

	enum ArgumentPlacement
	{
		AP_INVALID_PLACEMENT    =  0,
		AP_LEFT_COMPLETE        =  0x100, // Completed on the left side
		AP_RIGHT_COMPLETE       =  0x200, // Completed on the right side
		AP_COMPLETE             =  0x300, // Fully completed == LEFT_COMPLETE | RIGHT_COMPLETE
		AP_HOLE_1               = 0x0003, // Whether there's a hole or holes, at most 4 wholes as we design this
		AP_HOLE_2               = 0x000C, // Whether there's a hole or holes, at most 4 wholes as we design this
		AP_HOLE_3               = 0x0030, // Whether there's a hole or holes, at most 4 wholes as we design this
		AP_HOLE_4               = 0x00C0, // Whether there's a hole or holes, at most 4 wholes as we design this
	};

	

	// Functions for Argument placement
	inline bool IsCompleted(ArgumentPlacement a) {return a & AP_COMPLETE; };
	inline int GetHoleNonterminalIndex(ArgumentPlacement a, int idx) { return (a >> (idx *2)) & AP_HOLE_1 ; };
	inline ArgumentPlacement SetHoleNonterminalIndex(ArgumentPlacement a , int idx) { return (ArgumentPlacement) (a | ((idx & AP_HOLE_1) << (idx * 2)));};


	typedef int TArgumentType;
	typedef int TPredicateType;

	// SRL Argument structure
	struct SRLArgument
	{
		static TArgumentType MapAugumentName(std::string& pstr);
		static boost::unordered_map<std::string, TArgumentType> id_map;
		static void SaveMapping(std::ostream& ofs);
		static bool LoadMapping(std::istream& ifs);
		ArgumentPlacement ArgumentLocation;
		TArgumentType     ArgumentName;

		SRLArgument(): ArgumentLocation(AP_INVALID_PLACEMENT), ArgumentName(-1) {}
	};

	/// A single SRL Event for a phrase
	struct SRLFrame{
		static TPredicateType MapPredicateName(std::string& pstr);
		static boost::unordered_map<std::string, TArgumentType> id_map;
		static void SaveMapping(std::ostream& ofs);
		static bool LoadMapping(std::istream& ifs);
		TPredicateType PredicateName ; // The predicate name, no matter it is expected or provided
		PredicatePlacement PredicateLocation ; // Whether the predicate has been provide, or expected in any location
		int PredicateIndex; // Which word is the predicate. -1 if predicate is not provided by the terminals
		std::vector<SRLArgument> Arguments;
		SRLFrame() : PredicateName(-1), PredicateLocation(PP_INVALID_PLACEMENT), PredicateIndex(-1) {}
	};


	/// Class to store extracted SRL information
	typedef std::vector<SRLFrame> TProvidedFramesForPhrases;

	class SRLRuleExtractor
	{
	public:
		/**!
		Do the actual extraction, need to pass in the SRL information structure, the index maps and the wholes.
		NOTE: All indices are INCLUSIVE
		*/
		SRLFrame ExtractSRLInfo(SRLInformation &srlInfo, 
			const std::vector<int>& indexMap, const std::vector<int>& indexRevMap, const std::vector<std::pair<int, int> >& HolesInclusive, 
			int start, int end, const TagClassifier& tagclassifier = TagClassifier::Instance, bool lc = true, bool stem = false);

	private:
		struct stemmer* stemmer;
		std::string do_stemming(const std::string& ori);
		boost::mutex m_stemming;
		std::string GetRep(SRLInformation &srlInfo, const std::pair<int,int>& region, bool lc, bool stemming);
	public:
		SRLRuleExtractor(){
			boost::mutex::scoped_lock(m_stemming);
			stemmer = create_stemmer();
		}

		virtual ~SRLRuleExtractor(){
			boost::mutex::scoped_lock(m_stemming);
			free_stemmer(stemmer);
		}
	};

	/// Convert SRL frames to string representation
	std::string FramesToString(const TProvidedFramesForPhrases& srls);

	/// Convert SRL string representation to SRL frame
	void StringToFrame(const std::string, TProvidedFramesForPhrases& srls);


}

#endif /* SRCHEADER_H_ */
